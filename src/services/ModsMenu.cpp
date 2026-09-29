// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ModsMenu.h"
#include "LevelsMenu.h"
#include "NativeMenu.h"
#include "../core/Framework.h"
#include "../mod/ContentBuild.h"
#include "../mod/Discovery.h"
#include "../mod/Host.h"
#include "format/Ini.h"
#include "menu/ModRows.h"
#include "menu/ModsPage.h"

#include <windows.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{
    constexpr int kRow = 2;

    // A mod's page, copied at registration; the title is ours so the mod's string need not outlive the call.
    struct ModPage
    {
        GbhNativeMenuDesc desc;
        std::string       title;
    };

    SRWLOCK              g_pagesLock = SRWLOCK_INIT;   // mods register on the boot thread, the menu reads on the main one
    ModRows::Table       g_rows;
    std::vector<ModPage> g_pages;

    std::vector<ModsPage::Mod> g_mods;
    ModsPage::Header           g_header;
    int                        g_detail = -1;

    ModsPage::State StateOf(const ModSet::Record& r, const Host::Status* s, ContentBuild::Outcome c)
    {
        if (r.disabled) return ModsPage::State::OffIni;
        if (!r.accepted) return ModsPage::State::Refused;
        if (s && s->state == Host::State::Failed) return ModsPage::State::Failed;
        if (c == ContentBuild::Outcome::Failed) return ModsPage::State::Failed;
        if (!s || s->state == Host::State::Pending) return ModsPage::State::Pending;
        return s->state == Host::State::Loaded ? ModsPage::State::Loaded : ModsPage::State::NoCode;
    }

    // mods_disabled as it is on disk now, not as this boot read it.
    std::vector<std::string> OffOnDisk()
    {
        std::string text;
        Settings::ReadFile(text);
        return Ini::ListOf(text, "mods_disabled");
    }

    bool Listed(const std::vector<std::string>& off, const ModSet::Record& r)
    {
        for (const std::string& o : off) if (ModSet::Names(o, r)) return true;
        return false;
    }

    // Rewrites the one line; a mod listed under its folder name is replaced by its id.
    bool WriteOff(const ModSet::Record& r, const ModsPage::Mod& m, bool off)
    {
        return Settings::Rewrite([&](const std::string& text) {
            std::string value;
            for (const std::string& o : Ini::ListOf(text, "mods_disabled"))
                if (!ModSet::Names(o, r)) value += (value.empty() ? "" : ", ") + o;
            if (off) value += (value.empty() ? "" : ", ") + m.id;
            return Ini::Set(text, "mods_disabled", value);
        });
    }

    void FillContent(ModsPage::Mod& m, const ContentBuild::Info& i)
    {
        using CB = ContentBuild::Outcome;
        m.assetFiles = i.assetFiles;
        m.origin = i.origin == ContentBuild::Origin::Cached ? ModsPage::Origin::Cached
                 : i.origin == ContentBuild::Origin::Built  ? ModsPage::Origin::Built : ModsPage::Origin::None;
        switch (i.outcome)
        {
        case CB::Pending: m.mount = ModsPage::Mount::NotYet; break;
        case CB::Mounted: m.mount = ModsPage::Mount::Mounted; break;
        case CB::InChain: m.mount = ModsPage::Mount::InChain; break;
        case CB::Failed:  m.mount = m.origin == ModsPage::Origin::None ? ModsPage::Mount::BuildFailed : ModsPage::Mount::MountFailed; break;
        case CB::None:    m.mount = ModsPage::Mount::None; break;
        }
    }

    void Gather()
    {
        g_mods.clear();
        const std::vector<std::string> bootOff = Ini::List(Settings::Get("mods_disabled", ""));
        const std::vector<std::string> diskOff = OffOnDisk();
        for (const ModSet::Record& r : Mods::Result().records)
        {
            const Host::Status* s = r.mod.id.empty() || !r.accepted ? nullptr : Host::StatusOf(r.mod.id.c_str());
            const ContentBuild::Outcome c = r.mod.id.empty() ? ContentBuild::Outcome::None : ContentBuild::OutcomeOf(r.mod.id.c_str());
            ModsPage::Mod m;
            m.id      = r.mod.id.empty() ? r.folder : r.mod.id;
            m.version = r.mod.version;
            m.folder  = r.folder;
            m.state   = StateOf(r, s, c);
            m.note    = m.state == ModsPage::State::Refused ? r.refusal : (m.state == ModsPage::State::Failed && s ? s->note : "");
            m.codeFiles = r.mod.plugin.empty() ? 0 : 1;
            if (!r.mod.id.empty() && r.accepted) FillContent(m, ContentBuild::InfoOf(r.mod.id.c_str()));
            m.toggle  = Listed(diskOff, r) ? ModsPage::Switch::TurnOn : ModsPage::Switch::TurnOff;
            m.changed = Listed(diskOff, r) != Listed(bootOff, r);
            g_mods.push_back(m);
        }
        g_header.version      = Framework::kVersionString;
        g_header.targetMd5    = GBHOOK_TARGET_MD5;
        g_header.roots        = Ini::List(Settings::Get("mods_root", "mods"));
        g_header.missingRoots = Mods::MissingRoots();
    }

    // One mod in full; its Back row pops the page.
    void BuildDetail(void*)
    {
        if (g_detail < 0 || g_detail >= (int)g_mods.size()) return;
        for (const Rows::Row& r : ModsPage::Detail(g_mods[(size_t)g_detail])) NativeMenu::AddRow(r.label.c_str(), r.action);
    }
    int ActivateDetail(int action, void*)
    {
        if (action == ModsPage::kBack) return GBH_NATIVE_CLOSE;
        if (action != ModsPage::kToggle || g_detail < 0 || g_detail >= (int)g_mods.size()) return GBH_NATIVE_STAY;
        const ModsPage::Mod& m = g_mods[(size_t)g_detail];
        const bool off = m.toggle == ModsPage::Switch::TurnOff;
        const bool ok  = WriteOff(Mods::Result().records[(size_t)g_detail], m, off);
        if (ok) Log::Writef("MODS", "%s %s in gbhook.ini, from the next start", m.id.c_str(), off ? "disabled" : "enabled");
        else    Log::Writef("MODS", "gbhook.ini could not be written; %s is unchanged", m.id.c_str());
        Gather();
        g_mods[(size_t)g_detail].saveFailed = !ok;
        // Stay: the switch relabels to the inert restart row, same row count, so a second press does nothing.
        return GBH_NATIVE_STAY;
    }

    void BuildList(void*)
    {
        for (const Rows::Row& r : ModsPage::List(g_header, g_mods)) NativeMenu::AddRow(r.label.c_str(), r.action);
    }
    int ActivateList(int action, void*)
    {
        if (action < 0 || action >= (int)g_mods.size()) return GBH_NATIVE_STAY;
        g_detail = action;
        GbhNativeMenuDesc d = { sizeof d, BuildDetail, ActivateDetail, nullptr, g_mods[(size_t)action].id.c_str() };
        if (NativeMenu::OpenPage(&d) != GBH_OK) Log::Write("MODS", "the mod's page could not be opened");
        return GBH_NATIVE_STAY;
    }

    void BuildRoot(void*)
    {
        AcquireSRWLockShared(&g_pagesLock);
        std::vector<bool> live;
        for (const ModRows::Entry& e : g_rows.Entries())
        {
            const Host::Status* s = Host::StatusOf(e.owner.c_str());
            live.push_back(s && s->state == Host::State::Loaded);
        }
        const std::vector<Rows::Row> rows = ModRows::Root(g_rows, live);
        ReleaseSRWLockShared(&g_pagesLock);
        for (const Rows::Row& r : rows) NativeMenu::AddRow(r.label.c_str(), r.action);
    }

    void OpenModPage(int i)
    {
        AcquireSRWLockShared(&g_pagesLock);
        const bool known = i >= 0 && i < (int)g_pages.size();
        GbhNativeMenuDesc d = known ? g_pages[(size_t)i].desc : GbhNativeMenuDesc{};
        const std::string owner = known ? g_rows.Entries()[(size_t)i].owner : "";
        ReleaseSRWLockShared(&g_pagesLock);
        if (known && NativeMenu::OpenPage(&d) != GBH_OK) Log::Writef("MODS", "%s's page could not be opened", owner.c_str());
    }

    int ActivateRoot(int action, void*)
    {
        if (action >= ModRows::kFirstMod) OpenModPage(action - ModRows::kFirstMod);
        else if (action == ModRows::kLoadLevel) LevelsMenu::Open();
        else if (action == ModRows::kViewMods)
        {
            Gather();
            GbhNativeMenuDesc d = { sizeof d, BuildList, ActivateList, nullptr, "Mods" };
            if (NativeMenu::OpenPage(&d) != GBH_OK) Log::Write("MODS", "the mods list could not be opened");
        }
        return GBH_NATIVE_STAY;
    }

    void OnRow(int, void*)
    {
        GbhNativeMenuDesc d = { sizeof d, BuildRoot, ActivateRoot, nullptr, "Mods" };
        if (NativeMenu::OpenPage(&d) != GBH_OK) Log::Write("MODS", "the Mods page could not be opened");
    }
}

namespace ModsMenu
{
    void Install()
    {
        if (NativeMenu::ClaimRow(nullptr, kRow, OnRow, nullptr) != GBH_OK) return;
        NativeMenu::SetRowLabel(nullptr, kRow, "Mods");
    }

    int AddPage(const char* owner, const char* label, const GbhNativeMenuDesc* desc)
    {
        if (!owner || !*owner) return GBH_ERR_STATE;
        if (!label || !desc || !desc->build || !desc->activate || desc->struct_size < offsetof(GbhNativeMenuDesc, title))
            return GBH_ERR_ARG;

        ModPage p = {};
        const size_t n = desc->struct_size < sizeof p.desc ? desc->struct_size : sizeof p.desc;
        memcpy(&p.desc, desc, n);
        p.desc.struct_size = sizeof p.desc;
        const bool hasTitle = n >= offsetof(GbhNativeMenuDesc, title) + sizeof p.desc.title && p.desc.title;
        p.title = hasTitle ? p.desc.title : label;

        AcquireSRWLockExclusive(&g_pagesLock);
        const ModRows::Added a = g_rows.Add(owner, label);
        if (a == ModRows::Added::Ok)
        {
            g_pages.push_back(p);
            for (ModPage& q : g_pages) q.desc.title = q.title.c_str();   // the push may have moved every string
        }
        ReleaseSRWLockExclusive(&g_pagesLock);

        switch (a)
        {
        case ModRows::Added::Ok:
            Log::WriteFrom(owner, "MODS", ("page '" + std::string(label) + "' added to the Mods page").c_str());
            return GBH_OK;
        case ModRows::Added::Taken:
            Log::WriteFrom(owner, "MODS", "a second Mods page refused: one per mod");
            return GBH_ERR_CONFLICT;
        case ModRows::Added::Full:
            Log::WriteFrom(owner, "MODS", "the Mods page is full; this mod's row is not shown");
            return GBH_ERR;
        default:
            return GBH_ERR_ARG;
        }
    }
}
