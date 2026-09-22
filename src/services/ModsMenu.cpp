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
#include "menu/ModsPage.h"

#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>

namespace
{
    constexpr int kRow = 2;
    enum { kLoadLevel = 1, kViewMods = 2 };

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

    std::string IniPath() { return std::string(Framework::GameDir()) + "\\gbhook.ini"; }

    bool ReadIni(std::string& out)
    {
        FILE* f = nullptr;
        if (fopen_s(&f, IniPath().c_str(), "rb") != 0 || !f) return false;
        char   buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof buf, f)) > 0) out.append(buf, n);
        fclose(f);
        return true;
    }

    // mods.disabled as it is on disk now, not as this boot read it.
    std::vector<std::string> OffOnDisk()
    {
        std::string text;
        ReadIni(text);
        const std::string* v = Ini::Find(Ini::Parse(text), "mods.disabled");
        return v ? Ini::List(*v) : std::vector<std::string>{};
    }

    bool Names(const std::string& item, const ModsPage::Mod& m)
    {
        return _stricmp(item.c_str(), m.id.c_str()) == 0 || _stricmp(item.c_str(), m.folder.c_str()) == 0;
    }

    bool Listed(const std::vector<std::string>& off, const ModsPage::Mod& m)
    {
        for (const std::string& o : off) if (Names(o, m)) return true;
        return false;
    }

    // Rewrites the one line; a mod listed under its folder name is replaced by its id.
    bool WriteOff(const ModsPage::Mod& m, bool off)
    {
        std::string text;
        ReadIni(text);
        std::string value;
        const std::string* v = Ini::Find(Ini::Parse(text), "mods.disabled");
        for (const std::string& o : v ? Ini::List(*v) : std::vector<std::string>{})
            if (!Names(o, m)) value += (value.empty() ? "" : ", ") + o;
        if (off) value += (value.empty() ? "" : ", ") + m.id;
        text = Ini::Set(text, "mods.disabled", value);

        const std::string path = IniPath(), tmp = path + ".tmp";
        FILE* f = nullptr;
        if (fopen_s(&f, tmp.c_str(), "wb") != 0 || !f) return false;
        const bool ok = fwrite(text.data(), 1, text.size(), f) == text.size();
        fclose(f);
        if (!ok || !MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) { DeleteFileA(tmp.c_str()); return false; }
        return true;
    }

    void Gather()
    {
        g_mods.clear();
        const std::vector<std::string> bootOff = Ini::List(Settings::Get("mods.disabled", ""));
        const std::vector<std::string> diskOff = OffOnDisk();
        for (const ModSet::Record& r : Mods::Result().records)
        {
            const Host::Status* s = r.mod.id.empty() || !r.accepted ? nullptr : Host::StatusOf(r.mod.id.c_str());
            const ContentBuild::Outcome c = r.mod.id.empty() ? ContentBuild::Outcome::None : ContentBuild::OutcomeOf(r.mod.id.c_str());
            ModsPage::Mod m;
            m.id      = r.mod.id.empty() ? r.folder : r.mod.id;
            m.version = r.mod.version;
            m.folder  = r.folder;
            m.stage   = ModIni::StageName(r.mod.stage);
            m.state   = StateOf(r, s, c);
            m.note    = m.state == ModsPage::State::Refused ? r.refusal : (m.state == ModsPage::State::Failed && s ? s->note : "");
            m.content = r.mod.id.empty() || !r.accepted ? "" : ContentBuild::Summary(r.mod.id.c_str());
            m.toggle  = Listed(diskOff, m) ? ModsPage::Switch::TurnOn : ModsPage::Switch::TurnOff;
            m.changed = Listed(diskOff, m) != Listed(bootOff, m);
            g_mods.push_back(m);
        }
        g_header.version      = Framework::kVersionString;
        g_header.targetMd5    = GBHOOK_TARGET_MD5;
        g_header.roots        = Ini::List(Settings::Get("mods.root", "mods"));
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
        const bool ok  = WriteOff(m, off);
        if (ok) Log::Writef("MODS", "%s %s in gbhook.ini, from the next start", m.id.c_str(), off ? "disabled" : "enabled");
        else    Log::Writef("MODS", "gbhook.ini could not be written; %s is unchanged", m.id.c_str());
        Gather();
        g_mods[(size_t)g_detail].saveFailed = !ok;
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
        NativeMenu::AddRow("Load Level", kLoadLevel);
        NativeMenu::AddRow("View Mods", kViewMods);
    }
    int ActivateRoot(int action, void*)
    {
        if (action == kLoadLevel) LevelsMenu::Open();
        else if (action == kViewMods)
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
}
