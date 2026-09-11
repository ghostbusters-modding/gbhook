// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "SettingsTable.h"

namespace SettingsTable
{
    std::string Qualify(const char* id, const char* key)
    {
        std::string k = Ini::Lower(key ? key : "");
        if (!id || !*id) return k;
        return Ini::Lower(id) + "." + k;
    }

    Table Build(const Ini::Document& ini, const std::vector<Defaults>& mods)
    {
        Table t;
        for (const Ini::Entry& e : ini.entries) t.ini[e.key] = e.value;

        for (const Defaults& m : mods)
        {
            const std::string prefix = Ini::Lower(m.id) + ".";
            for (const auto& kv : m.settings)
            {
                const std::string key = Ini::Lower(kv.first);
                // The id is the namespace already; a key carrying it again is read doubled, which is never meant.
                if (key.compare(0, prefix.size(), prefix) == 0)
                    t.warnings.push_back(m.id + ": [settings] key '" + kv.first + "' repeats the mod id; it is read as '" +
                                         prefix + key + "'");
                t.defaults[prefix + key] = kv.second;
            }
        }
        return t;
    }

    const char* Get(const Table& t, const char* id, const char* key)
    {
        if (!key || !*key) return nullptr;
        const std::string q = Qualify(id, key);
        auto it = t.ini.find(q);
        if (it != t.ini.end()) return it->second.c_str();
        if (!id || !*id) return nullptr;
        it = t.defaults.find(q);
        return it == t.defaults.end() ? nullptr : it->second.c_str();
    }
}
