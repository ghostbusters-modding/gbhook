// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Reads are guarded leaves over the pointer chains. The natives run on the game thread, each inside its own leaf.
#include "Attr.h"
#include "Commands.h"
#include "Game.h"
#include "../attr/Catalogue.h"
#include "../core/Framework.h"
#include "../core/Seh.h"
#include "gb/Globals.h"
#include "gb/Offsets.h"
#include "gb/Structs/CCharacter.h"
#include "gb/Structs/CGameView.h"
#include "gb/Structs/CGhostbuster.h"
#include "gb/Generated/GBApi.generated.h"

#include <cstdio>
#include <cstring>

namespace
{
    constexpr float kTimeRamp      = 0.25f;   // what setTimeFactor ramps over, the same for a set and a reset
    constexpr float kTimeNormal    = 1.0f;
    constexpr float kGravityNormal = -32.0f;  // what resetGravity writes

    // ---- plain-C frames for the guard, so an unmapped or torn pointer reads as a failure ------------------------
    bool ReadPtr(const void* p, void** out)
    {
        if (!p) return false;
        GBH_SEH_TRY { *out = *(void* const*)p; return true; }
        GBH_SEH_EXCEPT { return false; }
    }
    bool ReadU8(const void* p, uint8_t* out)
    {
        if (!p) return false;
        GBH_SEH_TRY { *out = *(const uint8_t*)p; return true; }
        GBH_SEH_EXCEPT { return false; }
    }
    bool ReadU32(const void* p, uint32_t* out)
    {
        if (!p) return false;
        GBH_SEH_TRY { *out = *(const uint32_t*)p; return true; }
        GBH_SEH_EXCEPT { return false; }
    }
    bool ReadI32(const void* p, int32_t* out)
    {
        if (!p) return false;
        GBH_SEH_TRY { *out = *(const int32_t*)p; return true; }
        GBH_SEH_EXCEPT { return false; }
    }
    bool ReadF32(const void* p, float* out)
    {
        if (!p) return false;
        GBH_SEH_TRY { *out = *(const float*)p; return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    // ---- the natives, each in its own leaf, so a fault inside the engine answers GBH_ERR --------------------------
    bool CallInvulnerable(void* self, bool on)
    {
        GBH_SEH_TRY { GB::CCharacter::setInvulnerableFlag(self, on); return true; }
        GBH_SEH_EXCEPT { return false; }
    }
    bool CallGiant(void* self, bool on)
    {
        GBH_SEH_TRY { GB::CGhostbuster::enableGiantBossMode(self, on); return true; }
        GBH_SEH_EXCEPT { return false; }
    }
    bool CallTorpedo(void* self, bool on)
    {
        GBH_SEH_TRY { GB::CGhostbuster::enableProtonTorpedo(self, on); return true; }
        GBH_SEH_EXCEPT { return false; }
    }
    bool CallHunt(void* self, bool on)
    {
        GBH_SEH_TRY { GB::CGhostbuster::toggleHuntMode(self, on); return true; }
        GBH_SEH_EXCEPT { return false; }
    }
    bool CallSetGravity(Vector3 g)
    {
        GBH_SEH_TRY { GB::Global::setGravity(g); return true; }
        GBH_SEH_EXCEPT { return false; }
    }
    bool CallResetGravity()
    {
        GBH_SEH_TRY { GB::Global::resetGravity(); return true; }
        GBH_SEH_EXCEPT { return false; }
    }
    bool CallSetTimeFactor(float f, float ramp)
    {
        GBH_SEH_TRY { GB::Global::setTimeFactor(f, ramp); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    // ---- the reads ---------------------------------------------------------------------------------------------
    const char* Global(uintptr_t rva) { return gameBase ? gameBase + rva : nullptr; }

    // A wrong pointer into mapped memory reads garbage rather than faulting, so a float is also checked for sanity.
    bool Sane(float f) { return f > -1.0e9f && f < 1.0e9f; }

    bool ReadFlag8(uintptr_t off, float* out)
    {
        const char* p = static_cast<const char*>(Game::LocalPlayer());
        uint8_t v = 0;
        if (!p || !ReadU8(p + off, &v)) return false;
        *out = v ? 1.0f : 0.0f;
        return true;
    }

    bool ReadGod(float* out)
    {
        const char* p = static_cast<const char*>(Game::LocalPlayer());
        uint32_t v = 0;
        if (!p || !ReadU32(p + CCharacter::invulnerable, &v)) return false;
        *out = v ? 1.0f : 0.0f;
        return true;
    }

    bool ReadHunt(float* out)
    {
        const char* p = static_cast<const char*>(Game::LocalPlayer());
        void* pack = nullptr;
        uint8_t v = 0;
        if (!p || !ReadPtr(p + CGhostbuster::pack, &pack) || !pack) return false;
        if (!ReadU8(static_cast<const char*>(pack) + CGhostbuster::packHunt, &v)) return false;
        *out = v ? 1.0f : 0.0f;
        return true;
    }

    bool ReadGravity(float* out)
    {
        void* wrap = nullptr;
        void* scene = nullptr;
        float y = 0.0f;
        if (!ReadPtr(Global(Globals::physicsWrapper), &wrap) || !wrap) return false;
        if (!ReadPtr(wrap, &scene) || !scene) return false;
        if (!ReadF32(static_cast<const char*>(scene) + Globals::sceneGravity + 4, &y) || !Sane(y)) return false;
        *out = y;
        return true;
    }

    bool ReadTime(float* out)
    {
        void* t = nullptr;
        uint8_t on = 0;
        float f = 0.0f;
        if (!ReadPtr(Global(Globals::timeBlock), &t) || !t) return false;
        if (!ReadU8(static_cast<const char*>(t) + Globals::timeOverride, &on)) return false;
        if (!on) { *out = kTimeNormal; return true; }
        if (!ReadF32(static_cast<const char*>(t) + Globals::timeTarget, &f) || !Sane(f) || f <= 0.0f) return false;
        *out = f;
        return true;
    }

    const char* MainView()
    {
        void* v = nullptr;
        return ReadPtr(Global(Offsets::gMainView), &v) ? static_cast<const char*>(v) : nullptr;
    }

    bool ReadViewFloat(uintptr_t off, float* out)
    {
        const char* v = MainView();
        float f = 0.0f;
        if (!v || !ReadF32(v + off, &f) || !Sane(f)) return false;
        *out = f;
        return true;
    }

    bool ReadCamMode(float* out)
    {
        const char* v = MainView();
        int32_t m = 0;
        if (!v || !ReadI32(v + CGameView::mode, &m)) return false;
        *out = (float)m;
        return true;
    }

    bool Read(const Catalogue::Entry& e, float* out)
    {
        switch (e.id)
        {
        case Catalogue::kGod:     return ReadGod(out);
        case Catalogue::kGiant:   return ReadFlag8(CGhostbuster::giantBoss, out);
        case Catalogue::kTorpedo: return ReadFlag8(CGhostbuster::protonTorpedo, out);
        case Catalogue::kHunt:    return ReadHunt(out);
        case Catalogue::kGravity: return ReadGravity(out);
        case Catalogue::kTime:    return ReadTime(out);
        case Catalogue::kFov:     return ReadViewFloat(CGameView::fov, out);
        case Catalogue::kCamDist: return ReadViewFloat(CGameView::dist, out);
        case Catalogue::kCamMode: return ReadCamMode(out);
        }
        return false;
    }

    // ---- the writes: game thread, through the natives -----------------------------------------------------------
    bool Write(const Catalogue::Entry& e, const Catalogue::Parsed& p)
    {
        const bool on = p.value != 0.0f;
        void* self = Game::LocalPlayer();
        switch (e.id)
        {
        case Catalogue::kGod:     return self && CallInvulnerable(self, on);
        case Catalogue::kGiant:   return self && CallGiant(self, on);
        case Catalogue::kTorpedo: return self && CallTorpedo(self, on);
        case Catalogue::kHunt:    return self && CallHunt(self, on);
        case Catalogue::kGravity: return p.reset ? CallResetGravity() : CallSetGravity(Vector3{ 0.0f, p.value, 0.0f });
        case Catalogue::kTime:    return CallSetTimeFactor(p.reset ? kTimeNormal : p.value, kTimeRamp);
        default:                  return false;
        }
    }

    // What a reset lands on, for the log line when the re-read after it fails.
    float Requested(const Catalogue::Entry& e, const Catalogue::Parsed& p)
    {
        if (!p.reset) return p.value;
        return e.id == Catalogue::kGravity ? kGravityNormal : kTimeNormal;
    }

    // ---- the command ---------------------------------------------------------------------------------------------
    // One "key value" line, "(unreadable)" when the state is not there. Answers whether it was.
    bool Show(const Catalogue::Entry& e)
    {
        char v[48];
        float f = 0.0f;
        const bool ok = Read(e, &f);
        if (ok) Catalogue::Format(e, f, v, sizeof v);
        else    strcpy_s(v, "(unreadable)");
        Log::Writef("RES", "  %-8s %s%s", e.key, v, e.writable ? "" : "  (read-only)");
        return ok;
    }

    int CmdAttr(int argc, const char* const* argv, const char** err, void*)
    {
        if (argc == 0)
        {
            Log::Writef("RES", "%d attribute(s):", Catalogue::Count());
            for (int i = 0; i < Catalogue::Count(); ++i) Show(*Catalogue::At(i));
            return GBH_OK;
        }
        const Catalogue::Entry* e = Catalogue::Find(argv[0]);
        if (!e) { *err = "unknown key (try `attr`)"; return GBH_ERR_NOT_FOUND; }
        if (argc == 1)
        {
            if (!Show(*e)) { *err = "not readable now (no level?)"; return GBH_ERR_STATE; }
            return GBH_OK;
        }
        const int rc = Attr::Set(e->key, argv[1]);
        switch (rc)
        {
        case GBH_OK:              return GBH_OK;
        case GBH_ERR_UNSUPPORTED: *err = "read-only: no engine native sets it"; break;
        case GBH_ERR_STATE:       *err = "not settable now (no level?)"; break;
        case GBH_ERR_ARG:
        {
            // The range belongs in the message, and `err` must outlive the call: one buffer, game thread only.
            static char why[128];
            const char* reason = nullptr;
            Catalogue::Parsed p;
            Catalogue::Parse(*e, argv[1], 0.0f, &p, &reason);
            if (!reason) reason = "bad value";
            if (e->min != 0.0f || e->max != 0.0f) _snprintf_s(why, sizeof why, _TRUNCATE, "%s (%g..%g)", reason, e->min, e->max);
            else                                  _snprintf_s(why, sizeof why, _TRUNCATE, "%s", reason);
            *err = why;
            break;
        }
        default:                  *err = "the engine's setter faulted"; break;
        }
        return rc;
    }
}

namespace Attr
{
    int Count() { return Catalogue::Count(); }

    int At(int i, GbhAttrInfo* out, uint32_t stride)
    {
        const Catalogue::Entry* e = Catalogue::At(i);
        if (!e) return GBH_ERR_ARG;
        GbhAttrInfo info;
        memset(&info, 0, sizeof info);
        strncpy_s(info.key,  e->key,  _TRUNCATE);
        strncpy_s(info.unit, e->unit, _TRUNCATE);
        strncpy_s(info.help, e->help, _TRUNCATE);
        info.type     = (uint32_t)e->type;
        info.writable = e->writable ? 1u : 0u;
        info.min      = e->min;
        info.max      = e->max;
        const uint32_t n = stride < sizeof info ? stride : (uint32_t)sizeof info;
        info.struct_size = n;
        memcpy(out, &info, n);
        return GBH_OK;
    }

    int Get(const char* key, char* out, int cap)
    {
        const Catalogue::Entry* e = Catalogue::Find(key);
        if (!e) return GBH_ERR_NOT_FOUND;
        float f = 0.0f;
        if (!Read(*e, &f)) { out[0] = '\0'; return GBH_ERR_STATE; }
        return Catalogue::Format(*e, f, out, cap) < cap ? GBH_OK : GBH_ERR_TRUNCATED;
    }

    int GetFloat(const char* key, float* out)
    {
        const Catalogue::Entry* e = Catalogue::Find(key);
        if (!e) return GBH_ERR_NOT_FOUND;
        return Read(*e, out) ? GBH_OK : GBH_ERR_STATE;
    }

    int Set(const char* key, const char* value)
    {
        const Catalogue::Entry* e = Catalogue::Find(key);
        if (!e) return GBH_ERR_NOT_FOUND;
        if (!e->writable) return GBH_ERR_UNSUPPORTED;

        // The old value first, since toggle flips it and an unreadable one means there is nothing to set.
        float old = 0.0f;
        if (!Read(*e, &old)) return GBH_ERR_STATE;

        Catalogue::Parsed p;
        if (!Catalogue::Parse(*e, value, old, &p, nullptr)) return GBH_ERR_ARG;
        if (!Write(*e, p)) return GBH_ERR;

        float now = 0.0f;
        if (!Read(*e, &now)) now = Requested(*e, p);
        char a[48], b[48];
        Catalogue::Format(*e, old, a, sizeof a);
        Catalogue::Format(*e, now, b, sizeof b);
        Log::Writef("ATTR", "%s %s -> %s", e->key, a, b);
        return GBH_OK;
    }

    void RegisterCommands()
    {
        Commands::Register(nullptr, "attr", CmdAttr, nullptr, "[<key> [<value>]] -- engine state: list, read or set one",
                           Commands::kGameThread);
    }
}
