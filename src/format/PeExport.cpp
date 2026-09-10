// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "PeExport.h"
#include "Pe.h"

#include <cstring>

namespace
{
    // The file bytes plus the bounds every access is checked against.
    struct View
    {
        const uint8_t* base;
        size_t         size;

        template <typename T>
        const T* At(size_t off, size_t count = 1) const
        {
            if (off > size) return nullptr;
            if (count > (size - off) / sizeof(T)) return nullptr;
            return reinterpret_cast<const T*>(base + off);
        }

        bool InBounds(size_t off, size_t len) const
        {
            return off <= size && len <= size - off;
        }
    };

    struct Headers
    {
        const Pe::NtHeaders64*   nt       = nullptr;
        const Pe::SectionHeader* sections = nullptr;
        uint16_t                 count    = 0;
    };

    bool ReadHeaders(const View& m, Headers& h, const char** why)
    {
        const Pe::DosHeader* dos = m.At<Pe::DosHeader>(0);
        if (!dos || dos->e_magic != Pe::kDosMagic)
        { *why = "not a PE file (bad DOS header)"; return false; }

        if (dos->e_lfanew < 0) { *why = "bad e_lfanew"; return false; }
        const size_t ntOff = (size_t)dos->e_lfanew;

        const Pe::NtHeaders64* nt = m.At<Pe::NtHeaders64>(ntOff);
        if (!nt || nt->Signature != Pe::kNtSignature)
        { *why = "not a PE file (bad NT header)"; return false; }

        if (nt->FileHeader.Machine != Pe::kMachineAmd64)
        { *why = "not x64 -- gbhook is 64-bit only"; return false; }

        if (nt->OptionalHeader.Magic != Pe::kOptional64Magic)
        { *why = "not a PE32+ image"; return false; }

        if (!(nt->FileHeader.Characteristics & Pe::kFileDll))
        { *why = "not a DLL"; return false; }

        const size_t   secOff = ntOff + Pe::kOptionalHeaderOffset + nt->FileHeader.SizeOfOptionalHeader;
        const uint16_t n      = nt->FileHeader.NumberOfSections;
        if (n == 0 || n > 96) { *why = "implausible section count"; return false; }

        const Pe::SectionHeader* sec = m.At<Pe::SectionHeader>(secOff, n);
        if (!sec) { *why = "section table out of bounds"; return false; }

        h.nt = nt; h.sections = sec; h.count = n;
        return true;
    }

    // RVA to file offset through the section table. SIZE_MAX when no section's on-disk bytes hold it.
    size_t RvaToOffset(const Headers& h, uint32_t rva)
    {
        for (uint16_t i = 0; i < h.count; ++i)
        {
            const Pe::SectionHeader& s = h.sections[i];
            // The on-disk size: a bigger VirtualSize is BSS, which has no file bytes to read.
            const uint32_t len = s.SizeOfRawData;
            if (len == 0) continue;
            if (rva >= s.VirtualAddress && rva < s.VirtualAddress + len)
                return (size_t)s.PointerToRawData + (rva - s.VirtualAddress);
        }
        return SIZE_MAX;
    }
}

namespace PeExport
{
    bool ReadDataExport(const uint8_t* file, size_t size, const char* name, void* out, size_t n, const char** why)
    {
        if (!why) return false;
        auto fail = [&](const char* w) { *why = w; return false; };

        if (!file || size == 0 || !name || !out || n == 0) return fail("bad argument");

        const View m{ file, size };
        Headers h;
        if (!ReadHeaders(m, h, why)) return false;

        const Pe::DataDirectory& dir = h.nt->OptionalHeader.DataDirectory[Pe::kDirExport];
        if (dir.VirtualAddress == 0 || dir.Size == 0)
            return fail("no export directory -- did you forget GBHOOK_PLUGIN()?");

        const size_t expOff = RvaToOffset(h, dir.VirtualAddress);
        if (expOff == SIZE_MAX) return fail("export directory RVA not in any section");

        const Pe::ExportDirectory* exp = m.At<Pe::ExportDirectory>(expOff);
        if (!exp) return fail("export directory out of bounds");

        if (exp->NumberOfNames == 0 || exp->NumberOfNames > 65535)
            return fail("no named exports");

        const size_t namesOff = RvaToOffset(h, exp->AddressOfNames);
        const size_t ordsOff  = RvaToOffset(h, exp->AddressOfNameOrdinals);
        const size_t funcsOff = RvaToOffset(h, exp->AddressOfFunctions);
        if (namesOff == SIZE_MAX || ordsOff == SIZE_MAX || funcsOff == SIZE_MAX)
            return fail("export tables not in any section");

        const uint32_t* names = m.At<uint32_t>(namesOff, exp->NumberOfNames);
        const uint16_t* ords  = m.At<uint16_t>(ordsOff,  exp->NumberOfNames);
        const uint32_t* funcs = m.At<uint32_t>(funcsOff, exp->NumberOfFunctions);
        if (!names || !ords || !funcs) return fail("export tables out of bounds");

        for (uint32_t i = 0; i < exp->NumberOfNames; ++i)
        {
            const size_t nameOff = RvaToOffset(h, names[i]);
            if (nameOff == SIZE_MAX || nameOff >= m.size) continue;

            // Bound the comparison by what is mapped; an unterminated name is ignored.
            const char*  s   = (const char*)(m.base + nameOff);
            const size_t max = m.size - nameOff;
            size_t len = 0;
            while (len < max && s[len] != '\0') ++len;
            if (len >= max) continue;
            if (strcmp(s, name) != 0) continue;

            const uint16_t ord = ords[i];
            if (ord >= exp->NumberOfFunctions) return fail("bad name ordinal");

            const uint32_t rva = funcs[ord];

            // A forwarder RVA points back inside the export directory; following it would mean another module.
            if (rva >= dir.VirtualAddress && rva < dir.VirtualAddress + dir.Size)
                return fail("manifest export is a forwarder");

            const size_t dataOff = RvaToOffset(h, rva);
            if (dataOff == SIZE_MAX) return fail("manifest RVA not in any section");
            if (!m.InBounds(dataOff, n)) return fail("manifest extends past end of file");

            memcpy(out, m.base + dataOff, n);
            *why = nullptr;
            return true;
        }

        return fail("no 'GbhPluginManifest' export -- add GBHOOK_PLUGIN(...)");
    }
}
