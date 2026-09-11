// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ContentBuild.h"
#include "Discovery.h"
#include "../core/Framework.h"
#include "modset/Content.h"
#include "pod/Pod.h"
#include "pod/TreeHash.h"
#include "services/Pods.h"

#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>

namespace
{
    std::vector<ContentBuild::Planned> g_planned;
    bool g_done    = false;
    bool g_mounted = false;

    bool IsDir(const std::string& p)
    {
        DWORD a = GetFileAttributesA(p.c_str());
        return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
    }

    void MakeDirs(const std::string& path)
    {
        std::string acc;
        for (size_t i = 0; i < path.size(); ++i)
        {
            acc += path[i];
            if (path[i] == '\\' || i + 1 == path.size()) CreateDirectoryA(acc.c_str(), nullptr);
        }
    }

    // Recursively list a mod's loose tree, relpaths in the engine's backslash form, skipping gbhook/ and previews/.
    void WalkLoose(const std::string& root, const std::string& rel, std::vector<TreeHash::File>& out)
    {
        const std::string dir = rel.empty() ? root : root + "\\" + rel;
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA((dir + "\\*").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) return;
        do
        {
            const std::string name = fd.cFileName;
            if (name == "." || name == "..") continue;
            const std::string childRel = rel.empty() ? name : rel + "\\" + name;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (rel.empty() && (_stricmp(name.c_str(), "gbhook") == 0 || _stricmp(name.c_str(), "previews") == 0))
                    continue;
                WalkLoose(root, childRel, out);
            }
            else
            {
                TreeHash::File f;
                f.relpath = childRel;
                f.size  = ((uint64_t)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
                f.mtime = ((uint64_t)fd.ftLastWriteTime.dwHighDateTime << 32) | fd.ftLastWriteTime.dwLowDateTime;
                out.push_back(f);
            }
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }

    // The hash of whatever cache POD is already there, or "" -- the file name is the hash.
    std::string CachedHash(const std::string& cacheDir)
    {
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA((cacheDir + "\\*.POD").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) return "";
        std::string name = fd.cFileName;
        FindClose(h);
        size_t dot = name.rfind('.');
        return dot == std::string::npos ? name : name.substr(0, dot);
    }

    void PruneCache(const std::string& cacheDir)
    {
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA((cacheDir + "\\*.POD").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) return;
        do DeleteFileA((cacheDir + "\\" + fd.cFileName).c_str());
        while (FindNextFileA(h, &fd));
        FindClose(h);
    }

    // Read a whole file into `buf`; the source handed to the POD writer.
    bool ReadFile(const std::string& path, std::vector<uint8_t>& buf)
    {
        FILE* f = nullptr;
        if (fopen_s(&f, path.c_str(), "rb") != 0 || !f) return false;
        char tmp[8192];
        size_t n;
        buf.clear();
        while ((n = fread(tmp, 1, sizeof tmp, f)) > 0) buf.insert(buf.end(), tmp, tmp + n);
        fclose(f);
        return true;
    }

    // Every file name reachable through PATCH.POD's chain, engine backslash form. Empty if there is no chain.
    void ScanChain(std::vector<std::string>& names)
    {
        const std::string dir = Framework::GameDir();
        std::string cur = "PATCH.POD";
        int guard = 0;
        std::vector<std::string> seen;
        while (guard++ < 100 && !cur.empty())
        {
            const std::string full = dir + "\\" + cur;
            FILE* f = nullptr;
            if (fopen_s(&f, full.c_str(), "rb") != 0 || !f) break;   // a missing target silently ends the chain
            std::vector<uint8_t> bytes;
            char tmp[65536];
            size_t n;
            while ((n = fread(tmp, 1, sizeof tmp, f)) > 0) bytes.insert(bytes.end(), tmp, tmp + n);
            fclose(f);

            Pod::Archive a;
            std::string why;
            if (!Pod::Read(bytes.data(), bytes.size(), a, &why))
            {
                Log::Writef("MODS", "chain: %s is not a readable POD (%s)", cur.c_str(), why.c_str());
                break;
            }
            for (const Pod::IndexEntry& e : a.entries) names.push_back(e.name);

            bool cycle = false;
            for (const std::string& s : seen) if (_stricmp(s.c_str(), cur.c_str()) == 0) cycle = true;
            if (cycle) break;
            seen.push_back(cur);
            cur = a.nextPod;
        }
    }
}

namespace ContentBuild
{
    const std::vector<Planned>& Build()
    {
        if (g_done) return g_planned;
        g_done = true;

        std::vector<std::string> chain;
        ScanChain(chain);
        if (!chain.empty())
            Log::Writef("MODS", "PATCH.POD chain holds %d file(s) -- a mod already in it is left to the Mod Manager",
                        (int)chain.size());

        const std::string gameDir = Framework::GameDir();
        int built = 0, cached = 0, disabled = 0, skipped = 0;

        for (const ModSet::Record& r : Mods::Result().records)
        {
            if (!r.accepted) continue;

            const std::string modRoot = r.root + "\\" + r.folder;
            std::vector<TreeHash::File> loose;
            WalkLoose(modRoot, "", loose);

            const std::string cacheDir = gameDir + "\\gbhook\\cache\\" + r.mod.id;

            Content::Input in;
            in.id             = r.mod.id;
            in.loose          = loose;
            in.manualDisabled = r.mod.disabled;
            in.cachedHash     = IsDir(cacheDir) ? CachedHash(cacheDir) : "";
            in.chainPaths     = &chain;

            Content::Verdict v = Content::Decide(in);
            switch (v.action)
            {
            case Content::Action::Skip:
                ++skipped;
                break;

            case Content::Action::Disabled:
                ++disabled;
                Log::Writef("MODS", "content %s: off -- %s", r.mod.id.c_str(), v.reason.c_str());
                break;

            case Content::Action::MountCached:
            {
                ++cached;
                const std::string rel = "gbhook\\cache\\" + r.mod.id + "\\" + v.hash + ".POD";
                Log::Writef("MODS", "content %s: cached, %s", r.mod.id.c_str(), rel.c_str());
                g_planned.push_back({ r.mod.id, rel });
                break;
            }

            case Content::Action::Build:
            {
                MakeDirs(cacheDir);
                PruneCache(cacheDir);

                std::vector<Pod::Item> items;
                items.reserve(loose.size());
                for (const TreeHash::File& lf : loose)
                {
                    const std::string abs = modRoot + "\\" + lf.relpath;
                    items.push_back({ lf.relpath, [abs](std::vector<uint8_t>& out) { return ReadFile(abs, out); } });
                }

                const std::string relPod = "gbhook\\cache\\" + r.mod.id + "\\" + v.hash + ".POD";
                const std::string absPod = gameDir + "\\" + relPod;
                Pod::FileSink sink;
                std::string why;
                // Revision 1: overrides the bulk archives, loses to a rev-0 chained MODS.POD, so GBMM still wins.
                if (sink.Open(absPod.c_str()) && Pod::Write(sink, items, 1, "", &why))
                {
                    sink.Close();
                    ++built;
                    Log::Writef("MODS", "content %s: built %d file(s) -> %s", r.mod.id.c_str(), (int)loose.size(), relPod.c_str());
                    g_planned.push_back({ r.mod.id, relPod });
                }
                else
                {
                    sink.Close();
                    DeleteFileA(absPod.c_str());
                    Log::Writef("MODS", "content %s: build FAILED -- %s", r.mod.id.c_str(), why.c_str());
                }
                break;
            }
            }
        }

        Log::Writef("MODS", "content: %d built, %d cached, %d disabled, %d without content",
                    built, cached, disabled, skipped);
        return g_planned;
    }

    const std::vector<Planned>& Plans()  { return g_planned; }

    bool Mount()
    {
        if (g_mounted) return true;
        if (!Pods::Ready()) return false;
        g_mounted = true;

        int ok = 0;
        for (const Planned& p : g_planned)
        {
            const char* err = nullptr;
            if (Pods::Mount(p.cachePod.c_str(), &err)) ++ok;
            else Log::Writef("MODS", "content %s: mount FAILED -- %s", p.id.c_str(), err ? err : "no reason given");
        }
        Log::Writef("MODS", "content: %d of %d archive(s) mounted", ok, (int)g_planned.size());
        return true;
    }
}
