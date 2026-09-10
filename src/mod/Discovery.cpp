// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Discovery.h"
#include "PeFile.h"
#include "../core/Framework.h"
#include "format/Ini.h"

#include <algorithm>
#include <cstdio>

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

    ModSet::Candidate Gather(const std::string& root, const std::string& folder)
    {
        ModSet::Candidate c;
        c.root   = root;
        c.folder = folder;
        const std::string gbhook = root + "\\" + folder + "\\gbhook";

        std::string text;
        c.hasModIni  = ReadWhole(gbhook + "\\mod.ini", text);
        c.hasModInfo = IsFile(root + "\\" + folder + "\\previews\\modinfo.ini");
        if (!c.hasModIni) return c;

        c.ini = ModIni::Parse(text);
        if (!c.ini.refusal.empty() || c.ini.mod.plugin.empty()) return c;

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
        int assetOnly = 0;

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
                if (!IsDir(root + "\\" + folder + "\\gbhook")) { ++assetOnly; continue; }
                found.push_back(Gather(root, folder));
            }
        }

        g_result = ModSet::Resolve(found);

        int accepted = 0, refused = 0;
        for (const ModSet::Record& r : g_result.records)
        {
            if (r.accepted)
            {
                ++accepted;
                Log::Writef("MODS", "%2d. %s %s (%s, priority %d) from %s%s%s",
                            r.order + 1, r.mod.id.c_str(), r.mod.version.c_str(),
                            ModIni::StageName(r.mod.stage), r.mod.priority, r.folder.c_str(),
                            r.mod.plugin.empty() ? "" : ", plugin ", r.mod.plugin.c_str());
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

        Log::Writef("MODS", "%d folder(s) with gbhook/, %d accepted, %d refused, %d asset-only left to the Mod Manager",
                    (int)found.size(), accepted, refused, assetOnly);
    }

    const ModSet::Result&           Result()       { return g_result; }
    const std::vector<std::string>& MissingRoots() { return g_missingRoots; }

    std::string GbhookDir(const ModSet::Record& r)
    {
        return r.root + "\\" + r.folder + "\\gbhook";
    }
}
