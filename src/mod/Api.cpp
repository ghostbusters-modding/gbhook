// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Nothing here transfers ownership: every buffer is the caller's, every string handed back is ours and thread-local.
#include "Api.h"
#include "Host.h"
#include "../core/Framework.h"
#include "../core/HookBroker.h"

#include <windows.h>
#include <cstdarg>
#include <cstring>
#include <string>

#if defined(_MSC_VER)
#  include <intrin.h>
#  define GBH_CALLER() _ReturnAddress()
#else
#  define GBH_CALLER() __builtin_return_address(0)
#endif

namespace
{
    // The mod a call came from, by its return address. nullptr is gbhook itself.
    const char* Who(const void* ra) { return Host::OwnerOfAddress(ra); }

    // ---- environment --------------------------------------------------------
    void*       GameBase()  { return gameBase; }
    const char* GameDir()   { return Framework::GameDir(); }
    uint32_t    Version()   { return Framework::kVersion; }
    const char* ModDir()
    {
        const char* d = Host::DirOfAddress(GBH_CALLER());
        return d ? d : Framework::GameDir();
    }

    // ---- memory -------------------------------------------------------------
    void* Alloc(size_t n)            { return Framework::Alloc(n); }
    void* Realloc(void* p, size_t n) { return Framework::Realloc(p, n); }
    void  Free(void* p)              { Framework::Free(p); }

    // ---- log ----------------------------------------------------------------
    void LogLine(const char* tag, const char* line) { Log::WriteFrom(Who(GBH_CALLER()), tag, line); }
    void Logf(const char* tag, const char* fmt, ...)
    {
        const char* id = Who(GBH_CALLER());
        char buf[2048];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf_s(buf, sizeof buf, _TRUNCATE, fmt, ap);
        va_end(ap);
        Log::WriteFrom(id, tag, buf);
    }

    // ---- settings -----------------------------------------------------------
    const char* Setting(const char* key, const char* dflt)  { return Settings::GetFor(Who(GBH_CALLER()), key, dflt); }
    int         SettingInt(const char* key, int dflt)       { return Settings::GetIntFor(Who(GBH_CALLER()), key, dflt); }
    float       SettingFloat(const char* key, float dflt)   { return Settings::GetFloatFor(Who(GBH_CALLER()), key, dflt); }
    int         SettingBool(const char* key, int dflt)      { return Settings::GetBoolFor(Who(GBH_CALLER()), key, dflt != 0) ? 1 : 0; }

    // ---- hooks --------------------------------------------------------------
    GbhHook HookCreate(void* target, void* detour, void** original, uint32_t flags)
    {
        return HookBroker::Install(Who(GBH_CALLER()), target, detour, original, flags);
    }
    int HookEnable(GbhHook h)                        { return HookBroker::Enable(h)  ? GBH_OK : GBH_ERR; }
    int HookDisable(GbhHook h)                       { return HookBroker::Disable(h) ? GBH_OK : GBH_ERR; }
    int HookEnableBatch(const GbhHook* h, int n)     { return HookBroker::EnableBatch(h, n) ? GBH_OK : GBH_ERR; }

    // ---- patches ------------------------------------------------------------
    GbhPatch PatchWrite(void* at, const void* bytes, size_t n)
    {
        if (!at || !bytes || n == 0 || n > GBH_MAX_PATCH_BYTES) return nullptr;
        return HookBroker::PatchWrite(Who(GBH_CALLER()), at, bytes, n);
    }
    int PatchRevert(GbhPatch p) { return HookBroker::PatchRevert(p) ? GBH_OK : GBH_ERR; }

    // ---- vtables ------------------------------------------------------------
    void* VtableClone(void* original, int slots)
    {
        if (!original || slots <= 0) return nullptr;
        return HookBroker::VtableClone(Who(GBH_CALLER()), original, slots);
    }
    int VtableSlot(void* clone, int slot, void* fn, void** originalFn)
    {
        if (!clone || slot < 0 || !fn) return GBH_ERR_ARG;
        return HookBroker::VtableSlot(Who(GBH_CALLER()), clone, slot, fn, originalFn) ? GBH_OK : GBH_ERR_CONFLICT;
    }
    int VtableApply(void* object, void* clone)
    {
        if (!object || !clone) return GBH_ERR_ARG;
        return HookBroker::VtableApply(Who(GBH_CALLER()), object, clone) ? GBH_OK : GBH_ERR;
    }
    void* VtableOriginal(void* clone, int slot) { return HookBroker::VtableOriginal(clone, slot); }

    // ---- introspection ------------------------------------------------------
    int         ModCount()                { return Host::Count(); }
    const char* ModIdAt(int i)            { return Host::IdAt(i); }
    int         ModIsLoaded(const char* id) { return Host::IsLoaded(id) ? 1 : 0; }

    GbhApi g_api;
    bool   g_built = false;
}

namespace Api
{
    const GbhApi* Table()
    {
        if (g_built) return &g_api;
        memset(&g_api, 0, sizeof g_api);
        g_api.struct_size = sizeof(GbhApi);
        g_api.abi_version = GBHOOK_ABI_VERSION;

        g_api.game_base         = GameBase;
        g_api.game_dir          = GameDir;
        g_api.mod_dir           = ModDir;
        g_api.framework_version = Version;

        g_api.alloc   = Alloc;
        g_api.realloc = Realloc;
        g_api.free    = Free;

        g_api.log  = LogLine;
        g_api.logf = Logf;

        g_api.setting       = Setting;
        g_api.setting_int   = SettingInt;
        g_api.setting_float = SettingFloat;
        g_api.setting_bool  = SettingBool;

        g_api.hook_create       = HookCreate;
        g_api.hook_enable       = HookEnable;
        g_api.hook_disable      = HookDisable;
        g_api.hook_enable_batch = HookEnableBatch;

        g_api.patch_write  = PatchWrite;
        g_api.patch_revert = PatchRevert;

        g_api.vtable_clone    = VtableClone;
        g_api.vtable_slot     = VtableSlot;
        g_api.vtable_apply    = VtableApply;
        g_api.vtable_original = VtableOriginal;

        g_api.mod_count     = ModCount;
        g_api.mod_id_at     = ModIdAt;
        g_api.mod_is_loaded = ModIsLoaded;

        g_built = true;
        return &g_api;
    }
}
