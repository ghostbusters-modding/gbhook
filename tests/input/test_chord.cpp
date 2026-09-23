// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for input/Chord: names to virtual keys, chord text in and out, and every way a line can be wrong.

#include "check.h"
#include "input/Chord.h"

#include <string>

namespace
{
    Chord::Parse P(const char* text, Chord::Key* k = nullptr)
    {
        Chord::Key tmp;
        return Chord::FromText(text, k ? k : &tmp);
    }
    std::string Canon(const char* text)
    {
        Chord::Key k;
        return Chord::FromText(text, &k) == Chord::Parse::Ok ? Chord::ToText(k) : std::string("<bad>");
    }
}

int main()
{
    using Chord::Parse;

    // Names, any case, aliases included.
    CHECK_EQ(Chord::VkFromName("A"), 0x41);
    CHECK_EQ(Chord::VkFromName("z"), 0x5A);
    CHECK_EQ(Chord::VkFromName("0"), 0x30);
    CHECK_EQ(Chord::VkFromName("F1"), 0x70);
    CHECK_EQ(Chord::VkFromName("f12"), 0x7B);
    CHECK_EQ(Chord::VkFromName("ESC"), 0x1B);
    CHECK_EQ(Chord::VkFromName("Return"), 0x0D);
    CHECK_EQ(Chord::VkFromName("PGDN"), 0x22);
    CHECK_EQ(Chord::VkFromName("NUMPAD7"), 0x67);
    CHECK_EQ(Chord::VkFromName("GRAVE"), 0xC0);
    CHECK_EQ(Chord::VkFromName("CTRL"), -1);
    CHECK_EQ(Chord::VkFromName("BANANA"), -1);
    CHECK_EQ(Chord::VkFromName(""), -1);
    CHECK_EQ(Chord::VkFromName(nullptr), -1);
    CHECK_EQ(std::string(Chord::VkName(0x1B)), "ESCAPE");
    CHECK(Chord::VkName(0x10) == nullptr);

    // Every name resolves back to itself.
    for (int vk = 0; vk < 256; ++vk)
        if (const char* n = Chord::VkName(vk)) CHECK_EQ(Chord::VkFromName(n), vk);

    CHECK_EQ(Chord::ModOfVk(0x11), (unsigned)Chord::kCtrl);
    CHECK_EQ(Chord::ModOfVk(0x10), (unsigned)Chord::kShift);
    CHECK_EQ(Chord::ModOfVk(0x12), (unsigned)Chord::kAlt);
    CHECK_EQ(Chord::ModOfVk(0x41), 0u);

    // One key and chords.
    Chord::Key k;
    CHECK(P("F5", &k) == Parse::Ok);
    CHECK_EQ(k.vk, 0x74);
    CHECK_EQ(k.mods, 0u);
    CHECK(P("CTRL+SHIFT+F5", &k) == Parse::Ok);
    CHECK_EQ(k.vk, 0x74);
    CHECK_EQ(k.mods, (unsigned)(Chord::kCtrl | Chord::kShift));
    CHECK(P("alt+g", &k) == Parse::Ok);
    CHECK_EQ(k.mods, (unsigned)Chord::kAlt);

    // Every modifier order formats to one canonical text.
    const char* orders[] = { "CTRL+SHIFT+ALT+F5", "CTRL+ALT+SHIFT+F5", "SHIFT+CTRL+ALT+F5", "SHIFT+ALT+CTRL+F5",
                             "ALT+CTRL+SHIFT+F5", "ALT+SHIFT+CTRL+F5", "F5+ALT+SHIFT+CTRL", " alt + f5 + ctrl + shift " };
    for (const char* o : orders) CHECK_EQ(Canon(o), "CTRL+SHIFT+ALT+F5");
    CHECK_EQ(Canon("shift+ctrl+g"), "CTRL+SHIFT+G");
    CHECK_EQ(Canon("esc"), "ESCAPE");
    CHECK_EQ(Canon("ctrl + return"), "CTRL+ENTER");

    // Round trips: the canonical text parses back to the same key.
    for (int vk = 0; vk < 256; ++vk)
    {
        if (!Chord::VkName(vk)) continue;
        for (unsigned m = 0; m < 8; ++m)
        {
            Chord::Key in;
            in.vk = vk;
            in.mods = m;
            Chord::Key out;
            CHECK(Chord::FromText(Chord::ToText(in).c_str(), &out) == Parse::Ok && out == in);
        }
    }

    // Unbound on purpose.
    CHECK(P("") == Parse::None);
    CHECK(P("   ") == Parse::None);
    CHECK(P(nullptr) == Parse::None);
    CHECK(P("NONE") == Parse::None);
    CHECK(P("none") == Parse::None);
    CHECK_EQ(Chord::ToText(Chord::Key()), "");

    // Bad text leaves the key unbound.
    k.vk = 0x41;
    CHECK(P("CTRL+", &k) == Parse::Bad);
    CHECK_EQ(k.vk, 0);
    CHECK(P("+F5") == Parse::Bad);
    CHECK(P("CTRL++F5") == Parse::Bad);
    CHECK(P("CTRL") == Parse::Bad);
    CHECK(P("CTRL+SHIFT") == Parse::Bad);
    CHECK(P("F5+F6") == Parse::Bad);
    CHECK(P("CTRL+CTRL+F5") == Parse::Bad);
    CHECK(P("CTRL+BANANA") == Parse::Bad);
    CHECK(P("LCTRL+F5") == Parse::Bad);
    CHECK(P("NONE+F5") == Parse::Bad);
    CHECK(P("F 5") == Parse::Bad);

    return check::Done("chord");
}
