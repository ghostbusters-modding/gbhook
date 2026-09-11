// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// `pod list` / `pod mount`, straight onto the engine's CPod. Layout and addresses: engine/RE_POD_LOADING.md.
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

#include "Pods.h"
#include "Commands.h"
#include "../core/Framework.h"
#include "../core/Seh.h"

namespace
{
    // ---- RVAs (ghost.exe 0b89556c07e5b737efe444351227e747) ----------------
    const uintptr_t kRvaGPod     = 0xDE54F0;   // CPod**  -> the live CGhostPod
    const uintptr_t kRvaGhostPod = 0xDCF330;   // CGhostPod*, the boot source of gPod
    const uintptr_t kRvaMountPod = 0x456E30;   // bool CPod::mountPod(this,name,err512,hashed)
    const uintptr_t kRvaSortPods = 0x455800;   // void CPod::sortByRevision(this) == vtable[2]

    // ---- CPod / CPodFile members ------------------------------------------
    const uintptr_t kPodCount    = 0x08;       // int
    const uintptr_t kPodSlots    = 0x10;       // CPodFile*[100]
    const int       kMaxSlots    = 100;

    const uintptr_t kPfPath      = 0x004;      // char[0x100]
    const uintptr_t kPfNext      = 0x200;      // char[0x50]
    const uintptr_t kPfRevision  = 0x250;      // int  -- the priority key
    const uintptr_t kPfHashed    = 0x258;      // u8
    const uintptr_t kPfFiles     = 0x260;      // int
    const size_t    kPodFileSize = 0x288;

    const size_t    kErrBuf      = 512;        // openPod copies out of a 512-byte buffer

    typedef bool (__fastcall* tMountPod)(void*, const char*, char*, unsigned char);
    typedef void (__fastcall* tSortPods)(void*);

    // -----------------------------------------------------------------------
    //  guarded reads -- plain-C frames only below this line
    // -----------------------------------------------------------------------
    bool MemReadable(const void* p, size_t n)
    {
        if (!p || n == 0) return false;
        const unsigned char* q = (const unsigned char*)p;
        size_t left = n;
        while (left)
        {
            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQuery(q, &mbi, sizeof mbi) != sizeof mbi) return false;
            if (mbi.State != MEM_COMMIT) return false;
            if (mbi.Protect & PAGE_GUARD) return false;
            DWORD prot = mbi.Protect & 0xFF;
            if (prot != PAGE_READONLY && prot != PAGE_READWRITE && prot != PAGE_WRITECOPY &&
                prot != PAGE_EXECUTE_READ && prot != PAGE_EXECUTE_READWRITE &&
                prot != PAGE_EXECUTE_WRITECOPY)
                return false;

            const unsigned char* end = (const unsigned char*)mbi.BaseAddress + mbi.RegionSize;
            size_t avail = (size_t)(end - q);
            if (avail >= left) return true;
            left -= avail;
            q = end;
        }
        return true;
    }

    bool SafeRead(const void* src, void* dst, size_t n, const char* who)
    {
        if (!MemReadable(src, n)) { Log::Writef("POD", "unreadable %s at %p", who, src); return false; }
        GBH_SEH_TRY { memcpy(dst, src, n); return true; }
        GBH_SEH_EXCEPT
        {
            Log::Writef("POD", "EXC reading %s at %p", who, src);
            return false;
        }
    }

    // Copy a NUL-terminated string of at most `cap-1` bytes out of game memory.
    // Never fails hard: an unreadable or unterminated field becomes "?".
    void SafeStr(const void* src, size_t max, char* dst, size_t cap, const char* who)
    {
        dst[0] = 0;
        if (cap < 2) return;
        size_t want = max < cap - 1 ? max : cap - 1;
        if (!MemReadable(src, want)) { lstrcpynA(dst, "?", (int)cap); return; }
        GBH_SEH_TRY
        {
            const char* s = (const char*)src;
            size_t i = 0;
            for (; i < want && s[i]; ++i) dst[i] = s[i];
            dst[i] = 0;
        }
        GBH_SEH_EXCEPT
        {
            Log::Writef("POD", "EXC reading %s at %p", who, src);
            lstrcpynA(dst, "?", (int)cap);
        }
    }

    bool SafeMount(tMountPod fn, void* pod, const char* name, char* err, bool* out)
    {
        *out = false;
        GBH_SEH_TRY { *out = fn(pod, name, err, 0); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    bool SafeSort(tSortPods fn, void* pod)
    {
        GBH_SEH_TRY { fn(pod); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    // -----------------------------------------------------------------------
    //  the pod object
    // -----------------------------------------------------------------------

    // *(CPod**)(base + 0xDE54F0). Falls back to the CGhostPod pointer at
    // 0xDCF330, which is what boot assigns into gPod -- if the two disagree,
    // say so rather than guessing.
    void* GPod()
    {
        if (!gameBase) { Log::Write("POD", "gameBase is null -- too early"); return nullptr; }

        void* gp = nullptr;
        void* gg = nullptr;
        SafeRead(gameBase + kRvaGPod,     &gp, sizeof gp, "gPod");
        SafeRead(gameBase + kRvaGhostPod, &gg, sizeof gg, "gGhostPod");

        if (!gp && gg)
        {
            Log::Writef("POD", "gPod is null; falling back to gGhostPod %p "
                               "(file system not initialised yet?)", gg);
            gp = gg;
        }
        else if (gp && gg && gp != gg)
        {
            Log::Writef("POD", "gPod %p != gGhostPod %p -- using gPod, the one the "
                               "lookup hook reads", gp, gg);
        }
        if (!gp) Log::Write("POD", "no pod object -- CPod::init has not run");
        return gp;
    }

    bool ReadCount(void* pod, int* out)
    {
        *out = 0;
        if (!SafeRead((const char*)pod + kPodCount, out, sizeof(int), "CPod::count")) return false;
        if (*out < 0 || *out > kMaxSlots)
        {
            Log::Writef("POD", "CPod::count = %d is out of range [0,%d] -- refusing to walk",
                        *out, kMaxSlots);
            return false;
        }
        return true;
    }

    struct SlotInfo
    {
        void* pf;
        char  path[0x100];
        char  next[0x50];
        int   revision;
        int   files;
        unsigned char hashed;
        bool  ok;
    };

    bool ReadSlot(void* pod, int i, SlotInfo* s)
    {
        memset(s, 0, sizeof *s);
        if (!SafeRead((const char*)pod + kPodSlots + (size_t)i * sizeof(void*),
                      &s->pf, sizeof(void*), "CPod::pods[i]"))
            return false;
        if (!s->pf) return true;                    // an empty slot is not an error
        if (!MemReadable(s->pf, kPodFileSize))
        {
            Log::Writef("POD", "slot %d: CPodFile %p is not readable", i, s->pf);
            return true;
        }
        SafeRead((const char*)s->pf + kPfRevision, &s->revision, sizeof(int), "CPodFile::revision");
        SafeRead((const char*)s->pf + kPfFiles,    &s->files,    sizeof(int), "CPodFile::files");
        SafeRead((const char*)s->pf + kPfHashed,   &s->hashed,   1,           "CPodFile::hashed");
        SafeStr((const char*)s->pf + kPfPath, 0x100, s->path, sizeof s->path, "CPodFile::path");
        SafeStr((const char*)s->pf + kPfNext, 0x50,  s->next, sizeof s->next, "CPodFile::next");
        s->ok = true;
        return true;
    }

    // Log the whole slot table. `why` prefixes the header line.
    bool Dump(void* pod, const char* why)
    {
        int count = 0;
        if (!ReadCount(pod, &count)) return false;

        long long files = 0;
        Log::Writef("POD", "%s: %d archive%s mounted on CPod %p "
                           "(slot order = lookup order, first match wins)",
                    why, count, count == 1 ? "" : "s", pod);

        for (int i = 0; i < count; ++i)
        {
            SlotInfo s;
            if (!ReadSlot(pod, i, &s)) { Log::Writef("POD", "  slot %2d  <unreadable>", i); continue; }
            if (!s.pf)                 { Log::Writef("POD", "  slot %2d  <empty>", i); continue; }
            if (!s.ok)                 continue;

            files += s.files;
            Log::Writef("POD", "  slot %2d  rev %-6d %7d files%s  next='%s'  %s",
                        i, s.revision, s.files,
                        s.hashed ? "  [hashed names]" : "",
                        s.next, s.path);
        }
        Log::Writef("POD", "  total %lld files; priority is ascending header revision "
                           "(CPodFile+0x250), ties keep mount order", files);
        return true;
    }

    // A bare file name only: the engine resolves it with _wfullpath against the
    // process CWD (the game directory), and letting a caller reach outside that
    // buys nothing but surprises.
    bool NameIsSane(const char* name, const char** err)
    {
        if (!name || !*name)             { *err = "usage: pod mount <NAME.POD>"; return false; }
        if (strlen(name) >= 128)         { *err = "archive name is too long"; return false; }
        // A path relative to the game directory is allowed (gbhook\cache\<id>\<hash>.POD
        // is how boot-time content arrives); an absolute path, a drive, a
        // wildcard or '..' is not.
        if (strpbrk(name, "/:*?\"<>|"))  { *err = "give a file name relative to the game directory"; return false; }
        if (name[0] == '\\')             { *err = "give a file name relative to the game directory"; return false; }
        if (strstr(name, ".."))          { *err = "'..' is not allowed in the archive name"; return false; }
        return true;
    }
}

namespace
{
    int CmdPod(int argc, const char* const* argv, const char** err, void*)
    {
        if (argc >= 1 && _stricmp(argv[0], "list") == 0)
        {
            if (!Pods::List()) { *err = "could not read the engine's pod table"; return GBH_ERR; }
            return GBH_OK;
        }
        if (argc >= 2 && _stricmp(argv[0], "mount") == 0)
        {
            const char* why = nullptr;
            if (!Pods::Mount(argv[1], &why)) { *err = why ? why : "mount failed"; return GBH_ERR; }
            return GBH_OK;
        }
        *err = "usage: pod list | pod mount <NAME.POD>";
        return GBH_ERR_ARG;
    }
}

namespace Pods
{
    void RegisterCommands()
    {
        Commands::Register(nullptr, "pod", CmdPod, nullptr, "list | mount <NAME.POD>", Commands::kGameThread);
    }

    bool Ready()
    {
        if (!gameBase) return false;
        void* gp = nullptr;
        void* gg = nullptr;
        SafeRead(gameBase + kRvaGPod,     &gp, sizeof gp, "gPod");
        SafeRead(gameBase + kRvaGhostPod, &gg, sizeof gg, "gGhostPod");
        return gp != nullptr || gg != nullptr;
    }

    bool List()
    {
        void* pod = GPod();
        if (!pod) return false;
        return Dump(pod, "mounted");
    }

    bool Mount(const char* name, const char** err)
    {
        const char* dummy = nullptr;
        if (!err) err = &dummy;
        *err = nullptr;

        if (!NameIsSane(name, err)) return false;

        // 1. the file has to be there, and be at least a POD6 header long.
        char full[MAX_PATH];
        _snprintf_s(full, sizeof full, _TRUNCATE, "%s\\%s", Framework::GameDir(), name);
        // The engine writes the resolved full path into CPodFile::path, a char[0x100].
        if (strlen(full) >= 0x100)
        {
            Log::Writef("POD", "mount %s: full path is %d bytes, over the engine's 256-byte path field", name, (int)strlen(full));
            *err = "the archive's full path is too long for the engine";
            return false;
        }

        WIN32_FILE_ATTRIBUTE_DATA fad;
        if (!GetFileAttributesExA(full, GetFileExInfoStandard, &fad))
        {
            Log::Writef("POD", "mount %s: not found at %s", name, full);
            *err = "archive not found in the game directory";
            return false;
        }
        unsigned long long size = ((unsigned long long)fad.nFileSizeHigh << 32) | fad.nFileSizeLow;
        if (size < 0x80)
        {
            Log::Writef("POD", "mount %s: %llu bytes, shorter than a POD6 header", name, size);
            *err = "file is too short to be a POD6 archive";
            return false;
        }
        Log::Writef("POD", "mount %s: %s, %llu bytes", name, full, size);

        // 2. the pod object and a slot to put it in.
        void* pod = GPod();
        if (!pod) { *err = "the engine's pod object is not up yet"; return false; }

        int before = 0;
        if (!ReadCount(pod, &before)) { *err = "could not read CPod::count"; return false; }
        if (before >= kMaxSlots)
        {
            Log::Writef("POD", "mount %s: all %d slots are in use", name, kMaxSlots);
            *err = "the engine's 100 pod slots are full";
            return false;
        }
        Log::Writef("POD", "mount %s: %d/%d slots used before the call", name, before, kMaxSlots);

        // 3. call the engine's own mountPod. It follows the archive's chain, and
        //    dismounts any same-named archive first -- so a second mount of the
        //    same name is a reload, not a duplicate.
        tMountPod mountPod = (tMountPod)(gameBase + kRvaMountPod);
        char engineErr[kErrBuf];
        memset(engineErr, 0, sizeof engineErr);

        Log::Writef("POD", "mount %s: calling CPod::mountPod(%p, \"%s\", err, hashed=0) "
                           "at %p", name, pod, name, (void*)mountPod);

        bool ok = false;
        if (!SafeMount(mountPod, pod, name, engineErr, &ok))
        {
            Log::Writef("POD", "mount %s: EXCEPTION inside CPod::mountPod -- the pod table "
                               "may be inconsistent, restart the game", name);
            *err = "the engine faulted inside mountPod";
            return false;
        }

        int after = 0;
        ReadCount(pod, &after);
        engineErr[sizeof engineErr - 1] = 0;

        if (!ok)
        {
            Log::Writef("POD", "mount %s: FAILED -- engine says \"%s\" (slots %d -> %d)",
                        name, engineErr[0] ? engineErr : "(no reason given)", before, after);
            *err = "mountPod refused the archive; see the log for the engine's reason";
            return false;
        }
        Log::Writef("POD", "mount %s: mountPod returned true, slots %d -> %d%s%s",
                    name, before, after,
                    engineErr[0] ? ", note: " : "", engineErr[0] ? engineErr : "");

        // 4. mountPod appends; it does NOT re-establish priority. Both engine
        //    callers (mountDefaultPods, setLanguage) follow it with vtable[2],
        //    the stable ascending sort on the header revision. Prefer the
        //    object's own vtable slot over the hardcoded address, but say so
        //    when they differ.
        tSortPods sortPods = (tSortPods)(gameBase + kRvaSortPods);
        void** vt = nullptr;
        if (SafeRead(pod, &vt, sizeof vt, "CPod::vftable") && MemReadable(vt, 3 * sizeof(void*)))
        {
            void* slot2 = nullptr;
            if (SafeRead(&vt[2], &slot2, sizeof slot2, "vftable[2]") && slot2)
            {
                if (slot2 != (void*)sortPods)
                    Log::Writef("POD", "vftable[2] = %p, expected sortByRevision at %p "
                                       "-- calling the vtable slot", slot2, (void*)sortPods);
                sortPods = (tSortPods)slot2;
            }
        }
        Log::Writef("POD", "mount %s: calling sortByRevision at %p", name, (void*)sortPods);
        if (!SafeSort(sortPods, pod))
        {
            Log::Writef("POD", "mount %s: EXCEPTION inside sortByRevision -- the archive is "
                               "mounted but its priority is unsorted", name);
            *err = "the engine faulted inside sortByRevision";
            return false;
        }

        // 5. show where it landed.
        Dump(pod, "after mount");
        Log::Writef("POD", "mount %s: done. Assets already loaded are NOT re-read; "
                           "reload the level to pick the archive up.", name);
        return true;
    }
}
