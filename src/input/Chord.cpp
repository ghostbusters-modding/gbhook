// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Chord.h"

#include <cctype>
#include <cstring>

namespace
{
    struct Name { const char* name; int vk; };

    // The canonical name comes first for each code; aliases follow. NUMPADENTER is absent: its VK is ENTER's.
    const Name kNames[] = {
        { "ESCAPE", 0x1B }, { "ESC", 0x1B },
        { "0", 0x30 }, { "1", 0x31 }, { "2", 0x32 }, { "3", 0x33 }, { "4", 0x34 },
        { "5", 0x35 }, { "6", 0x36 }, { "7", 0x37 }, { "8", 0x38 }, { "9", 0x39 },
        { "A", 0x41 }, { "B", 0x42 }, { "C", 0x43 }, { "D", 0x44 }, { "E", 0x45 }, { "F", 0x46 }, { "G", 0x47 },
        { "H", 0x48 }, { "I", 0x49 }, { "J", 0x4A }, { "K", 0x4B }, { "L", 0x4C }, { "M", 0x4D }, { "N", 0x4E },
        { "O", 0x4F }, { "P", 0x50 }, { "Q", 0x51 }, { "R", 0x52 }, { "S", 0x53 }, { "T", 0x54 }, { "U", 0x55 },
        { "V", 0x56 }, { "W", 0x57 }, { "X", 0x58 }, { "Y", 0x59 }, { "Z", 0x5A },
        { "F1", 0x70 }, { "F2", 0x71 }, { "F3", 0x72 }, { "F4", 0x73 }, { "F5", 0x74 }, { "F6", 0x75 },
        { "F7", 0x76 }, { "F8", 0x77 }, { "F9", 0x78 }, { "F10", 0x79 }, { "F11", 0x7A }, { "F12", 0x7B },
        { "ENTER", 0x0D }, { "RETURN", 0x0D }, { "SPACE", 0x20 }, { "TAB", 0x09 },
        { "BACKSPACE", 0x08 }, { "BACK", 0x08 },
        { "HOME", 0x24 }, { "END", 0x23 }, { "PGUP", 0x21 }, { "PRIOR", 0x21 }, { "PGDN", 0x22 }, { "NEXT", 0x22 },
        { "INSERT", 0x2D }, { "INS", 0x2D }, { "DELETE", 0x2E }, { "DEL", 0x2E },
        { "UP", 0x26 }, { "DOWN", 0x28 }, { "LEFT", 0x25 }, { "RIGHT", 0x27 },
        { "NUMPAD0", 0x60 }, { "NUMPAD1", 0x61 }, { "NUMPAD2", 0x62 }, { "NUMPAD3", 0x63 }, { "NUMPAD4", 0x64 },
        { "NUMPAD5", 0x65 }, { "NUMPAD6", 0x66 }, { "NUMPAD7", 0x67 }, { "NUMPAD8", 0x68 }, { "NUMPAD9", 0x69 },
        { "MULTIPLY", 0x6A }, { "ADD", 0x6B }, { "SUBTRACT", 0x6D }, { "DECIMAL", 0x6E }, { "DIVIDE", 0x6F },
        { "PAUSE", 0x13 }, { "CAPSLOCK", 0x14 }, { "CAPITAL", 0x14 }, { "NUMLOCK", 0x90 }, { "SCROLL", 0x91 },
        { "SEMICOLON", 0xBA }, { "EQUALS", 0xBB }, { "COMMA", 0xBC }, { "MINUS", 0xBD }, { "PERIOD", 0xBE },
        { "SLASH", 0xBF }, { "GRAVE", 0xC0 }, { "LBRACKET", 0xDB }, { "BACKSLASH", 0xDC }, { "RBRACKET", 0xDD },
        { "APOSTROPHE", 0xDE },
    };

    // The first three are canonical. The sided spellings fold in, since the window procedure cannot tell sides apart.
    struct Modifier { const char* name; unsigned bit; };
    const Modifier kMods[] = {
        { "CTRL", KeyChord::kCtrl }, { "SHIFT", KeyChord::kShift }, { "ALT", KeyChord::kAlt },
        { "LCTRL", KeyChord::kCtrl }, { "RCTRL", KeyChord::kCtrl }, { "LCONTROL", KeyChord::kCtrl }, { "RCONTROL", KeyChord::kCtrl },
        { "LSHIFT", KeyChord::kShift }, { "RSHIFT", KeyChord::kShift },
        { "LALT", KeyChord::kAlt }, { "RALT", KeyChord::kAlt }, { "LMENU", KeyChord::kAlt }, { "RMENU", KeyChord::kAlt },
    };
    constexpr int kCanonicalMods = 3;

    bool IEquals(const char* a, const char* b)
    {
        for (;; ++a, ++b)
        {
            const int x = toupper((unsigned char)*a), y = toupper((unsigned char)*b);
            if (x != y) return false;
            if (!x) return true;
        }
    }

    std::string Trim(const std::string& s)
    {
        size_t b = 0, e = s.size();
        while (b < e && isspace((unsigned char)s[b])) ++b;
        while (e > b && isspace((unsigned char)s[e - 1])) --e;
        return s.substr(b, e - b);
    }

    unsigned ModFromName(const char* name)
    {
        for (const Modifier& m : kMods) if (IEquals(m.name, name)) return m.bit;
        return 0;
    }
}

namespace KeyChord
{
    int VkFromName(const char* name)
    {
        if (!name || !*name) return -1;
        for (const Name& n : kNames) if (IEquals(n.name, name)) return n.vk;
        return -1;
    }

    const char* VkName(int vk)
    {
        for (const Name& n : kNames) if (n.vk == vk) return n.name;
        return nullptr;
    }

    unsigned ModOfVk(int vk)
    {
        switch (vk)
        {
        case kVkCtrl:  return kCtrl;
        case kVkShift: return kShift;
        case kVkAlt:   return kAlt;
        default:       return 0;
        }
    }

    Parse FromText(const char* text, Key* out)
    {
        Key k;
        if (out) *out = k;
        const std::string whole = Trim(text ? text : "");
        if (whole.empty() || IEquals(whole.c_str(), "NONE")) return Parse::None;

        size_t start = 0;
        for (;;)
        {
            const size_t plus  = whole.find('+', start);
            const std::string tok = Trim(whole.substr(start, plus == std::string::npos ? std::string::npos : plus - start));
            if (tok.empty()) return Parse::Bad;
            if (const unsigned bit = ModFromName(tok.c_str()))
            {
                if (k.mods & bit) return Parse::Bad;
                k.mods |= bit;
            }
            else
            {
                const int vk = VkFromName(tok.c_str());
                if (vk < 0 || k.vk) return Parse::Bad;
                k.vk = vk;
            }
            if (plus == std::string::npos) break;
            start = plus + 1;
        }
        if (!k.vk) return Parse::Bad;
        if (out) *out = k;
        return Parse::Ok;
    }

    std::string ToText(const Key& k)
    {
        const char* name = VkName(k.vk);
        if (!k.vk || !name) return std::string();
        std::string s;
        for (int i = 0; i < kCanonicalMods; ++i)
            if (k.mods & kMods[i].bit) { s += kMods[i].name; s += '+'; }
        return s + name;
    }
}
