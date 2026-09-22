// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ContentBuild.h"
#include "Discovery.h"
#include "../core/Framework.h"
#include "modset/Content.h"
#include "pod/Pod.h"
#include "pod/TreeHash.h"
#include "services/FrameHook.h"
#include "services/Pods.h"
#include "services/Pump.h"

#include <windows.h>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    struct Planned
    {
        std::string id;
        std::string cachePod;   // <gamedir>-relative path to the cache POD, ready for Pods::Mount
    };

    // Shared between the build thread and the pump: the queue of archives ready to mount, and the summaries.
    CRITICAL_SECTION g_lock;
    bool             g_lockReady = false;
    std::vector<Planned> g_queue;
    std::unordered_map<std::string, std::string> g_summary;
    bool          g_parked      = false;   // a mount job is on the pump and will drain the queue
    volatile LONG g_started     = 0;
    volatile LONG g_buildDone   = 0;
    volatile LONG g_allMounted  = 0;
    int           g_mountOk     = 0;
    int           g_mountTotal  = 0;

    void EnsureLock()
    {
        if (g_lockReady) return;
        InitializeCriticalSection(&g_lock);
        g_lockReady = true;
    }

    void SetSummary(const std::string& id, const std::string& text)
    {
        EnterCriticalSection(&g_lock);
        g_summary[id] = text;
        LeaveCriticalSection(&g_lock);
    }

    void AppendSummary(const std::string& id, const std::string& text)
    {
        EnterCriticalSection(&g_lock);
        g_summary[id] += text;
        LeaveCriticalSection(&g_lock);
    }

    bool MountJob(void*);

    // Park the mount job unless one is already waiting. Outside g_lock: the pump has a lock of its own.
    void ParkMount()
    {
        bool park = false;
        EnterCriticalSection(&g_lock);
        if (!g_parked) { g_parked = true; park = true; }
        LeaveCriticalSection(&g_lock);
        if (park && !Pump::Park(&MountJob, nullptr, "content mount"))
        {
            EnterCriticalSection(&g_lock);
            g_parked = false;
            LeaveCriticalSection(&g_lock);
        }
    }

    void Enqueue(const std::string& id, const std::string& cachePod)
    {
        EnterCriticalSection(&g_lock);
        g_queue.push_back({ id, cachePod });
        LeaveCriticalSection(&g_lock);
        ParkMount();
    }

    // Main thread, from the pump. Mounts whatever is queued and return
    bool MountJob(void*)
    {
        if (!Pods::Ready()) return false;
        if (FrameHook::TickRecently())
        {
            EnterCriticalSection(&g_lock);
            g_parked = false;
            LeaveCriticalSection(&g_lock);
            ParkMount();
            return true;
        }

        std::vector<Planned> batch;
        EnterCriticalSection(&g_lock);
        batch.swap(g_queue);
        g_parked = false;
        LeaveCriticalSection(&g_lock);

        for (const Planned& p : batch)
        {
            const char* err = nullptr;
            ++g_mountTotal;
            if (Pods::Mount(p.cachePod.c_str(), &err)) { ++g_mountOk; AppendSummary(p.id, ", mounted"); }
            else
            {
                Log::Writef("MODS", "content %s: mount FAILED -- %s", p.id.c_str(), err ? err : "no reason given");
                AppendSummary(p.id, std::string(", mount failed: ") + (err ? err : "no reason given"));
            }
        }

        bool finished = false;
        EnterCriticalSection(&g_lock);
        if (g_buildDone && g_queue.empty() && !g_allMounted) { g_allMounted = 1; finished = true; }
        LeaveCriticalSection(&g_lock);
        if (finished) Log::Writef("MODS", "content: %d of %d archive(s) mounted", g_mountOk, g_mountTotal);
        return true;
    }

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

    // What the walk left out of one mod, for the log: root entries in one line, strays under a root one each.
    struct LeftOut
    {
        std::vector<std::string> roots;    // "gen\\", "README.md": not an asset root, so never descended
        std::vector<std::string> strays;   // "art\\crate.png": under a root, but not a type the engine reads
    };

    // Recursively list a mod's asset files, relpaths in the engine's backslash form. Only the engine's asset
    // roots are entered, so gbhook/, previews/ and a generator tree cost nothing beyond their name.
    void WalkLoose(const std::string& root, const std::string& rel, std::vector<TreeHash::File>& out, LeftOut& left)
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
                if (rel.empty() && !Content::IsAssetRoot(name)) { left.roots.push_back(name + "\\"); continue; }
                WalkLoose(root, childRel, out, left);
            }
            else
            {
                switch (Content::Classify(childRel))
                {
                case Content::Kind::NotAssetRoot: left.roots.push_back(name); continue;
                case Content::Kind::NotAssetType: left.strays.push_back(childRel); continue;
                case Content::Kind::Asset: break;
                }
                TreeHash::File f;
                f.relpath = childRel;
                f.size  = ((uint64_t)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
                f.mtime = ((uint64_t)fd.ftLastWriteTime.dwHighDateTime << 32) | fd.ftLastWriteTime.dwLowDateTime;
                out.push_back(f);
            }
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }

    std::string Join(const std::vector<std::string>& v)
    {
        std::string s;
        for (const std::string& x : v) { if (!s.empty()) s += ", "; s += x; }
        return s;
    }

    bool EndsWith(const std::string& s, const char* suffix)
    {
        const size_t n = strlen(suffix);
        return s.size() >= n && _stricmp(s.c_str() + s.size() - n, suffix) == 0;
    }

    // Every file in the cache folder ending in `suffix`. Matched here, not by FindFirstFile: its "*.POD" is 8.3-loose.
    std::vector<std::string> ListCache(const std::string& cacheDir, const char* suffix)
    {
        std::vector<std::string> names;
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA((cacheDir + "\\*").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) return names;
        do if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && EndsWith(fd.cFileName, suffix)) names.push_back(fd.cFileName);
        while (FindNextFileA(h, &fd));
        FindClose(h);
        return names;
    }

    // The hash the cache POD was built from, or "" when there is none worth mounting. The name is the hash, but
    // the name alone once passed a POD a dead thread had left half written, so the layout is checked too.
    std::string CachedHash(const std::string& id, const std::string& cacheDir)
    {
        for (const std::string& tmp : ListCache(cacheDir, ".POD.tmp")) DeleteFileA((cacheDir + "\\" + tmp).c_str());

        const std::vector<std::string> pods = ListCache(cacheDir, ".POD");
        if (pods.empty()) return "";
        const std::string& name = pods.front();

        std::string why;
        FILE* f = nullptr;
        uint8_t hdr[Pod::kHeader];
        if (fopen_s(&f, (cacheDir + "\\" + name).c_str(), "rb") != 0 || !f) why = "cannot be opened";
        else
        {
            const size_t got = fread(hdr, 1, sizeof hdr, f);
            _fseeki64(f, 0, SEEK_END);
            const long long size = _ftelli64(f);
            fclose(f);
            Pod::CheckLayout(hdr, got, size < 0 ? 0 : (uint64_t)size, &why);
        }
        if (!why.empty())
        {
            Log::Writef("MODS", "content %s: cached %s is unusable (%s), rebuilding", id.c_str(), name.c_str(), why.c_str());
            return "";
        }
        const size_t dot = name.find('.');
        return dot == std::string::npos ? name : name.substr(0, dot);
    }

    void PruneCache(const std::string& cacheDir)
    {
        for (const std::string& n : ListCache(cacheDir, ".POD"))     DeleteFileA((cacheDir + "\\" + n).c_str());
        for (const std::string& n : ListCache(cacheDir, ".POD.tmp")) DeleteFileA((cacheDir + "\\" + n).c_str());
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

    // A cache folder whose mod is gone is gbhook's own dead weight. A refused or disabled mod keeps its cache:
    // the hash makes that free, and it is used again the moment the mod is fixed or switched back on.
    void PruneOrphans(const std::string& cacheRoot)
    {
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA((cacheRoot + "\\*").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) return;
        std::vector<std::string> orphans;
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || fd.cFileName[0] == '.') continue;
            bool known = false;
            for (const ModSet::Record& r : Mods::Result().records)
                if (!r.mod.id.empty() && _stricmp(r.mod.id.c_str(), fd.cFileName) == 0) { known = true; break; }
            if (!known) orphans.push_back(fd.cFileName);
        } while (FindNextFileA(h, &fd));
        FindClose(h);

        for (const std::string& id : orphans)
        {
            const std::string dir = cacheRoot + "\\" + id;
            PruneCache(dir);
            if (RemoveDirectoryA(dir.c_str())) Log::Writef("MODS", "cache for '%s' removed: no such mod in any root", id.c_str());
            else Log::Writef("MODS", "cache for '%s' has no mod behind it, but its folder would not go (error %lu)", id.c_str(), GetLastError());
        }
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

namespace
{
    void Build()
    {
        std::vector<std::string> chain;
        ScanChain(chain);
        if (!chain.empty())
            Log::Writef("MODS", "PATCH.POD chain holds %d file(s) -- a mod already in it is left to the Mod Manager",
                        (int)chain.size());

        const std::string gameDir = Framework::GameDir();
        PruneOrphans(gameDir + "\\gbhook\\cache");
        int built = 0, cached = 0, disabled = 0, skipped = 0;
        std::unordered_map<std::string, std::string> shipped;   // lowered relpath -> the first mod shipping it

        for (const ModSet::Record& r : Mods::Result().records)
        {
            if (!r.accepted) continue;

            const std::string modRoot = r.root + "\\" + r.folder;
            std::vector<TreeHash::File> loose;
            LeftOut left;
            WalkLoose(modRoot, "", loose, left);
            if (!left.roots.empty())
                Log::Writef("MODS", "content %s: left out %s -- not engine content", r.mod.id.c_str(), Join(left.roots).c_str());
            for (size_t i = 0; i < left.strays.size() && i < 5; ++i)
                Log::Writef("MODS", "content %s: left out %s -- not a file type the engine reads", r.mod.id.c_str(), left.strays[i].c_str());
            if (left.strays.size() > 5)
                Log::Writef("MODS", "content %s: left out %d more file(s) under the asset roots", r.mod.id.c_str(), (int)left.strays.size() - 5);

            // Two mods shipping one loose path is resolved by the engine's mount order and said nowhere else.
            for (const TreeHash::File& lf : loose)
            {
                std::string key = lf.relpath;
                for (char& ch : key) ch = (char)tolower((unsigned char)ch);
                auto it = shipped.find(key);
                if (it == shipped.end()) shipped.emplace(key, r.mod.id);
                else if (it->second != r.mod.id)
                    Log::Writef("MODS", "CONFLICT content %s shipped by both '%s' and '%s' -- code order decides, silently",
                                lf.relpath.c_str(), it->second.c_str(), r.mod.id.c_str());
            }

            const std::string cacheDir = gameDir + "\\gbhook\\cache\\" + r.mod.id;

            Content::Input in;
            in.id             = r.mod.id;
            in.loose          = loose;
            in.manualDisabled = r.mod.disabled;
            in.cachedHash     = IsDir(cacheDir) ? CachedHash(r.mod.id, cacheDir) : "";
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
                SetSummary(r.mod.id, "off, " + v.reason);
                break;

            case Content::Action::MountCached:
            {
                ++cached;
                const std::string rel = "gbhook\\cache\\" + r.mod.id + "\\" + v.hash + ".POD";
                Log::Writef("MODS", "content %s: cached, %s", r.mod.id.c_str(), rel.c_str());
                SetSummary(r.mod.id, std::to_string(loose.size()) + " file(s), cached");
                Enqueue(r.mod.id, rel);
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
                const std::string absTmp = absPod + ".tmp";
                Pod::FileSink sink;
                std::string why;
                // Revision 1: overrides the bulk archives, loses to a rev-0 chained MODS.POD, so GBMM still wins.
                // Written under .tmp and renamed at the end: whatever cuts the build short leaves no <hash>.POD.
                bool ok = sink.Open(absTmp.c_str()) && Pod::Write(sink, items, 1, "", &why);
                sink.Close();
                if (ok && !MoveFileExA(absTmp.c_str(), absPod.c_str(), MOVEFILE_REPLACE_EXISTING))
                {
                    why = "rename of the finished archive failed (error " + std::to_string(GetLastError()) + ")";
                    ok = false;
                }
                if (ok)
                {
                    ++built;
                    Log::Writef("MODS", "content %s: built %d file(s) -> %s", r.mod.id.c_str(), (int)loose.size(), relPod.c_str());
                    SetSummary(r.mod.id, std::to_string(loose.size()) + " file(s) built");
                    Enqueue(r.mod.id, relPod);
                }
                else
                {
                    DeleteFileA(absTmp.c_str());
                    Log::Writef("MODS", "content %s: build FAILED -- %s", r.mod.id.c_str(), why.c_str());
                    SetSummary(r.mod.id, "build failed, " + why);
                }
                break;
            }
            }
        }

        Log::Writef("MODS", "content: %d built, %d cached, %d disabled, %d without content",
                    built, cached, disabled, skipped);

        // One more visit from the pump, so the tally is printed even when the last mods had nothing to mount.
        InterlockedExchange(&g_buildDone, 1);
        ParkMount();
    }

    DWORD WINAPI BuildThread(LPVOID)
    {
        Build();
        return 0;
    }
}

namespace ContentBuild
{
    void Start()
    {
        EnsureLock();
        if (InterlockedCompareExchange(&g_started, 1, 0) != 0) return;

        HANDLE h = CreateThread(nullptr, 0, BuildThread, nullptr, 0, nullptr);
        if (h) { CloseHandle(h); return; }
        Log::Writef("MODS", "content build thread could not start (error %lu); building on the boot thread", GetLastError());
        Build();
    }

    bool Done() { return g_allMounted != 0; }

    std::string Summary(const char* id)
    {
        if (!id) return "";
        EnsureLock();
        std::string out;
        EnterCriticalSection(&g_lock);
        auto it = g_summary.find(id);
        if (it != g_summary.end()) out = it->second;
        else if (!g_buildDone)     out = "building";
        LeaveCriticalSection(&g_lock);
        return out;
    }
}
