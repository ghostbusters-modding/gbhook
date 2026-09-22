// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Discovery.h"
#include "modset/Content.h"
#include "PeFile.h"
#include "../core/Framework.h"
#include "format/Ini.h"

#include <algorithm>
#include <cstdio>
#include <utility>

namespace
{
    ModSet::Result           g_result;
    std::vector<std::string> g_missingRoots;
    bool                     g_scanned = false;

    bool IsDir(const std::string& p)
    {
        DWORD a = GetFileAttributesA(p.c_str());
        return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
    }

    bool IsFile(const std::string& p)
    {
        DWORD a = GetFileAttributesA(p.c_str());
        return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
    }

    bool ReadWhole(const std::string& path, std::string& out)
    {
        FILE* f = nullptr;
        if (fopen_s(&f, path.c_str(), "rb") != 0 || !f) return false;
        char   buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof buf, f)) > 0) out.append(buf, n);
        fclose(f);
        return true;
    }

    std::string Backslashed(std::string p)
    {
        for (char& c : p) if (c == '/') c = '\\';
        while (!p.empty() && p.back() == '\\') p.pop_back();
        return p;
    }

    // <gamedir>-relative unless it names a drive or a share.
    std::string AbsoluteRoot(const std::string& root)
    {
        std::string r = Backslashed(root);
        if ((r.size() >= 2 && r[1] == ':') || r.compare(0, 2, "\\\\") == 0) return r;
        return std::string(Framework::GameDir()) + "\\" + r;
    }

    // Sorted, so two runs of one install see the same order before priorities apply.
    std::vector<std::string> SubDirs(const std::string& dir)
    {
        std::vector<std::string> out;
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA((dir + "\\*").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) return out;
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
            if (fd.cFileName[0] == '.') continue;
            out.push_back(fd.cFileName);
        } while (FindNextFileA(h, &fd));
        FindClose(h);
        std::sort(out.begin(), out.end(), [](const std::string& a, const std::string& b)
                  { return _stricmp(a.c_str(), b.c_str()) < 0; });
        return out;
    }

    // Reads the DLL named by an already-parsed modinfo.ini. Nothing here runs mod code.
    ModSet::Candidate Gather(const std::string& root, const std::string& folder, bool hasModInfo, ModIni::Result ini)
    {
        ModSet::Candidate c;
        c.root       = root;
        c.folder     = folder;
        c.hasModInfo = hasModInfo;
        c.ini        = std::move(ini);
        const std::string gbhook = root + "\\" + folder + "\\gbhook";
        c.hasModIni  = IsFile(gbhook + "\\mod.ini");
        if (!c.hasModInfo || !c.ini.refusal.empty() || c.ini.mod.plugin.empty()) return c;

        const std::string dll = gbhook + "\\" + Backslashed(c.ini.mod.plugin);
        if (!IsFile(dll)) { c.binary = ModSet::Binary::Missing; return c; }

        const char* why = nullptr;
        if (PeFile::ReadDataExport(dll.c_str(), GBH_EXPORT_MANIFEST, &c.manifest, sizeof c.manifest, &why))
            c.binary = ModSet::Binary::Ok;
        else
        {
            c.binary    = ModSet::Binary::Unreadable;
            c.binaryWhy = why ? why : "unreadable";
        }
        return c;
    }
}

namespace Mods
{
    void Scan()
    {
        if (g_scanned) return;
        g_scanned = true;

        std::vector<ModSet::Candidate> found;
        int notMods = 0;

        for (const std::string& raw : Ini::List(Settings::Get("mods.root", "mods")))
        {
            const std::string root = AbsoluteRoot(raw);
            if (!IsDir(root))
            {
                Log::Writef("MODS", "root %s does not exist", root.c_str());
                g_missingRoots.push_back(root);
                continue;
            }
            Log::Writef("MODS", "root %s", root.c_str());
            for (const std::string& folder : SubDirs(root))
            {
                // Every folder is a mod unless it has none of the three: the manager's file, a gbhook/ half, an asset root.
                const std::string dir = root + "\\" + folder;
                std::string    text;
                const bool     hasModInfo   = ReadWhole(dir + "\\previews\\modinfo.ini", text);
                const bool     hasGbhookDir = IsDir(dir + "\\gbhook");
                bool           hasAssets    = false;
                for (const std::string& sub : SubDirs(dir))
                    if (Content::IsAssetRoot(sub)) { hasAssets = true; break; }
                if (!hasModInfo && !hasGbhookDir && !hasAssets) { ++notMods; continue; }

                ModIni::Result ini = hasModInfo ? ModIni::Parse(text) : ModIni::Result{};
                ModSet::Candidate c = Gather(root, folder, hasModInfo, std::move(ini));
                c.hasGbhookDir = hasGbhookDir;
                c.hasAssets    = hasAssets;
                found.push_back(std::move(c));
            }
        }

        g_result = ModSet::Resolve(found);

        int accepted = 0, implicit = 0, off = 0, refused = 0;
        for (const ModSet::Record& r : g_result.records)
        {
            if (r.accepted)
            {
                ++accepted;
                if (r.implicit) ++implicit;
                Log::Writef("MODS", "%2d. %s %s (%s, priority %d) from %s%s%s%s",
                            r.order + 1, r.mod.id.c_str(), r.mod.version.c_str(),
                            ModIni::StageName(r.mod.stage), r.mod.priority, r.folder.c_str(),
                            r.mod.plugin.empty() ? "" : ", plugin ", r.mod.plugin.c_str(),
                            r.implicit ? ", no [gbhook] section" : "");
            }
            else if (r.disabled)
            {
                ++off;
                Log::Writef("MODS", "OFF %s (%s): disabled in modinfo.ini", r.mod.id.c_str(), r.folder.c_str());
            }
            else
            {
                ++refused;
                Log::Writef("MODS", "REFUSED %s: %s", r.folder.c_str(), r.refusal.c_str());
            }
            for (const std::string& w : r.warnings)
                Log::Writef("MODS", "warning %s: %s", r.folder.c_str(), w.c_str());
        }
        for (const std::string& c : g_result.conflicts)
            Log::Writef("MODS", "CONFLICT %s", c.c_str());

        Log::Writef("MODS", "%d mod folder(s), %d accepted (%d without a [gbhook] section), %d off, %d refused, %d folder(s) that are not mods",
                    (int)found.size(), accepted, implicit, off, refused, notMods);
    }

    const ModSet::Result&           Result()       { return g_result; }
    const std::vector<std::string>& MissingRoots() { return g_missingRoots; }

    std::string GbhookDir(const ModSet::Record& r)
    {
        return r.root + "\\" + r.folder + "\\gbhook";
    }
}
