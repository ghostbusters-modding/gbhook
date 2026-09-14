// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Nothing here transfers ownership: every buffer is the caller's, every string handed back is ours and thread-local.
#include "Api.h"
#include "Host.h"
#include "../core/Framework.h"
#include "../core/HookBroker.h"
#include "../services/Commands.h"
#include "../services/Events.h"
#include "../services/Files.h"
#include "../services/FrameHook.h"
#include "../services/Game.h"
#include "../services/Hud.h"
#include "../services/InputInject.h"
#include "../services/NativeMenu.h"
#include "../services/Registry.h"
#include "../services/Services.h"
#include "../services/Levels.h"
#include "../services/Actors.h"
#include "../services/Attr.h"
#include "../core/ProcessMemory.h"
#include "input/Dik.h"

#include <windows.h>
#include <cstdarg>
#include <cstring>
#include <string>
#include <vector>

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

    // ---- events -------------------------------------------------------------
    GbhSub OnFrame(GbhFrameFn f, void* u) { return Events::Subscribe(Events::Frame, Who(GBH_CALLER()), (void*)f, u); }
    GbhSub OnPump(GbhFrameFn f, void* u)  { return Events::Subscribe(Events::Pump,  Who(GBH_CALLER()), (void*)f, u); }
    GbhSub OnLevel(GbhLevelFn f, void* u) { return Events::Subscribe(Events::Level, Who(GBH_CALLER()), (void*)f, u); }
    GbhSub OnActor(GbhActorFn f, void* u) { return Events::Subscribe(Events::Actor, Who(GBH_CALLER()), (void*)f, u); }
    void   Unsubscribe(GbhSub s)          { Events::Unsubscribe(s); }

    // ---- game model ---------------------------------------------------------
    void*    GameSingleton() { return Game::Singleton(); }
    void*    LocalPlayer()   { return Game::LocalPlayer(); }
    uint32_t GameThreadId()  { return (uint32_t)FrameHook::GameThreadId(); }
    int      IsGameThread()  { return FrameHook::IsGameThread() ? 1 : 0; }

    thread_local char t_stem[64];
    const char* LevelName()
    {
        Game::LevelStem(t_stem, sizeof t_stem);
        return t_stem;
    }

    void Fill(GbhRegistryEntry& o, const Registry::Entry& e)
    {
        memset(&o, 0, sizeof o);
        o.ptr        = e.ptr;
        o.generation = e.generation;
        strncpy_s(o.cls,  e.cls.c_str(),  _TRUNCATE);
        strncpy_s(o.name, e.name.c_str(), _TRUNCATE);
    }

    int RegistrySnapshot(GbhRegistryEntry* buf, int cap)
    {
        if (!buf && cap == 0) return Registry::Count();
        if (!buf || cap <= 0) return GBH_ERR_ARG;
        std::vector<Registry::Entry> list;
        Registry::Snapshot(list);
        const int n = (int)list.size() < cap ? (int)list.size() : cap;
        for (int i = 0; i < n; ++i) Fill(buf[i], list[(size_t)i]);
        return n;
    }

    int RegistryFind(const char* name, GbhRegistryEntry* out)
    {
        if (!name || !out) return GBH_ERR_ARG;
        Registry::Entry e;
        if (!Registry::Find(name, e)) return GBH_ERR_NOT_FOUND;
        Fill(*out, e);
        return GBH_OK;
    }

    // ---- commands, input, hud, level ------------------------------------------
    // Game thread by default: almost every mod command ends up touching a GB:: native.
    int CommandRegister(const char* name, GbhCommandFn fn, void* user, const char* help)
    {
        return Commands::Register(Who(GBH_CALLER()), name, fn, user, help, Commands::kGameThread);
    }
    int  CommandRun(const char* line)         { return Commands::Execute(line); }
    int  CommandQueue(const char* line)       { return Commands::Queue(line); }
    void InputSetKey(int dik, int down)       { InputInject::SetKey(dik, down != 0); }
    int  InputDikFromName(const char* name)   { return Dik::FromName(name); }
    void HudMessage(const char* t, float s)   { Hud::Message(t, s); }

    // Through the command channel, so it is queued onto the right thread and logged like any other level change.
    int LevelChain(const char* level, const char* checkpoint)
    {
        if (!level || !*level) return GBH_ERR_ARG;
        char line[256];
        if (checkpoint && *checkpoint) _snprintf_s(line, sizeof line, _TRUNCATE, "level %s %s", level, checkpoint);
        else                           _snprintf_s(line, sizeof line, _TRUNCATE, "level %s", level);
        return Commands::Execute(line);
    }

    // ---- the native front end ----------------------------------------------------
    int  NativeRowClaim(int row, GbhRowFn fn, void* user) { return NativeMenu::ClaimRow(Who(GBH_CALLER()), row, fn, user); }
    int  NativeRowLabel(int row, const char* label)       { return NativeMenu::SetRowLabel(Who(GBH_CALLER()), row, label); }
    int  NativeOpen(const GbhNativeMenuDesc* d)           { return NativeMenu::OpenPage(d); }
    int  NativeAddRow(const char* label, int action)      { return NativeMenu::AddRow(label, action); }
    void NativeRefresh()                                  { NativeMenu::Refresh(); }

    // ---- files: the engine's tables take no lock, so only the thread the engine itself runs on -----
    int FileList(const char* dir, const char* pattern, void (*cb)(const char*, void*), void* user)
    {
        if (!pattern || !cb) return GBH_ERR_ARG;
        if (!FrameHook::IsEngineThread()) return GBH_ERR_WRONG_THREAD;
        std::vector<std::string> names;
        const int n = Files::List(dir, pattern, names);
        if (n < 0) return n;
        for (const std::string& s : names) cb(s.c_str(), user);   // borrowed for the call only
        return (int)names.size();
    }

    int FileRead(const char* path, void* buf, int cap)
    {
        if (!path || cap < 0 || (cap > 0 && !buf)) return GBH_ERR_ARG;
        if (!FrameHook::IsEngineThread()) return GBH_ERR_WRONG_THREAD;
        std::vector<unsigned char> bytes;
        const int n = Files::Read(path, bytes);
        if (n < 0) return n;
        if (!buf && cap == 0) return n;
        if (n > cap) return GBH_ERR_TRUNCATED;
        if (n > 0) memcpy(buf, bytes.data(), (size_t)n);
        return n;
    }

    // ---- services --------------------------------------------------------------
    int ServicePublish(const char* name, const void* table, uint32_t size)
    {
        if (!name || !*name || !table || size < sizeof(uint32_t)) return GBH_ERR_ARG;
        return Services::Publish(Who(GBH_CALLER()), name, table, size);
    }
    const void* ServiceFind(const char* name, uint32_t* size) { return name ? Services::Find(name, size) : nullptr; }
    int         ServiceCount()                                { return Services::Count(); }
    const char* ServiceNameAt(int i)                          { return Services::NameAt(i); }
    const char* ServiceOwner(const char* name)                { return name ? Services::OwnerOf(name) : nullptr; }

    // ---- memory ----------------------------------------------------------------
    int MemRead(const void* src, void* dst, size_t n)
    {
        if (!src || !dst || n == 0) return 0;
        ProcessMemory mem;
        return mem.Read(reinterpret_cast<uintptr_t>(src), dst, n) ? 1 : 0;
    }

    // ---- levels: the same thread rule as files, and for the same reason ---------------
    int LevelList(int kind, void (*cb)(const char*, void*), void* user)
    {
        if (!cb || (kind != GBH_LEVELS_CAREER && kind != GBH_LEVELS_CUSTOM)) return GBH_ERR_ARG;
        if (!FrameHook::IsEngineThread()) return GBH_ERR_WRONG_THREAD;
        std::vector<std::string> stems;
        const int n = kind == GBH_LEVELS_CAREER ? Levels::Career(stems) : Levels::Custom(stems);
        if (n < 0) return n;
        for (const std::string& s : stems) cb(s.c_str(), user);
        return (int)stems.size();
    }

    int LevelCheckpoints(const char* stem, void (*cb)(const char*, void*), void* user)
    {
        if (!stem || !*stem || !cb) return GBH_ERR_ARG;
        if (!FrameHook::IsEngineThread()) return GBH_ERR_WRONG_THREAD;
        std::vector<std::string> names;
        const int n = Levels::Checkpoints(stem, names);
        if (n < 0) return n;
        for (const std::string& s : names) cb(s.c_str(), user);
        return (int)names.size();
    }

    // ---- actors: the caller's struct_size in the first entry is the stride ------------------
    // The pointer sits after the size's alignment padding, so the smallest useful entry reaches past it.
    constexpr uint32_t kActorMinStride = offsetof(GbhActorInfo, ptr) + sizeof(void*);

    int ActorSnapshot(GbhActorInfo* buf, int cap)
    {
        if (!buf && cap == 0) return Actors::Count();
        if (!buf || cap <= 0) return GBH_ERR_ARG;
        const uint32_t stride = buf->struct_size;
        if (stride < kActorMinStride) return GBH_ERR_ARG;
        return Actors::Snapshot(buf, cap, stride);
    }
    int ActorFind(const char* name, GbhActorInfo* out)
    {
        if (!name || !out) return GBH_ERR_ARG;
        if (out->struct_size < kActorMinStride) return GBH_ERR_ARG;
        return Actors::Find(name, out, out->struct_size);
    }
    int ActorIsA(void* actor, const char* cls)
    {
        if (!actor || !cls || !*cls) return GBH_ERR_ARG;
        return Actors::IsA(actor, cls);
    }

    // ---- attributes ----------------------------------------------------------------
    int AttrCount()                                   { return Attr::Count(); }
    int AttrAt(int i, GbhAttrInfo* out)
    {
        if (!out || out->struct_size < sizeof(uint32_t)) return GBH_ERR_ARG;
        return Attr::At(i, out, out->struct_size);
    }
    int AttrGet(const char* key, char* out, int cap)
    {
        if (!key || !out || cap <= 0) return GBH_ERR_ARG;
        return Attr::Get(key, out, cap);
    }
    int AttrGetFloat(const char* key, float* out)     { return (key && out) ? Attr::GetFloat(key, out) : GBH_ERR_ARG; }
    int AttrSet(const char* key, const char* value)
    {
        if (!key || !value) return GBH_ERR_ARG;
        if (!FrameHook::IsGameThread()) return GBH_ERR_WRONG_THREAD;
        return Attr::Set(key, value);
    }

    // ---- keys ----------------------------------------------------------------------
    GbhSub OnKey(GbhKeyFn f, void* u)   { return Events::Subscribe(Events::Key,  Who(GBH_CALLER()), (void*)f, u); }
    GbhSub OnChar(GbhCharFn f, void* u) { return Events::Subscribe(Events::Char, Who(GBH_CALLER()), (void*)f, u); }

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

        g_api.on_frame            = OnFrame;
        g_api.on_pump             = OnPump;
        g_api.on_level            = OnLevel;
        g_api.on_actor_registered = OnActor;
        g_api.unsubscribe         = Unsubscribe;

        g_api.game_singleton    = GameSingleton;
        g_api.local_player      = LocalPlayer;
        g_api.game_thread_id    = GameThreadId;
        g_api.is_game_thread    = IsGameThread;
        g_api.level_name        = LevelName;
        g_api.registry_snapshot = RegistrySnapshot;
        g_api.registry_find     = RegistryFind;

        g_api.command_register    = CommandRegister;
        g_api.command_run         = CommandRun;
        g_api.command_queue       = CommandQueue;
        g_api.input_set_key       = InputSetKey;
        g_api.input_dik_from_name = InputDikFromName;
        g_api.hud_message         = HudMessage;
        g_api.level_chain         = LevelChain;

        g_api.native_row_claim       = NativeRowClaim;
        g_api.native_row_label       = NativeRowLabel;
        g_api.native_submenu_open    = NativeOpen;
        g_api.native_submenu_add_row = NativeAddRow;
        g_api.native_submenu_refresh = NativeRefresh;
        g_api.file_list              = FileList;
        g_api.file_read              = FileRead;

        g_api.service_publish = ServicePublish;
        g_api.service_find    = ServiceFind;
        g_api.service_count   = ServiceCount;
        g_api.service_name_at = ServiceNameAt;
        g_api.service_owner   = ServiceOwner;

        g_api.mem_read = MemRead;

        g_api.level_list        = LevelList;
        g_api.level_checkpoints = LevelCheckpoints;

        g_api.actor_snapshot = ActorSnapshot;
        g_api.actor_find     = ActorFind;
        g_api.actor_is_a     = ActorIsA;

        g_api.attr_count     = AttrCount;
        g_api.attr_at        = AttrAt;
        g_api.attr_get       = AttrGet;
        g_api.attr_get_float = AttrGetFloat;
        g_api.attr_set       = AttrSet;

        g_api.on_key  = OnKey;
        g_api.on_char = OnChar;

        g_built = true;
        return &g_api;
    }
}
