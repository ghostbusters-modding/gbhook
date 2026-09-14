// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Header-only C++ conveniences over gbhook.h. Everything compiles into the mod, so no allocation crosses the boundary.

#include "gbhook.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace gbh
{
    // The table, stashed once by bind() so nothing has to thread it through.
    inline const GbhApi*& api_ref()           { static const GbhApi* a = nullptr; return a; }
    inline const GbhApi*  api()               { return api_ref(); }
    inline void           bind(const GbhApi* a) { api_ref() = a; }

    // ---- log: your mod id is added by the framework, so `tag` names the subsystem ----
    inline void log(const char* tag, const char* line) { if (api()) api()->log(tag, line); }
    inline void logf(const char* tag, const char* fmt, ...)
    {
        if (!api()) return;
        char buf[1024];
        va_list ap;
        va_start(ap, fmt);
        vsnprintf(buf, sizeof buf, fmt, ap);
        va_end(ap);
        api()->log(tag, buf);
    }

    // ---- settings, in your mod's own namespace ----
    inline std::string setting(const char* key, const char* dflt = "")
    {
        const char* v = api() ? api()->setting(key, dflt) : dflt;
        return v ? std::string(v) : std::string();
    }
    inline int   setting_int(const char* key, int d = 0)      { return api() ? api()->setting_int(key, d) : d; }
    inline float setting_float(const char* key, float d = 0)  { return api() ? api()->setting_float(key, d) : d; }
    inline bool  setting_bool(const char* key, bool d = true) { return api() ? api()->setting_bool(key, d ? 1 : 0) != 0 : d; }

    // ---- environment ----
    inline void* game_base() { return api() ? api()->game_base() : nullptr; }
    inline std::string game_dir()
    {
        const char* s = api() ? api()->game_dir() : nullptr;
        return s ? std::string(s) : std::string();
    }
    inline std::string mod_dir()
    {
        const char* s = api() ? api()->mod_dir() : nullptr;
        return s ? std::string(s) : std::string();
    }

    // An absolute address from a ghost.exe-relative offset.
    template <typename T = void>
    inline T* at(uintptr_t rva) { return reinterpret_cast<T*>(static_cast<char*>(game_base()) + rva); }

    // ---- hooks ----
    inline GbhHook hook(void* target, void* detour, void** original, uint32_t flags = GBH_HOOK_NONE)
    {
        return api() ? api()->hook_create(target, detour, original, flags) : nullptr;
    }
    inline GbhHook hook_at(uintptr_t rva, void* detour, void** original, uint32_t flags = GBH_HOOK_NONE)
    {
        return hook(at(rva), detour, original, flags);
    }
    inline bool enable_batch(const GbhHook* hooks, int n)
    {
        return api() && api()->hook_enable_batch(hooks, n) == GBH_OK;
    }

    // ---- a byte patch reverted when the guard dies, unless released ----
    class Patch
    {
    public:
        Patch() = default;
        Patch(void* at, const void* bytes, size_t n) : p_(api() ? api()->patch_write(at, bytes, n) : nullptr) {}
        Patch(const Patch&) = delete;
        Patch& operator=(const Patch&) = delete;
        Patch(Patch&& o) noexcept : p_(o.p_) { o.p_ = nullptr; }
        Patch& operator=(Patch&& o) noexcept { reset(); p_ = o.p_; o.p_ = nullptr; return *this; }
        ~Patch() { reset(); }

        bool     ok() const   { return p_ != nullptr; }
        GbhPatch release()    { GbhPatch p = p_; p_ = nullptr; return p; }
        void     reset()      { if (p_ && api()) api()->patch_revert(p_); p_ = nullptr; }
    private:
        GbhPatch p_ = nullptr;
    };

    // ---- a cloned vtable: claim slots, then point objects at it. The copy is the framework's for the process ----
    class VtableOverride
    {
    public:
        VtableOverride(void* original, int slots) : clone_(api() ? api()->vtable_clone(original, slots) : nullptr) {}

        bool  ok() const    { return clone_ != nullptr; }
        void* clone() const { return clone_; }

        template <typename Fn>
        bool set(int slot, Fn fn, Fn* original = nullptr)
        {
            void* orig = nullptr;
            if (!clone_ || !api()) return false;
            if (api()->vtable_slot(clone_, slot, reinterpret_cast<void*>(fn), &orig) != GBH_OK) return false;
            if (original) *original = reinterpret_cast<Fn>(orig);
            return true;
        }
        bool apply(void* object) const { return clone_ && api() && api()->vtable_apply(object, clone_) == GBH_OK; }

        template <typename Fn>
        Fn original(int slot) const
        {
            return reinterpret_cast<Fn>(clone_ && api() ? api()->vtable_original(clone_, slot) : nullptr);
        }
    private:
        void* clone_;
    };

    // ---- events: a Sub unsubscribes when it dies. release() keeps it for the process, which is the usual case ----
    class Sub
    {
    public:
        Sub() = default;
        explicit Sub(GbhSub s) : s_(s) {}
        Sub(const Sub&) = delete;
        Sub& operator=(const Sub&) = delete;
        Sub(Sub&& o) noexcept : s_(o.s_) { o.s_ = nullptr; }
        Sub& operator=(Sub&& o) noexcept { reset(); s_ = o.s_; o.s_ = nullptr; return *this; }
        ~Sub() { reset(); }

        bool   active() const { return s_ != nullptr; }
        GbhSub release()      { GbhSub s = s_; s_ = nullptr; return s; }
        void   reset()        { if (s_ && api()) api()->unsubscribe(s_); s_ = nullptr; }
    private:
        GbhSub s_ = nullptr;
    };

    inline Sub on_frame(GbhFrameFn fn, void* user = nullptr)            { return api() ? Sub(api()->on_frame(fn, user)) : Sub(); }
    inline Sub on_pump(GbhFrameFn fn, void* user = nullptr)             { return api() ? Sub(api()->on_pump(fn, user)) : Sub(); }
    inline Sub on_level(GbhLevelFn fn, void* user = nullptr)            { return api() ? Sub(api()->on_level(fn, user)) : Sub(); }
    inline Sub on_actor_registered(GbhActorFn fn, void* user = nullptr) { return api() ? Sub(api()->on_actor_registered(fn, user)) : Sub(); }

    // ---- game model ----
    inline void* game_singleton() { return api() ? api()->game_singleton() : nullptr; }
    inline void* local_player()   { return api() ? api()->local_player() : nullptr; }
    inline bool  is_game_thread() { return api() && api()->is_game_thread() != 0; }
    inline std::string level_name()
    {
        const char* s = api() ? api()->level_name() : nullptr;
        return s ? std::string(s) : std::string();
    }

    // The vector owns its storage in your module; the framework only fills the buffer it is handed.
    inline std::vector<GbhRegistryEntry> registry()
    {
        std::vector<GbhRegistryEntry> out;
        if (!api()) return out;
        int n = api()->registry_snapshot(nullptr, 0);
        if (n <= 0) return out;
        out.resize(static_cast<size_t>(n) + 64);   // headroom for registrations landing between the two calls
        n = api()->registry_snapshot(out.data(), static_cast<int>(out.size()));
        out.resize(n > 0 ? static_cast<size_t>(n) : 0);
        return out;
    }
    inline bool registry_find(const char* name, GbhRegistryEntry& out)
    {
        return api() && api()->registry_find(name, &out) == GBH_OK;
    }

    // ---- commands: registered as "<id>.<name>"; the handler runs on the game thread ----
    inline int command_register(const char* name, GbhCommandFn fn, void* user = nullptr, const char* help = "")
    {
        return api() ? api()->command_register(name, fn, user, help) : GBH_ERR_STATE;
    }
    inline int run(const char* line)   { return api() ? api()->command_run(line)   : GBH_ERR_STATE; }
    inline int queue(const char* line) { return api() ? api()->command_queue(line) : GBH_ERR_STATE; }

    // ---- input, hud, level ----
    inline int  dik(const char* name)                       { return api() ? api()->input_dik_from_name(name) : -1; }
    inline void key(int dik, bool down)                     { if (api()) api()->input_set_key(dik, down ? 1 : 0); }
    inline void hud(const char* text, float seconds = 3.0f) { if (api()) api()->hud_message(text, seconds); }
    inline int  level(const char* stem, const char* checkpoint = nullptr)
    {
        return api() ? api()->level_chain(stem, checkpoint) : GBH_ERR_STATE;
    }

    // ---- the game's own main menu ----
    inline int  native_row_claim(int row, GbhRowFn fn, void* user = nullptr)  { return api() ? api()->native_row_claim(row, fn, user) : GBH_ERR_STATE; }
    inline int  native_row_label(int row, const char* label)                  { return api() ? api()->native_row_label(row, label) : GBH_ERR_STATE; }
    inline int  native_submenu_open(const GbhNativeMenuDesc& d)               { return api() ? api()->native_submenu_open(&d) : GBH_ERR_STATE; }
    inline int  native_submenu_add_row(const char* label, int action)         { return api() ? api()->native_submenu_add_row(label, action) : GBH_ERR_STATE; }
    inline void native_submenu_refresh()                                      { if (api()) api()->native_submenu_refresh(); }

    // ---- files, engine main thread only: a command, on_frame or on_pump ----
    inline std::vector<std::string> files(const char* dir, const char* pattern)
    {
        std::vector<std::string> out;
        if (!api()) return out;
        api()->file_list(dir, pattern, [](const char* name, void* user) { static_cast<std::vector<std::string>*>(user)->push_back(name); }, &out);
        return out;
    }
    inline std::vector<unsigned char> file_read(const char* path)
    {
        std::vector<unsigned char> out;
        if (!api()) return out;
        const int n = api()->file_read(path, nullptr, 0);
        if (n <= 0) return out;
        out.resize(static_cast<size_t>(n));
        const int got = api()->file_read(path, out.data(), n);
        out.resize(got > 0 ? static_cast<size_t>(got) : 0);
        return out;
    }

    // ---- the block appended after file_read; every wrapper answers as if unsupported on an older framework ----
    inline bool has_services() { return gbh_api_has(api(), on_char); }

    // ---- services: publish a table of your own, or find another mod's. A found table is gated on its size ----
    inline int service_publish(const char* name, const void* table, uint32_t size)
    {
        return has_services() ? api()->service_publish(name, table, size) : GBH_ERR_UNSUPPORTED;
    }
    template <typename T>
    inline const T* service_find(const char* name)
    {
        if (!has_services()) return nullptr;
        uint32_t size = 0;
        const void* t = api()->service_find(name, &size);
        return (t && size >= sizeof(uint32_t)) ? static_cast<const T*>(t) : nullptr;
    }

    // ---- memory ----
    inline bool mem_read(const void* src, void* dst, size_t n) { return has_services() && api()->mem_read(src, dst, n) == 1; }
    template <typename T>
    inline bool read(const void* src, T& out) { return mem_read(src, &out, sizeof(T)); }

    // ---- levels, engine main thread only ----
    inline std::vector<std::string> levels(int kind)
    {
        std::vector<std::string> out;
        if (!has_services()) return out;
        api()->level_list(kind, [](const char* s, void* u) { static_cast<std::vector<std::string>*>(u)->push_back(s); }, &out);
        return out;
    }
    inline std::vector<std::string> checkpoints(const char* stem)
    {
        std::vector<std::string> out;
        if (!has_services()) return out;
        api()->level_checkpoints(stem, [](const char* s, void* u) { static_cast<std::vector<std::string>*>(u)->push_back(s); }, &out);
        return out;
    }

    // ---- actors: the engine's own list. The vector owns its storage in your module ----
    inline std::vector<GbhActorInfo> actors()
    {
        std::vector<GbhActorInfo> out;
        if (!has_services()) return out;
        int n = api()->actor_snapshot(nullptr, 0);
        if (n <= 0) return out;
        out.resize(static_cast<size_t>(n) + 64);   // headroom for spawns landing between the two calls
        out[0].struct_size = sizeof(GbhActorInfo);
        n = api()->actor_snapshot(out.data(), static_cast<int>(out.size()));
        out.resize(n > 0 ? static_cast<size_t>(n) : 0);
        return out;
    }
    inline bool actor_find(const char* name, GbhActorInfo& out)
    {
        out.struct_size = sizeof(GbhActorInfo);
        return has_services() && api()->actor_find(name, &out) == GBH_OK;
    }
    inline bool actor_is_a(void* actor, const char* cls) { return has_services() && api()->actor_is_a(actor, cls) == 1; }

    // ---- attributes ----
    inline std::vector<GbhAttrInfo> attrs()
    {
        std::vector<GbhAttrInfo> out;
        if (!has_services()) return out;
        const int n = api()->attr_count();
        for (int i = 0; i < n; ++i)
        {
            GbhAttrInfo a;
            a.struct_size = sizeof a;
            if (api()->attr_at(i, &a) == GBH_OK) out.push_back(a);
        }
        return out;
    }
    inline std::string attr(const char* key)
    {
        char buf[64] = { 0 };
        if (!has_services() || api()->attr_get(key, buf, sizeof buf) != GBH_OK) return std::string();
        return std::string(buf);
    }
    inline bool attr_float(const char* key, float& out) { return has_services() && api()->attr_get_float(key, &out) == GBH_OK; }
    inline bool attr_bool(const char* key, bool& out)
    {
        float f = 0;
        if (!attr_float(key, f)) return false;
        out = f != 0.0f;
        return true;
    }
    inline int attr_set(const char* key, const char* value) { return has_services() ? api()->attr_set(key, value) : GBH_ERR_UNSUPPORTED; }

    // ---- keys, message thread ----
    inline Sub on_key(GbhKeyFn fn, void* user = nullptr)   { return has_services() ? Sub(api()->on_key(fn, user)) : Sub(); }
    inline Sub on_char(GbhCharFn fn, void* user = nullptr) { return has_services() ? Sub(api()->on_char(fn, user)) : Sub(); }
}
