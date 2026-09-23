// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Key chords as a player writes them, "CTRL+SHIFT+F5", over virtual-key codes. Pure; tests/input/test_chord.cpp is the spec.
// The window procedure delivers the generic VK for a modifier, so there are no left and right variants.

#include <string>

namespace Chord
{
    enum Mod : unsigned { kCtrl = 1u, kShift = 2u, kAlt = 4u };

    constexpr int kVkShift = 0x10, kVkCtrl = 0x11, kVkAlt = 0x12;

    // vk 0 is unbound.
    struct Key
    {
        int      vk   = 0;
        unsigned mods = 0;
        bool operator==(const Key& o) const { return vk == o.vk && mods == o.mods; }
    };

    enum class Parse { Ok, None, Bad };

    // Empty, blank or NONE is None. A modifier alone, two keys, an unknown name or a repeated modifier is Bad.
    Parse FromText(const char* text, Key* out);

    // Canonical text, modifiers first as CTRL+SHIFT+ALT. "" for an unbound key.
    std::string ToText(const Key& k);

    // Names in the spellings input/Dik uses, any case. -1 for an unknown name or a modifier.
    int         VkFromName(const char* name);
    const char* VkName(int vk);

    // The Mod bit a modifier's VK stands for, 0 for any other key.
    unsigned ModOfVk(int vk);
}
