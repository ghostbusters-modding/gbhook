// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "PeFile.h"
#include "format/Pe.h"
#include "format/PeExport.h"

#include <windows.h>

// Our own PE structs against the system's: the walk is only as right as these layouts.
static_assert(sizeof(Pe::DosHeader)        == sizeof(IMAGE_DOS_HEADER));
static_assert(sizeof(Pe::FileHeader)       == sizeof(IMAGE_FILE_HEADER));
static_assert(sizeof(Pe::OptionalHeader64) == sizeof(IMAGE_OPTIONAL_HEADER64));
static_assert(sizeof(Pe::NtHeaders64)      == sizeof(IMAGE_NT_HEADERS64));
static_assert(sizeof(Pe::SectionHeader)    == sizeof(IMAGE_SECTION_HEADER));
static_assert(sizeof(Pe::ExportDirectory)  == sizeof(IMAGE_EXPORT_DIRECTORY));
static_assert(Pe::kOptionalHeaderOffset    == offsetof(IMAGE_NT_HEADERS64, OptionalHeader));

namespace
{
    // A read-only file mapping. No image mapping, no relocations, no entry point: nothing in the file runs.
    struct Mapping
    {
        HANDLE   file = INVALID_HANDLE_VALUE;
        HANDLE   map  = nullptr;
        uint8_t* base = nullptr;
        size_t   size = 0;

        ~Mapping()
        {
            if (base) UnmapViewOfFile(base);
            if (map)  CloseHandle(map);
            if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
        }

        bool Open(const char* path)
        {
            file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (file == INVALID_HANDLE_VALUE) return false;

            LARGE_INTEGER sz;
            if (!GetFileSizeEx(file, &sz) || sz.QuadPart <= 0 ||
                sz.QuadPart > (LONGLONG)64 * 1024 * 1024)
                return false;                      // a mod DLL is not 64 MB

            size = (size_t)sz.QuadPart;
            map  = CreateFileMappingW(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
            if (!map) return false;
            base = (uint8_t*)MapViewOfFile(map, FILE_MAP_READ, 0, 0, 0);
            return base != nullptr;
        }
    };
}

namespace PeFile
{
    bool ReadDataExport(const char* path, const char* name, void* out, size_t n, const char** why)
    {
        if (!why) return false;
        if (!path) { *why = "bad argument"; return false; }

        Mapping m;
        if (!m.Open(path)) { *why = "cannot open file"; return false; }

        return PeExport::ReadDataExport(m.base, m.size, name, out, n, why);
    }
}
