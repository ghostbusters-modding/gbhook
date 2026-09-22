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

    // Every top folder and extension found across the retail archives (COMMON, W64*, LANGUAGE, PATCH), and no
    // other. Source formats the exe also names (.tga .mtl .smf .hbt .dvm) are dev-build inputs and stay out.
    const char* const kRoots[] = {
        "animations", "art", "cinemats", "data", "fx", "materials",
        "models", "physics", "sets", "skeletal", "sound", "world",
    };
    const char* const kTypes[] = {
        ".ani", ".bfm", ".bst", ".cib", ".cinemat", ".dante", ".fnt", ".fxa", ".fxe", ".hbb", ".jug", ".lvl",
        ".mtb", ".phys2b", ".sbs", ".sec", ".skb", ".smb", ".smp", ".snb", ".subb", ".tex", ".tfb", ".txt", ".ui",
    };
}

namespace Content
{
    bool IsAssetRoot(const std::string& topFolder)
    {
        const std::string k = Key(topFolder);
        for (const char* r : kRoots) if (k == r) return true;
        return false;
    }

    Kind Classify(const std::string& relpath)
    {
        const std::string k = Key(relpath);
        const size_t slash = k.find('\\');
        if (slash == std::string::npos || !IsAssetRoot(k.substr(0, slash))) return Kind::NotAssetRoot;

        const size_t dot = k.rfind('.');
        if (dot == std::string::npos || dot < k.rfind('\\')) return Kind::NotAssetType;
        const std::string ext = k.substr(dot);
        for (const char* t : kTypes) if (ext == t) return Kind::Asset;
        return Kind::NotAssetType;
    }

    Verdict Decide(const Input& in)
    {
        Verdict v;

        if (in.loose.empty()) { v.action = Action::Skip; v.reason = "no loose content"; return v; }

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
                v.action = Action::InChain;
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
