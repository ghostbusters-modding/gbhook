// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ModIni.h"
#include "Ini.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace
{
    std::string Fmt(const char* fmt, ...)
    {
        char b[512];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(b, sizeof b, fmt, ap);
        va_end(ap);
        return b;
    }

    // The whole string must be the number: "12abc" is a typo here, not 12.
    bool StrictInt(const std::string& s, int* out)
    {
        if (s.empty()) return false;
        char* end = nullptr;
        long  v   = strtol(s.c_str(), &end, 10);
        if (end == s.c_str() || *end != '\0') return false;
        *out = (int)v;
        return true;
    }

    // Absolute, drive-rooted, or carrying a ".." component: anything that can resolve outside gbhook/.
    bool LeavesFolder(const std::string& p)
    {
        if (p.empty()) return false;
        if (p[0] == '/' || p[0] == '\\') return true;
        if (p.size() >= 2 && p[1] == ':') return true;
        size_t i = 0;
        while (i <= p.size())
        {
            size_t j = p.find_first_of("/\\", i);
            if (j == std::string::npos) j = p.size();
            if (j - i == 2 && p[i] == '.' && p[i + 1] == '.') return true;
            i = j + 1;
        }
        return false;
    }

    const char* const kKnown[] = {
        "id", "abi", "plugin", "scripts", "content", "stage", "priority", "requires",
    };
}

namespace ModIni
{
    const char* StageName(GbhStage s)
    {
        switch (s)
        {
        case GBH_STAGE_PREBOOT: return "preboot";
        case GBH_STAGE_EARLY:   return "early";
        case GBH_STAGE_BOOT:    return "boot";
        case GBH_STAGE_READY:   return "ready";
        }
        return "?";
    }

    bool StageFromName(const std::string& name, GbhStage* out)
    {
        const std::string l = Ini::Lower(name);
        for (int i = 0; i < GBH_STAGE_COUNT; ++i)
            if (l == StageName((GbhStage)i)) { *out = (GbhStage)i; return true; }
        return false;
    }

    Result Parse(const std::string& text)
    {
        Result r;
        Ini::Document d = Ini::Parse(text);
        for (const Ini::Entry& e : d.entries)
            for (const char* k : kKnown) if (e.key == k) r.gbhook = true;

        // An empty value reads as absent, so "id =" is the same mistake as no id line.
        auto get = [&](const char* key) -> const std::string*
        {
            const std::string* v = Ini::Find(d, key);
            return (v && !v->empty()) ? v : nullptr;
        };

        const std::string* v = get("version");
        if (v) r.mod.version = *v;
        if ((v = get("description"))) r.mod.description = *v;
        if (!r.gbhook) return r;

        if (!d.malformed.empty())
        {
            r.refusal = Fmt("modinfo.ini line %d is not key = value", d.malformed[0]);
            return r;
        }

        // Every key doubles as a setting default, the Mod Manager's included.
        for (const Ini::Entry& e : d.entries)
        {
            if (e.key == "disabled")
                r.warnings.push_back(Fmt("line %d: 'disabled' is no longer read; list the mod under mods_disabled in gbhook.ini", e.line));
            else
                r.mod.settings.emplace_back(e.key, e.value);
        }

        v = get("id");
        if (!v) { r.refusal = "modinfo.ini has no id"; return r; }
        if (v->find_first_of(" \t") != std::string::npos) { r.refusal = Fmt("id '%s' contains whitespace", v->c_str()); return r; }
        // A setting or command is <id>.<name>, split at the first dot.
        if (v->find('.') != std::string::npos) { r.refusal = Fmt("id '%s' contains a dot", v->c_str()); return r; }
        if (v->size() > 63) { r.refusal = "id is longer than 63 characters"; return r; }
        r.mod.id = *v;

        v = get("abi");
        if (!v) { r.refusal = "modinfo.ini has no abi"; return r; }
        if (!StrictInt(*v, &r.mod.abi)) { r.refusal = Fmt("abi '%s' is not a number (the integer GBHOOK_ABI_VERSION)", v->c_str()); return r; }
        if (r.mod.abi != GBHOOK_ABI_VERSION)
        {
            r.refusal = Fmt("built for ABI %d, this gbhook speaks ABI %d -- rebuild the mod", r.mod.abi, (int)GBHOOK_ABI_VERSION);
            return r;
        }

        if ((v = get("plugin")))
        {
            if (LeavesFolder(*v)) { r.refusal = Fmt("plugin '%s' leaves gbhook/", v->c_str()); return r; }
            r.mod.plugin = *v;
        }
        if ((v = get("scripts")))
        {
            if (LeavesFolder(*v)) { r.refusal = Fmt("scripts '%s' leaves gbhook/", v->c_str()); return r; }
            r.mod.scripts = *v;
        }
        if ((v = get("content")))
        {
            for (const std::string& item : Ini::List(*v))
            {
                if (LeavesFolder(item)) { r.refusal = Fmt("content '%s' leaves gbhook/", item.c_str()); return r; }
                r.mod.content.push_back(item);
            }
        }

        if ((v = get("stage")) && !StageFromName(*v, &r.mod.stage))
        {
            r.refusal = Fmt("stage '%s' is not one of preboot, early, boot, ready", v->c_str());
            return r;
        }
        if ((v = get("priority")) && !StrictInt(*v, &r.mod.priority))
        {
            r.refusal = Fmt("priority '%s' is not a number", v->c_str());
            return r;
        }
        if ((v = get("requires"))) r.mod.requires_ = Ini::List(*v);

        return r;
    }
}
