// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Named actions bound to chords: claims, clashes, key edges, held state and capture. Pure and unlocked.
// services/Bindings locks around it and owns the settings, the log and the fire ring. tests/input/test_bindtable.cpp is the spec.

#include "Chord.h"

namespace BindTable
{
    constexpr int kMaxClaims = 128;
    constexpr int kOwnerCap  = 64;
    constexpr int kNameCap   = 32;
    constexpr int kHelpCap   = 96;
    constexpr int kVkCount   = 256;

    struct Claim
    {
        char          owner[kOwnerCap];   // "" is the framework
        char          name[kNameCap];
        char          help[kHelpCap];
        KeyChord::Key key;                // vk 0: unbound
        void*         fn;
        void*         user;
        bool          enabled;
        bool          faulted;            // off for the process; enable cannot bring it back
    };

    enum class Add { Ok, BadName, Taken, Full };

    struct AddResult
    {
        Add status    = Add::Ok;
        int index     = -1;
        int clashWith = -1;   // Ok but unbound: the claim already holding the chord
    };

    struct Edge
    {
        bool consume = false;
        int  fired   = -1;    // the claim to fire, or -1
    };

    // Non-empty, no whitespace, at most kNameCap - 1 characters.
    bool ValidName(const char* name);

    class Table
    {
    public:
        Table();

        // `key` with vk 0 registers unbound. The first claim on a chord wins, whoever made the second.
        AddResult Add(const char* owner, const char* name, const char* help, KeyChord::Key key, void* fn, void* user);
        int       Find(const char* owner, const char* name) const;

        // The window procedure's edges. A repeat down fires nothing and is consumed only if the first down was.
        Edge Down(int vk);
        bool Up(int vk);
        void ReleaseAll();   // focus lost: no key-up will ever come for what is down

        bool Held(int i) const;
        bool Enable(int i, bool on);
        void Kill(int i);

        // False when another owner holds the capture, for on and off alike.
        bool Capture(const char* owner, bool on);
        bool CapturedBy(const char* owner) const;

        // Enabled, not faulted and not shut out by someone else's capture.
        bool Live(int i) const;

        int          Count() const { return count_; }
        const Claim* At(int i) const;

    private:
        unsigned HeldMods() const;

        Claim claims_[kMaxClaims];
        int   count_;
        bool  down_[kVkCount];
        bool  eaten_[kVkCount];
        bool  captured_;
        char  captor_[kOwnerCap];
    };
}
