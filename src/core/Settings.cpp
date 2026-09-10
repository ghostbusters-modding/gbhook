// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// <gamedir>\gbhook.ini through the shared grammar in format/Ini.h. Not GetPrivateProfileString: that needs sections.

#include "Framework.h"
#include "format/Ini.h"

#include <cstdio>
#include <string>
#include <unordered_map>

namespace
{
    std::unordered_map<std::string, std::string>* g_map = nullptr;
    bool g_loaded = false;

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
}

namespace Settings
{
    void Load()
    {
        if (g_loaded) return;
        g_loaded = true;
        g_map = new std::unordered_map<std::string, std::string>();

        char path[MAX_PATH];
        _snprintf_s(path, sizeof path, _TRUNCATE, "%s\\gbhook.ini", Framework::GameDir());

        std::string text;
        if (!ReadWhole(path, text))
        {
            Log::Writef("INI", "no gbhook.ini at %s -- defaults for everything", path);
            return;
        }

        Ini::Document doc = Ini::Parse(text);
        for (const Ini::Entry& e : doc.entries) (*g_map)[e.key] = e.value;
        for (int line : doc.malformed)
            Log::Writef("INI", "gbhook.ini line %d is not key = value; ignored", line);
        Log::Writef("INI", "%d settings from gbhook.ini", (int)doc.entries.size());
    }

    const char* Get(const char* key, const char* dflt)
    {
        if (!key) return dflt;
        Load();
        auto it = g_map->find(Ini::Lower(key));
        return it == g_map->end() ? dflt : it->second.c_str();
    }

    int GetInt(const char* key, int dflt)
    {
        const char* v = Get(key, nullptr);
        return v ? Ini::ToInt(v, dflt) : dflt;
    }

    float GetFloat(const char* key, float dflt)
    {
        const char* v = Get(key, nullptr);
        return v ? Ini::ToFloat(v, dflt) : dflt;
    }

    bool GetBool(const char* key, bool dflt)
    {
        const char* v = Get(key, nullptr);
        return v ? Ini::ToBool(v, dflt) : dflt;
    }
}
