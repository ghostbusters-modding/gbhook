// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Dik.h"

#include <cctype>
#include <cstring>

namespace
{
    struct Key { const char* name; int dik; };

    // The canonical name comes first for each code; aliases follow.
    const Key kKeys[] = {
        { "ESCAPE", 0x01 }, { "ESC", 0x01 },
        { "1", 0x02 }, { "2", 0x03 }, { "3", 0x04 }, { "4", 0x05 }, { "5", 0x06 },
        { "6", 0x07 }, { "7", 0x08 }, { "8", 0x09 }, { "9", 0x0A }, { "0", 0x0B },
        { "MINUS", 0x0C }, { "EQUALS", 0x0D }, { "BACKSPACE", 0x0E }, { "BACK", 0x0E }, { "TAB", 0x0F },
        { "Q", 0x10 }, { "W", 0x11 }, { "E", 0x12 }, { "R", 0x13 }, { "T", 0x14 }, { "Y", 0x15 },
        { "U", 0x16 }, { "I", 0x17 }, { "O", 0x18 }, { "P", 0x19 }, { "LBRACKET", 0x1A }, { "RBRACKET", 0x1B },
        { "ENTER", 0x1C }, { "RETURN", 0x1C }, { "LCONTROL", 0x1D }, { "CTRL", 0x1D }, { "LCTRL", 0x1D },
        { "A", 0x1E }, { "S", 0x1F }, { "D", 0x20 }, { "F", 0x21 }, { "G", 0x22 }, { "H", 0x23 },
        { "J", 0x24 }, { "K", 0x25 }, { "L", 0x26 }, { "SEMICOLON", 0x27 }, { "APOSTROPHE", 0x28 }, { "GRAVE", 0x29 },
        { "LSHIFT", 0x2A }, { "SHIFT", 0x2A }, { "BACKSLASH", 0x2B },
        { "Z", 0x2C }, { "X", 0x2D }, { "C", 0x2E }, { "V", 0x2F }, { "B", 0x30 }, { "N", 0x31 }, { "M", 0x32 },
        { "COMMA", 0x33 }, { "PERIOD", 0x34 }, { "SLASH", 0x35 }, { "RSHIFT", 0x36 }, { "MULTIPLY", 0x37 },
        { "LALT", 0x38 }, { "ALT", 0x38 }, { "LMENU", 0x38 }, { "SPACE", 0x39 }, { "CAPSLOCK", 0x3A }, { "CAPITAL", 0x3A },
        { "F1", 0x3B }, { "F2", 0x3C }, { "F3", 0x3D }, { "F4", 0x3E }, { "F5", 0x3F },
        { "F6", 0x40 }, { "F7", 0x41 }, { "F8", 0x42 }, { "F9", 0x43 }, { "F10", 0x44 },
        { "NUMLOCK", 0x45 }, { "SCROLL", 0x46 },
        { "NUMPAD7", 0x47 }, { "NUMPAD8", 0x48 }, { "NUMPAD9", 0x49 }, { "SUBTRACT", 0x4A },
        { "NUMPAD4", 0x4B }, { "NUMPAD5", 0x4C }, { "NUMPAD6", 0x4D }, { "ADD", 0x4E },
        { "NUMPAD1", 0x4F }, { "NUMPAD2", 0x50 }, { "NUMPAD3", 0x51 }, { "NUMPAD0", 0x52 }, { "DECIMAL", 0x53 },
        { "F11", 0x57 }, { "F12", 0x58 },
        { "NUMPADENTER", 0x9C }, { "RCONTROL", 0x9D }, { "RCTRL", 0x9D }, { "DIVIDE", 0xB5 },
        { "RALT", 0xB8 }, { "RMENU", 0xB8 }, { "PAUSE", 0xC5 },
        { "HOME", 0xC7 }, { "UP", 0xC8 }, { "PGUP", 0xC9 }, { "PRIOR", 0xC9 },
        { "LEFT", 0xCB }, { "RIGHT", 0xCD }, { "END", 0xCF }, { "DOWN", 0xD0 }, { "PGDN", 0xD1 }, { "NEXT", 0xD1 },
        { "INSERT", 0xD2 }, { "INS", 0xD2 }, { "DELETE", 0xD3 }, { "DEL", 0xD3 },
    };

    bool IEquals(const char* a, const char* b)
    {
        for (;; ++a, ++b)
        {
            const int x = toupper((unsigned char)*a), y = toupper((unsigned char)*b);
            if (x != y) return false;
            if (!x) return true;
        }
    }
}

namespace Dik
{
    int FromName(const char* name)
    {
        if (!name || !*name) return -1;
        for (const Key& k : kKeys) if (IEquals(k.name, name)) return k.dik;
        return -1;
    }

    const char* Name(int dik)
    {
        for (const Key& k : kKeys) if (k.dik == dik) return k.name;
        return nullptr;
    }

    int TableIndex(int dik, unsigned mask)
    {
        if (dik < 0 || dik > 0xFF) return -1;
        const int code = (dik & 0x7F) | ((dik & 0x80) ? 0x100 : 0);
        const int idx  = code & (int)mask;
        return idx < kTableEntries ? idx : -1;
    }
}
