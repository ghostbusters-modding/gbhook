// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// <gamedir>\gbhook.ini through the shared grammar, with each accepted mod's [settings] folded in as its defaults.
// Not GetPrivateProfileString: that needs sections, and modset/SettingsTable is where the two layers meet.

#include "Framework.h"
#include "format/Ini.h"
#include "modset/ModSet.h"
#include "modset/SettingsTable.h"

#include <cstdio>
#include <string>
#include <vector>

namespace
{
    Ini::Document*        g_doc    = nullptr;
    SettingsTable::Table* g_table  = nullptr;
    bool                  g_loaded = false;

    bool ReadWhole(const char* path, std::string& out)
    {
        FILE* f = nullptr;
        if (fopen_s(&f, path, "rb") != 0 || !f) return false;
        char   buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof buf, f)) > 0) out.append(buf, n);
        fclose(f);
        return true;
    }

    const char* Lookup(const char* id, const char* key, const char* dflt)
    {
        if (!key) return dflt;
        Settings::Load();
        const char* v = SettingsTable::Get(*g_table, id, key);
        return v ? v : dflt;
    }
}

namespace Settings
{
    void Load()
    {
        if (g_loaded) return;
        g_loaded = true;
        g_doc   = new Ini::Document();
        g_table = new SettingsTable::Table();

        char path[MAX_PATH];
        _snprintf_s(path, sizeof path, _TRUNCATE, "%s\\gbhook.ini", Framework::GameDir());

        std::string text;
        if (!ReadWhole(path, text))
        {
            Log::Writef("INI", "no gbhook.ini at %s -- defaults for everything", path);
            return;
        }

        *g_doc   = Ini::Parse(text);
        *g_table = SettingsTable::Build(*g_doc, {});
        for (int line : g_doc->malformed)
            Log::Writef("INI", "gbhook.ini line %d is not key = value; ignored", line);
        Log::Writef("INI", "%d settings from gbhook.ini", (int)g_doc->entries.size());
    }

    void Attach(const ModSet::Result& mods)
    {
        Load();
        std::vector<SettingsTable::Defaults> defaults;
        for (const ModSet::Record& r : mods.records)
            if (r.accepted && !r.mod.settings.empty()) defaults.push_back({ r.mod.id, r.mod.settings });

        *g_table = SettingsTable::Build(*g_doc, defaults);
        for (const std::string& w : g_table->warnings) Log::Writef("INI", "warning %s", w.c_str());
        Log::Writef("INI", "%d mod default(s) from %d modinfo.ini [settings] block(s)",
                    (int)g_table->defaults.size(), (int)defaults.size());
    }

    const char* Get(const char* key, const char* dflt)    { return Lookup(nullptr, key, dflt); }
    int         GetInt(const char* key, int dflt)         { return GetIntFor(nullptr, key, dflt); }
    float       GetFloat(const char* key, float dflt)     { return GetFloatFor(nullptr, key, dflt); }
    bool        GetBool(const char* key, bool dflt)       { return GetBoolFor(nullptr, key, dflt); }

    const char* GetFor(const char* id, const char* key, const char* dflt) { return Lookup(id, key, dflt); }

    int GetIntFor(const char* id, const char* key, int dflt)
    {
        const char* v = Lookup(id, key, nullptr);
        return v ? Ini::ToInt(v, dflt) : dflt;
    }

    float GetFloatFor(const char* id, const char* key, float dflt)
    {
        const char* v = Lookup(id, key, nullptr);
        return v ? Ini::ToFloat(v, dflt) : dflt;
    }

    bool GetBoolFor(const char* id, const char* key, bool dflt)
    {
        const char* v = Lookup(id, key, nullptr);
        return v ? Ini::ToBool(v, dflt) : dflt;
    }
}
