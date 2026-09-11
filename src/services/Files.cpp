// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// The engine is only ever entered through a guarded plain-C frame, and its own free releases its own strings.
#include "Files.h"
#include "Commands.h"
#include "../core/Framework.h"
#include "../core/Seh.h"
#include "gbhook/gbhook.h"

#include <windows.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace
{
    // ---- RVAs (ghost.exe 0b89556c07e5b737efe444351227e747) --------------------
    constexpr uintptr_t kRvaListFiles   = 0x3B9980;   // void listFiles(CStrList*, dir, pattern)
    constexpr uintptr_t kRvaStrListCtor = 0x326BE0;
    constexpr uintptr_t kRvaStrListDtor = 0x326C00;   // frees the elements too
    constexpr uintptr_t kRvaStrListAt   = 0x327060;   // char* CStrList::at(int)
    constexpr uintptr_t kRvaStreamCtor  = 0x463AD0;   // CInStreamFile, 0x28 bytes
    constexpr uintptr_t kRvaStreamDtor  = 0x463EF0;
    constexpr uintptr_t kRvaStreamOpen  = 0x465180;   // bool open(name, mode): memory, then PODs, then loose
    constexpr uintptr_t kRvaStreamRead  = 0x465B70;   // int read(dst, len): the decompressed asset; -1 at EOF
    constexpr uintptr_t kRvaStreamSize  = 0x464DD0;   // the RAW stream size, wrong for a deflated entry

    constexpr uintptr_t kStrListCount = 0x08;         // CStrList { vftable, int count, int cap, char** items }
    constexpr size_t    kStrListSlack = 64;           // the ctor writes 0x18; the rest is paranoia
    constexpr size_t    kStreamSlack  = 128;
    constexpr int       kMaxEntries   = 4096;
    constexpr int       kChunk        = 64 * 1024;
    constexpr int       kMaxFileBytes = 64 * 1024 * 1024;

    typedef void  (__fastcall* tListFiles)(void*, const char*, const char*);
    typedef void* (__fastcall* tCtor)(void*);
    typedef void  (__fastcall* tDtor)(void*);
    typedef char* (__fastcall* tAt)(void*, int);
    typedef bool  (__fastcall* tOpen)(void*, const char*, int);
    typedef int   (__fastcall* tRead)(void*, void*, int);
    typedef int   (__fastcall* tSize)(void*);

    template <typename T> T At(uintptr_t rva) { return (T)(void*)(gameBase + rva); }

    // ---- plain-C frames --------------------------------------------------------
    bool SafeCall(tCtor fn, void* o)  { GBH_SEH_TRY { fn(o); return true; } GBH_SEH_EXCEPT { return false; } }
    bool SafeDtor(tDtor fn, void* o)  { GBH_SEH_TRY { fn(o); return true; } GBH_SEH_EXCEPT { return false; } }

    bool SafeList(tListFiles fn, void* l, const char* dir, const char* pat)
    {
        GBH_SEH_TRY { fn(l, dir, pat); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    bool SafeCount(const void* l, int* out)
    {
        *out = 0;
        GBH_SEH_TRY { *out = *(const int*)((const char*)l + kStrListCount); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    // The element is an engine-owned string: read, never freed here.
    bool SafeAt(tAt fn, void* l, int i, char* dst, size_t cap)
    {
        dst[0] = 0;
        GBH_SEH_TRY
        {
            const char* s = fn(l, i);
            if (!s) return false;
            size_t n = 0;
            while (n < cap - 1 && s[n]) { dst[n] = s[n]; ++n; }
            dst[n] = 0;
            return true;
        }
        GBH_SEH_EXCEPT { return false; }
    }

    bool SafeOpen(tOpen fn, void* s, const char* name, bool* ok)
    {
        *ok = false;
        GBH_SEH_TRY { *ok = fn(s, name, 0); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    bool SafeRead(tRead fn, void* s, void* dst, int len, int* got)
    {
        *got = -2;
        GBH_SEH_TRY { *got = fn(s, dst, len); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    bool SafeSize(tSize fn, void* s, int* out)
    {
        *out = -1;
        GBH_SEH_TRY { *out = fn(s); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    // The engine builds "dir\pattern" into a 0x100 buffer and splits it on ';' and ','.
    bool PatternIsSane(const char* dir, const char* pattern)
    {
        const size_t d = dir ? strlen(dir) : 0, p = pattern ? strlen(pattern) : 0;
        if (p == 0)          { Log::Write("FILE", "list: empty pattern"); return false; }
        if (d + p + 2 > 200) { Log::Write("FILE", "list: dir and pattern are too long"); return false; }
        return true;
    }

    // The loose-file fallback resolves against the process directory, so '..' would reach outside the game.
    bool PathIsSane(const char* path)
    {
        if (!path || !*path)      { Log::Write("FILE", "read: empty path"); return false; }
        if (strlen(path) >= 0xF0) { Log::Writef("FILE", "read: path is too long: %.64s", path); return false; }
        if (strstr(path, ".."))   { Log::Writef("FILE", "read: '..' refused: %s", path); return false; }
        return true;
    }

    // ---- commands --------------------------------------------------------------
    int CmdFiles(int argc, const char* const* argv, const char** err, void*)
    {
        if (argc < 1) { *err = "usage: files [dir] <pattern>   e.g. files world *.lvl"; return GBH_ERR_ARG; }
        const char* dir = argc >= 2 ? argv[0] : nullptr;
        const char* pat = argc >= 2 ? argv[1] : argv[0];
        std::vector<std::string> names;
        const int n = Files::List(dir, pat, names);
        if (n < 0) { *err = "the engine's enumerator failed; see the FILE lines"; return n; }
        Log::Writef("RES", "%d file(s) matching %s\\%s across every mounted archive", n, dir ? dir : "", pat);
        int shown = 0;
        for (const std::string& s : names)
        {
            Log::Writef("RES", "  %s", s.c_str());
            if (++shown >= 200) { Log::Write("RES", "  ... cut at 200"); break; }
        }
        return GBH_OK;
    }

    int CmdFileSize(int argc, const char* const* argv, const char** err, void*)
    {
        if (argc < 1) { *err = "usage: filesize <path>   e.g. filesize world\\haunt1.dante"; return GBH_ERR_ARG; }
        std::vector<unsigned char> bytes;
        const int n = Files::Read(argv[0], bytes);
        if (n == GBH_ERR_NOT_FOUND) { *err = "no mounted archive and no loose file has that path"; return n; }
        if (n < 0)                  { *err = "the engine read failed; see the FILE lines"; return n; }
        char head[3 * 16 + 1] = { 0 }, text[17] = { 0 };
        const int m = n < 16 ? n : 16;
        for (int i = 0; i < m; ++i)
        {
            _snprintf_s(head + i * 3, 4, _TRUNCATE, "%02X ", bytes[(size_t)i]);
            text[i] = (bytes[(size_t)i] >= 32 && bytes[(size_t)i] < 127) ? (char)bytes[(size_t)i] : '.';
        }
        Log::Writef("RES", "%s -> %d bytes", argv[0], n);
        Log::Writef("RES", "  %-48s |%s|", head, text);
        return GBH_OK;
    }
}

namespace Files
{
    int List(const char* dir, const char* pattern, std::vector<std::string>& out)
    {
        out.clear();
        if (!gameBase) return GBH_ERR_STATE;
        if (!PatternIsSane(dir, pattern)) return GBH_ERR_ARG;

        alignas(16) unsigned char list[kStrListSlack];
        memset(list, 0, sizeof list);
        if (!SafeCall(At<tCtor>(kRvaStrListCtor), list)) { Log::Write("FILE", "list: CStrList ctor faulted"); return GBH_ERR; }

        const char* d = (dir && *dir) ? dir : nullptr;
        if (!SafeList(At<tListFiles>(kRvaListFiles), list, d, pattern))
        {
            Log::Writef("FILE", "list %s\\%s: listFiles faulted", d ? d : "", pattern);
            SafeDtor(At<tDtor>(kRvaStrListDtor), list);
            return GBH_ERR;
        }

        int count = 0;
        if (!SafeCount(list, &count) || count < 0 || count > kMaxEntries)
        {
            Log::Writef("FILE", "list %s\\%s: count %d is not usable", d ? d : "", pattern, count);
            SafeDtor(At<tDtor>(kRvaStrListDtor), list);
            return GBH_ERR;
        }

        tAt at = At<tAt>(kRvaStrListAt);
        out.reserve((size_t)count);
        for (int i = 0; i < count; ++i)
        {
            char name[0x120];
            if (SafeAt(at, list, i, name, sizeof name) && name[0]) out.push_back(name);
        }
        if (!SafeDtor(At<tDtor>(kRvaStrListDtor), list)) Log::Write("FILE", "list: CStrList dtor faulted; leaked");

        // The engine deduplicates but does not sort.
        std::sort(out.begin(), out.end(), [](const std::string& a, const std::string& b) { return _stricmp(a.c_str(), b.c_str()) < 0; });
        out.erase(std::unique(out.begin(), out.end(), [](const std::string& a, const std::string& b) { return _stricmp(a.c_str(), b.c_str()) == 0; }), out.end());
        Log::Writef("FILE", "list %s\\%s -> %d name(s)", d ? d : "", pattern, (int)out.size());
        return (int)out.size();
    }

    int Read(const char* path, std::vector<unsigned char>& out)
    {
        out.clear();
        if (!gameBase) return GBH_ERR_STATE;
        if (!PathIsSane(path)) return GBH_ERR_ARG;

        alignas(16) unsigned char stream[kStreamSlack];
        memset(stream, 0, sizeof stream);
        if (!SafeCall(At<tCtor>(kRvaStreamCtor), stream)) { Log::Write("FILE", "read: stream ctor faulted"); return GBH_ERR; }

        int  rc = GBH_ERR;
        bool opened = false;
        if (!SafeOpen(At<tOpen>(kRvaStreamOpen), stream, path, &opened))
            Log::Writef("FILE", "read %s: open faulted", path);
        else if (!opened)
        {
            Log::Writef("FILE", "read %s: not in any mounted archive or on disk", path);
            rc = GBH_ERR_NOT_FOUND;
        }
        else
        {
            // size() is the raw stream's length and wrong for a deflated entry: a starting capacity, never a stop.
            int hint = 0;
            SafeSize(At<tSize>(kRvaStreamSize), stream, &hint);
            if (hint > 0 && hint < kMaxFileBytes) out.reserve((size_t)hint);

            tRead rd = At<tRead>(kRvaStreamRead);
            int  total = 0;
            bool ok = true;
            for (;;)
            {
                const size_t base = out.size();
                out.resize(base + (size_t)kChunk);
                int got = 0;
                if (!SafeRead(rd, stream, out.data() + base, kChunk, &got) || got > kChunk)
                {
                    Log::Writef("FILE", "read %s: read faulted at %d bytes", path, total);
                    out.resize(base);
                    ok = false;
                    break;
                }
                if (got <= 0) { out.resize(base); if (got == -2 && total == 0) ok = false; break; }
                out.resize(base + (size_t)got);
                total += got;
                if (total > kMaxFileBytes) { Log::Writef("FILE", "read %s: over the cap; cut", path); break; }
            }
            rc = ok ? total : GBH_ERR;
            if (ok) Log::Writef("FILE", "read %s -> %d bytes (raw %d)", path, total, hint);
        }
        if (!SafeDtor(At<tDtor>(kRvaStreamDtor), stream)) Log::Writef("FILE", "read %s: stream dtor faulted", path);
        if (rc < 0) out.clear();
        return rc;
    }

    void RegisterCommands()
    {
        Commands::Register(nullptr, "files",    CmdFiles,    nullptr, "[dir] <pattern> -- list assets across every mounted archive", Commands::kGameThread);
        Commands::Register(nullptr, "filesize", CmdFileSize, nullptr, "<path> -- read an asset and report its size", Commands::kGameThread);
    }
}
