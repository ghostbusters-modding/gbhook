// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The one ini grammar behind gbhook.ini and modinfo.ini. No windows.h; tests/format/test_ini.cpp is the spec.

#include <string>
#include <vector>

namespace Ini
{
    struct Entry
    {
        std::string key;     // lowercased
        std::string value;   // trimmed, comment stripped, case kept
        int         line;    // 1-based, for refusal text
    };

    struct Document
    {
        std::vector<Entry> entries;    // file order, duplicates kept
        std::vector<int>   malformed;  // lines that are none of blank, comment, key = value
    };

    Document Parse(const std::string& text);

    // Last value for `key`, any case, or nullptr.
    const std::string* Find(const Document& doc, const std::string& key);

    // `text` with the last `key` line set to `value`, its comment kept. A new key goes at the end.
    std::string Set(const std::string& text, const std::string& key, const std::string& value);

    // Comma-separated list, items trimmed, empties dropped.
    std::vector<std::string> List(const std::string& value);

    // List of the last `key` in `text`, empty when absent. By value: Find into a temporary Parse dangles.
    std::vector<std::string> ListOf(const std::string& text, const std::string& key);

    // Typed readers. Empty or unparsable yields the default.
    int   ToInt(const std::string& v, int dflt);
    float ToFloat(const std::string& v, float dflt);
    bool  ToBool(const std::string& v, bool dflt);

    std::string Trim(const std::string& s);
    std::string Lower(std::string s);
}
