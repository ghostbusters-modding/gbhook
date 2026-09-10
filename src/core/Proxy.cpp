// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// The dinput8 proxy: every real export forwarded. The exports are named in GbHook.def, which says why.

#include <windows.h>

namespace
{
    HMODULE g_real = nullptr;

    HMODULE RealDInput8()
    {
        if (g_real != nullptr)
            return g_real;

        // Absolute path into the system directory: a bare LoadLibraryW(L"dinput8.dll") finds this file and recurses.
        wchar_t path[MAX_PATH];
        UINT n = GetSystemDirectoryW(path, MAX_PATH);
        if (n == 0 || n >= MAX_PATH)
            return nullptr;

        const wchar_t* leaf = L"\\dinput8.dll";
        if (n + lstrlenW(leaf) >= MAX_PATH)
            return nullptr;
        lstrcatW(path, leaf);

        g_real = LoadLibraryW(path);
        if (g_real == nullptr)
        {
            MessageBoxA(NULL,
                        "gbhook: could not load the system dinput8.dll.\n"
                        "Delete dinput8.dll from the game folder to restore the game.",
                        "gbhook", MB_OK | MB_ICONERROR);
        }
        return g_real;
    }

    FARPROC Real(const char* name)
    {
        HMODULE m = RealDInput8();
        return m ? GetProcAddress(m, name) : nullptr;
    }
}

extern "C" {

HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, const GUID& riidltf,
                                  void** ppvOut, void* punkOuter)
{
    using Fn = HRESULT(WINAPI*)(HINSTANCE, DWORD, const GUID&, void**, void*);
    Fn fn = reinterpret_cast<Fn>(Real("DirectInput8Create"));
    if (!fn) return E_FAIL;
    return fn(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

HRESULT WINAPI DllCanUnloadNow(void)
{
    using Fn = HRESULT(WINAPI*)(void);
    Fn fn = reinterpret_cast<Fn>(Real("DllCanUnloadNow"));
    return fn ? fn() : S_FALSE;      // S_FALSE is "do not unload", the safe answer
}

HRESULT WINAPI DllGetClassObject(const GUID& rclsid, const GUID& riid, void** ppv)
{
    using Fn = HRESULT(WINAPI*)(const GUID&, const GUID&, void**);
    Fn fn = reinterpret_cast<Fn>(Real("DllGetClassObject"));
    return fn ? fn(rclsid, riid, ppv) : E_FAIL;
}

HRESULT WINAPI DllRegisterServer(void)
{
    using Fn = HRESULT(WINAPI*)(void);
    Fn fn = reinterpret_cast<Fn>(Real("DllRegisterServer"));
    return fn ? fn() : E_FAIL;
}

HRESULT WINAPI DllUnregisterServer(void)
{
    using Fn = HRESULT(WINAPI*)(void);
    Fn fn = reinterpret_cast<Fn>(Real("DllUnregisterServer"));
    return fn ? fn() : E_FAIL;
}

void* WINAPI GetdfDIJoystick(void)
{
    using Fn = void*(WINAPI*)(void);
    Fn fn = reinterpret_cast<Fn>(Real("GetdfDIJoystick"));
    return fn ? fn() : nullptr;
}

} // extern "C"
