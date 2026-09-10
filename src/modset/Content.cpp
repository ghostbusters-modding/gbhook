// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Content.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace
{
    // The engine compares paths case-insensitively with a backslash separator.
    std::string Key(std::string s)
    {
        for (char& c : s) { if (c == '/') c = '\\'; c = (char)tolower((unsigned char)c); }
        return s;
    }
}

namespace Content
{
    Verdict Decide(const Input& in)
    {
        Verdict v;

        if (in.loose.empty()) { v.action = Action::Skip; v.reason = "no loose content"; return v; }

        if (in.manualDisabled) { v.action = Action::Disabled; v.reason = "disabled in mod.ini"; return v; }

        if (in.chainPaths && !in.chainPaths->empty())
        {
            std::unordered_set<std::string> have;
            have.reserve(in.chainPaths->size());
            for (const std::string& p : *in.chainPaths) have.insert(Key(p));

            bool allInChain = true;
            for (const TreeHash::File& f : in.loose)
                if (!have.count(Key(f.relpath))) { allInChain = false; break; }

            if (allInChain)
            {
                v.action = Action::Disabled;
                v.reason = "already in a chained POD -- the Mod Manager deployed it, gbhook stands down";
                return v;
            }
        }

        v.hash = TreeHash::Of(in.loose);
        if (!in.cachedHash.empty() && in.cachedHash == v.hash)
        {
            v.action = Action::MountCached;
            v.reason = "unchanged since the cache was built";
        }
        else
        {
            v.action = Action::Build;
            v.reason = in.cachedHash.empty() ? "no cache yet" : "loose tree changed since the cache was built";
        }
        return v;
    }
}
