// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The PE32+ structures the export walk reads, so no need for windows.h. Sizes are asserted.

#include <cstddef>
#include <cstdint>

namespace Pe
{
    constexpr uint16_t kDosMagic        = 0x5A4D;       // "MZ"
    constexpr uint32_t kNtSignature     = 0x00004550;   // "PE\0\0"
    constexpr uint16_t kMachineAmd64    = 0x8664;
    constexpr uint16_t kOptional64Magic = 0x20B;
    constexpr uint16_t kFileDll         = 0x2000;
    constexpr int      kDirExport       = 0;

    struct DosHeader
    {
        uint16_t e_magic, e_cblp, e_cp, e_crlc, e_cparhdr, e_minalloc, e_maxalloc, e_ss, e_sp;
        uint16_t e_csum, e_ip, e_cs, e_lfarlc, e_ovno, e_res[4], e_oemid, e_oeminfo, e_res2[10];
        int32_t  e_lfanew;
    };

    struct FileHeader
    {
        uint16_t Machine, NumberOfSections;
        uint32_t TimeDateStamp, PointerToSymbolTable, NumberOfSymbols;
        uint16_t SizeOfOptionalHeader, Characteristics;
    };

    struct DataDirectory { uint32_t VirtualAddress, Size; };

    struct OptionalHeader64
    {
        uint16_t Magic;
        uint8_t  MajorLinkerVersion, MinorLinkerVersion;
        uint32_t SizeOfCode, SizeOfInitializedData, SizeOfUninitializedData, AddressOfEntryPoint, BaseOfCode;
        uint64_t ImageBase;
        uint32_t SectionAlignment, FileAlignment;
        uint16_t MajorOperatingSystemVersion, MinorOperatingSystemVersion, MajorImageVersion, MinorImageVersion;
        uint16_t MajorSubsystemVersion, MinorSubsystemVersion;
        uint32_t Win32VersionValue, SizeOfImage, SizeOfHeaders, CheckSum;
        uint16_t Subsystem, DllCharacteristics;
        uint64_t SizeOfStackReserve, SizeOfStackCommit, SizeOfHeapReserve, SizeOfHeapCommit;
        uint32_t LoaderFlags, NumberOfRvaAndSizes;
        Pe::DataDirectory DataDirectory[16];
    };

    struct NtHeaders64
    {
        uint32_t         Signature;
        Pe::FileHeader   FileHeader;
        OptionalHeader64 OptionalHeader;
    };

    struct SectionHeader
    {
        uint8_t  Name[8];
        uint32_t VirtualSize, VirtualAddress, SizeOfRawData, PointerToRawData, PointerToRelocations, PointerToLinenumbers;
        uint16_t NumberOfRelocations, NumberOfLinenumbers;
        uint32_t Characteristics;
    };

    struct ExportDirectory
    {
        uint32_t Characteristics, TimeDateStamp;
        uint16_t MajorVersion, MinorVersion;
        uint32_t Name, Base, NumberOfFunctions, NumberOfNames, AddressOfFunctions, AddressOfNames, AddressOfNameOrdinals;
    };

    // The section table follows the optional header, whose size is declared rather than fixed.
    constexpr size_t kOptionalHeaderOffset = offsetof(NtHeaders64, OptionalHeader);

    static_assert(sizeof(DosHeader) == 64);
    static_assert(sizeof(FileHeader) == 20);
    static_assert(sizeof(OptionalHeader64) == 240);
    static_assert(sizeof(NtHeaders64) == 264);
    static_assert(sizeof(SectionHeader) == 40);
    static_assert(sizeof(ExportDirectory) == 40);
    static_assert(kOptionalHeaderOffset == 24);
}
