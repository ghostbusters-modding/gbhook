// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for input/Chord: names to virtual keys, chord text in and out, and every way a line can be wrong.

#include "check.h"
#include "input/Chord.h"

#include <string>

namespace
{
    KeyChord::Parse P(const char* text, KeyChord::Key* k = nullptr)
    {
        KeyChord::Key tmp;
        return KeyChord::FromText(text, k ? k : &tmp);
    }
    std::string Canon(const char* text)
    {
        KeyChord::Key k;
        return KeyChord::FromText(text, &k) == KeyChord::Parse::Ok ? KeyChord::ToText(k) : std::string("<bad>");
    }
}

int main()
{
    using KeyChord::Parse;

    // Names, any case, aliases included.
    CHECK_EQ(KeyChord::VkFromName("A"), 0x41);
    CHECK_EQ(KeyChord::VkFromName("z"), 0x5A);
    CHECK_EQ(KeyChord::VkFromName("0"), 0x30);
    CHECK_EQ(KeyChord::VkFromName("F1"), 0x70);
    CHECK_EQ(KeyChord::VkFromName("f12"), 0x7B);
    CHECK_EQ(KeyChord::VkFromName("ESC"), 0x1B);
    CHECK_EQ(KeyChord::VkFromName("Return"), 0x0D);
    CHECK_EQ(KeyChord::VkFromName("PGDN"), 0x22);
    CHECK_EQ(KeyChord::VkFromName("NUMPAD7"), 0x67);
    CHECK_EQ(KeyChord::VkFromName("GRAVE"), 0xC0);
    CHECK_EQ(KeyChord::VkFromName("CTRL"), -1);
    CHECK_EQ(KeyChord::VkFromName("BANANA"), -1);
    CHECK_EQ(KeyChord::VkFromName(""), -1);
    CHECK_EQ(KeyChord::VkFromName(nullptr), -1);
    CHECK_EQ(std::string(KeyChord::VkName(0x1B)), "ESCAPE");
    CHECK(KeyChord::VkName(0x10) == nullptr);

    // Every name resolves back to itself.
    for (int vk = 0; vk < 256; ++vk)
        if (const char* n = KeyChord::VkName(vk)) CHECK_EQ(KeyChord::VkFromName(n), vk);

    CHECK_EQ(KeyChord::ModOfVk(0x11), (unsigned)KeyChord::kCtrl);
    CHECK_EQ(KeyChord::ModOfVk(0x10), (unsigned)KeyChord::kShift);
    CHECK_EQ(KeyChord::ModOfVk(0x12), (unsigned)KeyChord::kAlt);
    CHECK_EQ(KeyChord::ModOfVk(0x41), 0u);

    // One key and chords.
    KeyChord::Key k;
    CHECK(P("F5", &k) == Parse::Ok);
    CHECK_EQ(k.vk, 0x74);
    CHECK_EQ(k.mods, 0u);
    CHECK(P("CTRL+SHIFT+F5", &k) == Parse::Ok);
    CHECK_EQ(k.vk, 0x74);
    CHECK_EQ(k.mods, (unsigned)(KeyChord::kCtrl | KeyChord::kShift));
    CHECK(P("alt+g", &k) == Parse::Ok);
    CHECK_EQ(k.mods, (unsigned)KeyChord::kAlt);

    // Every modifier order formats to one canonical text.
    const char* orders[] = { "CTRL+SHIFT+ALT+F5", "CTRL+ALT+SHIFT+F5", "SHIFT+CTRL+ALT+F5", "SHIFT+ALT+CTRL+F5",
                             "ALT+CTRL+SHIFT+F5", "ALT+SHIFT+CTRL+F5", "F5+ALT+SHIFT+CTRL", " alt + f5 + ctrl + shift " };
    for (const char* o : orders) CHECK_EQ(Canon(o), "CTRL+SHIFT+ALT+F5");
    CHECK_EQ(Canon("shift+ctrl+g"), "CTRL+SHIFT+G");
    CHECK_EQ(Canon("esc"), "ESCAPE");
    CHECK_EQ(Canon("ctrl + return"), "CTRL+ENTER");

    // Sided modifier names fold into the generic one.
    CHECK_EQ(Canon("LCTRL+F5"), "CTRL+F5");
    CHECK_EQ(Canon("rctrl+f5"), "CTRL+F5");
    CHECK_EQ(Canon("LCONTROL+F5"), "CTRL+F5");
    CHECK_EQ(Canon("RCONTROL+F5"), "CTRL+F5");
    CHECK_EQ(Canon("LSHIFT+F5"), "SHIFT+F5");
    CHECK_EQ(Canon("RSHIFT+F5"), "SHIFT+F5");
    CHECK_EQ(Canon("LALT+F5"), "ALT+F5");
    CHECK_EQ(Canon("RALT+F5"), "ALT+F5");
    CHECK_EQ(Canon("LMENU+F5"), "ALT+F5");
    CHECK_EQ(Canon("RMENU+F5"), "ALT+F5");
    CHECK_EQ(Canon("rshift+lctrl+ralt+g"), "CTRL+SHIFT+ALT+G");

    // Round trips: the canonical text parses back to the same key.
    for (int vk = 0; vk < 256; ++vk)
    {
        if (!KeyChord::VkName(vk)) continue;
        for (unsigned m = 0; m < 8; ++m)
        {
            KeyChord::Key in;
            in.vk = vk;
            in.mods = m;
            KeyChord::Key out;
            CHECK(KeyChord::FromText(KeyChord::ToText(in).c_str(), &out) == Parse::Ok && out == in);
        }
    }

    // Unbound on purpose.
    CHECK(P("") == Parse::None);
    CHECK(P("   ") == Parse::None);
    CHECK(P(nullptr) == Parse::None);
    CHECK(P("NONE") == Parse::None);
    CHECK(P("none") == Parse::None);
    CHECK_EQ(KeyChord::ToText(KeyChord::Key()), "");

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
    CHECK(P("LCTRL+CTRL+F5") == Parse::Bad);
    CHECK(P("LSHIFT+RSHIFT+F5") == Parse::Bad);
    CHECK(P("LALT") == Parse::Bad);
    CHECK(P("NONE+F5") == Parse::Bad);
    CHECK(P("F 5") == Parse::Bad);

    return check::Done("chord");
}
