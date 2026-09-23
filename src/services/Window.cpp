// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// The engine fills its scan-code table from its own WM_KEYDOWN handler, so answering 0 here really does hide a key.
// Never un-subclassed: nothing in gbhook is ever unloaded, and the window outlives every mod.
#include "Window.h"
#include "Bindings.h"
#include "Events.h"
#include "Pump.h"
#include "../core/Framework.h"

#include <windows.h>

namespace
{
    HWND    g_hwnd      = nullptr;
    WNDPROC g_original  = nullptr;
    bool    g_unicode   = true;
    bool    g_installed = false;
    bool    g_waited    = false;

    LRESULT CALLBACK SubclassProc(HWND h, UINT msg, WPARAM w, LPARAM l)
    {
        switch (msg)
        {
        // Raw subscribers first: a text line that keeps typed keys also keeps them from firing actions.
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (Events::FireKey((int)w, true) || Bindings::OnKey((int)w, true)) return 0;
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (Events::FireKey((int)w, false) || Bindings::OnKey((int)w, false)) return 0;
            break;
        // No key-up ever comes for a key released while the window is in the background.
        case WM_KILLFOCUS:
            Bindings::OnFocusLost();
            break;
        case WM_ACTIVATE:
            if (LOWORD(w) == WA_INACTIVE) Bindings::OnFocusLost();
            break;
        case WM_CHAR:
        case WM_SYSCHAR:
            if (Events::FireChar((unsigned int)w)) return 0;
            break;
        default:
            break;
        }
        return g_unicode ? CallWindowProcW(g_original, h, msg, w, l) : CallWindowProcA(g_original, h, msg, w, l);
    }

    // The first visible top-level window of the calling thread; the pump runs this on the main thread that made it.
    BOOL CALLBACK PickTopLevel(HWND h, LPARAM out)
    {
        if (!IsWindowVisible(h) || GetParent(h) || GetWindow(h, GW_OWNER)) return TRUE;
        *reinterpret_cast<HWND*>(out) = h;
        return FALSE;
    }

    bool Subclass(void*)
    {
        HWND h = nullptr;
        EnumThreadWindows(GetCurrentThreadId(), PickTopLevel, reinterpret_cast<LPARAM>(&h));
        if (!h)
        {
            if (!g_waited) Log::Write("WND", "no top-level window on the main thread yet; the pump will retry");
            g_waited = true;
            return false;
        }

        // Subclassing an ANSI window through the W entry flips its WM_CHAR translation; match what the class was.
        g_unicode = IsWindowUnicode(h) != FALSE;
        WNDPROC prev = g_unicode
            ? reinterpret_cast<WNDPROC>(SetWindowLongPtrW(h, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&SubclassProc)))
            : reinterpret_cast<WNDPROC>(SetWindowLongPtrA(h, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&SubclassProc)));
        if (!prev)
        {
            Log::Writef("WND", "subclass FAILED (error %lu); keys will not fan out", (unsigned long)GetLastError());
            return true;
        }
        g_original  = prev;
        g_hwnd      = h;
        g_installed = true;

        char title[128] = { 0 }, cls[64] = { 0 };
        GetWindowTextA(h, title, sizeof title);
        GetClassNameA(h, cls, sizeof cls);
        Log::Writef("WND", "subclassed '%s' (%s)", title, cls);
        return true;
    }
}

namespace Window
{
    void Install()
    {
        if (g_installed) return;
        Pump::Park(Subclass, nullptr, "window subclass");
    }

    bool  Installed() { return g_installed; }
    void* Handle()    { return g_hwnd; }
}
