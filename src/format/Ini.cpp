// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Ini.h"

#include <cctype>
#include <cstdlib>

namespace Ini
{
    std::string Trim(const std::string& s)
    {
        size_t a = s.find_first_not_of(" \t\r\n");
        if (a == std::string::npos) return {};
        size_t b = s.find_last_not_of(" \t\r\n");
        return s.substr(a, b - a + 1);
    }

    std::string Lower(std::string s)
    {
        for (char& c : s) c = (char)tolower((unsigned char)c);
        return s;
    }

    Document Parse(const std::string& text)
    {
        Document doc;
        const size_t npos = std::string::npos;

        // Notepad writes a BOM, and it is not part of the first key.
        size_t pos = text.compare(0, 3, "\xEF\xBB\xBF") == 0 ? 3 : 0;

        std::string section;
        int line = 0;
        while (pos <= text.size())
        {
            size_t nl = text.find('\n', pos);
            std::string s = text.substr(pos, nl == npos ? npos : nl - pos);
            pos = nl == npos ? text.size() + 1 : nl + 1;
            ++line;

            size_t cut = s.find_first_of("#;");
            if (cut != npos) s.erase(cut);
            s = Trim(s);
            if (s.empty()) continue;

            if (s.front() == '[')
            {
                if (s.back() != ']') { doc.malformed.push_back(line); continue; }
                section = Lower(Trim(s.substr(1, s.size() - 2)));
                continue;
            }

            size_t eq = s.find('=');
            if (eq == npos) { doc.malformed.push_back(line); continue; }

            std::string key = Lower(Trim(s.substr(0, eq)));
            if (key.empty()) { doc.malformed.push_back(line); continue; }
            if (!section.empty()) key = section + "." + key;

            doc.entries.push_back({ key, Trim(s.substr(eq + 1)), line });
        }
        return doc;
    }

    const std::string* Find(const Document& doc, const std::string& key)
    {
        const std::string  k     = Lower(key);
        const std::string* found = nullptr;
        for (const Entry& e : doc.entries)
            if (e.key == k) found = &e.value;
        return found;
    }

    std::vector<std::string> List(const std::string& value)
    {
        std::vector<std::string> out;
        size_t pos = 0;
        for (;;)
        {
            size_t comma = value.find(',', pos);
            std::string item = Trim(value.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos));
            if (!item.empty()) out.push_back(item);
            if (comma == std::string::npos) break;
            pos = comma + 1;
        }
        return out;
    }

    int ToInt(const std::string& v, int dflt)
    {
        const char* s   = v.c_str();
        char*       end = nullptr;
        long        n   = strtol(s, &end, 0);
        return end == s ? dflt : (int)n;
    }

    float ToFloat(const std::string& v, float dflt)
    {
        const char* s   = v.c_str();
        char*       end = nullptr;
        float       f   = strtof(s, &end);
        return end == s ? dflt : f;
    }

    bool ToBool(const std::string& v, bool dflt)
    {
        if (v.empty()) return dflt;
        const std::string l = Lower(v);
        // "off" starts the way "on" does, so it is spelled out; the rest goes by first letter.
        if (l == "off") return false;
        return !(l[0] == '0' || l[0] == 'f' || l[0] == 'n');
    }
}
