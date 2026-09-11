// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Dante.h"

#include <cstring>

namespace
{
    const char kPrefix[] = "void checkpoint_";
    const char kSuffix[] = "()";

    bool StartsWith(const std::string& s, const char* p) { return s.compare(0, strlen(p), p) == 0; }
}

namespace Dante
{
    std::vector<std::string> Checkpoints(const char* text, size_t n)
    {
        std::vector<std::string> out;
        if (!text) return out;
        const std::string all(text, n);
        const size_t begin = all.find("BEGIN STRINGS");
        if (begin == std::string::npos) return out;
        const size_t end = all.find("END STRINGS", begin);
        if (end == std::string::npos) return out;

        size_t at = begin;
        while (at < end)
        {
            size_t eol = all.find('\n', at);
            if (eol == std::string::npos || eol > end) eol = end;
            std::string line = all.substr(at, eol - at);
            at = eol + 1;

            // <hex offset> "<string>"; the prototype text carries no escapes, so the quotes bound it.
            const size_t q1 = line.find('"');
            const size_t q2 = line.rfind('"');
            if (q1 == std::string::npos || q2 <= q1) continue;
            const std::string s = line.substr(q1 + 1, q2 - q1 - 1);
            if (!StartsWith(s, kPrefix) || s.size() <= strlen(kPrefix) + strlen(kSuffix)) continue;
            if (s.compare(s.size() - strlen(kSuffix), strlen(kSuffix), kSuffix) != 0) continue;

            const std::string name = s.substr(strlen("void "), s.size() - strlen("void ") - strlen(kSuffix));
            bool dup = false;
            for (const std::string& o : out) if (o == name) { dup = true; break; }
            if (!dup) out.push_back(name);
        }
        return out;
    }
}
