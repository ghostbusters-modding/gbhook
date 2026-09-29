// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// <gamedir>\gbhook.ini through the shared grammar, with each accepted mod's modinfo.ini keys folded in as its defaults.
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
    SRWLOCK               g_lock   = SRWLOCK_INIT;   // mods write their keys while other threads read
    thread_local std::string t_value;

    std::string IniPath() { return std::string(Framework::GameDir()) + "\\gbhook.ini"; }

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

    // Through a temporary and a rename, so a crash mid-write never leaves half a gbhook.ini.
    bool WriteWhole(const std::string& text)
    {
        const std::string path = IniPath(), tmp = path + ".tmp";
        FILE* f = nullptr;
        if (fopen_s(&f, tmp.c_str(), "wb") != 0 || !f) return false;
        const bool ok = fwrite(text.data(), 1, text.size(), f) == text.size();
        fclose(f);
        if (!ok || !MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) { DeleteFileA(tmp.c_str()); return false; }
        return true;
    }

    // A mod's value is copied out: setting_set can replace it under another thread's feet.
    const char* Lookup(const char* id, const char* key, const char* dflt)
    {
        if (!key) return dflt;
        Settings::Load();
        AcquireSRWLockShared(&g_lock);
        const char* v = SettingsTable::Get(*g_table, id, key);
        if (v && id && *id) { t_value = v; v = t_value.c_str(); }
        ReleaseSRWLockShared(&g_lock);
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

        const std::string path = IniPath();
        std::string text;
        if (!ReadWhole(path.c_str(), text))
        {
            Log::Writef("INI", "no gbhook.ini at %s -- defaults for everything", path.c_str());
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

        AcquireSRWLockExclusive(&g_lock);
        *g_table = SettingsTable::Build(*g_doc, defaults);
        ReleaseSRWLockExclusive(&g_lock);
        for (const std::string& w : g_table->warnings) Log::Writef("INI", "warning %s", w.c_str());
        Log::Writef("INI", "%d mod default(s) from %d modinfo.ini file(s)",
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

    bool ReadFile(std::string& text)
    {
        text.clear();
        return ReadWhole(IniPath().c_str(), text);
    }

    bool Rewrite(const std::function<std::string(const std::string&)>& edit)
    {
        AcquireSRWLockExclusive(&g_lock);
        std::string text;
        ReadWhole(IniPath().c_str(), text);
        const bool ok = WriteWhole(edit(text));
        ReleaseSRWLockExclusive(&g_lock);
        return ok;
    }

    int SetFor(const char* id, const char* key, const char* value)
    {
        if (!id || !*id) return GBH_ERR_STATE;
        if (!SettingsTable::Writable(key, value)) return GBH_ERR_ARG;
        Load();
        const std::string q = SettingsTable::Qualify(id, key);

        AcquireSRWLockExclusive(&g_lock);
        std::string text;
        ReadWhole(IniPath().c_str(), text);
        const bool ok = WriteWhole(value ? Ini::Set(text, q, Ini::Written(value)) : Ini::Remove(text, q));
        if (ok && value) SettingsTable::Set(*g_table, id, key, value);
        else if (ok)     SettingsTable::Unset(*g_table, id, key);
        ReleaseSRWLockExclusive(&g_lock);

        if (!ok) { Log::WriteFrom(id, "INI", ("gbhook.ini could not be written; " + q + " is unchanged").c_str()); return GBH_ERR; }
        return GBH_OK;
    }
}
