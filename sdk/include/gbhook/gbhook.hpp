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
}
