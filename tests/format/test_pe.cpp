// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for format/PeExport: a real mingw-built DLL, then every way to break it, each with its refusal.

#include "check.h"
#include "format/Pe.h"
#include "format/PeExport.h"
#include "gbhook/gbhook.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{
    using Bytes = std::vector<uint8_t>;

    Bytes ReadFile(const char* path)
    {
        Bytes b;
        FILE* f = fopen(path, "rb");
        if (!f) return b;
        uint8_t buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof buf, f)) > 0) b.insert(b.end(), buf, buf + n);
        fclose(f);
        return b;
    }

    template <typename T> T Get(const Bytes& f, size_t off) { T v; memcpy(&v, &f[off], sizeof v); return v; }
    template <typename T> void Put(Bytes& f, size_t off, T v) { memcpy(&f[off], &v, sizeof v); }

    // Where everything the walk touches sits in the file, found independently so the suite can aim its damage.
    struct Loc
    {
        size_t   ntOff = 0, secOff = 0, expOff = 0, namesOff = 0, ordsOff = 0, funcsOff = 0;
        size_t   nameStrOff = 0, ordEntryOff = 0, funcEntryOff = 0, manifestOff = 0;
        uint32_t expRva = 0;
        uint16_t nSections = 0;
        uint32_t nNames = 0;
        size_t   lastRawEnd = 0;   // end of the last section's bytes
        uint32_t lastRva = 0;      // the RVA of that byte
    };

    size_t RvaToOff(const Bytes& f, const Loc& l, uint32_t rva)
    {
        for (uint16_t i = 0; i < l.nSections; ++i)
        {
            Pe::SectionHeader s = Get<Pe::SectionHeader>(f, l.secOff + i * sizeof(Pe::SectionHeader));
            if (s.SizeOfRawData && rva >= s.VirtualAddress && rva < s.VirtualAddress + s.SizeOfRawData)
                return s.PointerToRawData + (rva - s.VirtualAddress);
        }
        return SIZE_MAX;
    }

    Loc Locate(const Bytes& f)
    {
        Loc l;
        l.ntOff = (size_t)Get<Pe::DosHeader>(f, 0).e_lfanew;
        Pe::NtHeaders64 nt = Get<Pe::NtHeaders64>(f, l.ntOff);
        l.secOff    = l.ntOff + Pe::kOptionalHeaderOffset + nt.FileHeader.SizeOfOptionalHeader;
        l.nSections = nt.FileHeader.NumberOfSections;
        l.expRva    = nt.OptionalHeader.DataDirectory[Pe::kDirExport].VirtualAddress;
        l.expOff    = RvaToOff(f, l, l.expRva);
        Pe::ExportDirectory e = Get<Pe::ExportDirectory>(f, l.expOff);
        l.nNames   = e.NumberOfNames;
        l.namesOff = RvaToOff(f, l, e.AddressOfNames);
        l.ordsOff  = RvaToOff(f, l, e.AddressOfNameOrdinals);
        l.funcsOff = RvaToOff(f, l, e.AddressOfFunctions);
        for (uint32_t i = 0; i < l.nNames; ++i)
        {
            size_t so = RvaToOff(f, l, Get<uint32_t>(f, l.namesOff + 4 * i));
            if (strcmp((const char*)&f[so], GBH_EXPORT_MANIFEST) != 0) continue;
            l.nameStrOff   = so;
            l.ordEntryOff  = l.ordsOff + 2 * i;
            l.funcEntryOff = l.funcsOff + 4 * Get<uint16_t>(f, l.ordEntryOff);
            l.manifestOff  = RvaToOff(f, l, Get<uint32_t>(f, l.funcEntryOff));
        }
        for (uint16_t i = 0; i < l.nSections; ++i)
        {
            Pe::SectionHeader s = Get<Pe::SectionHeader>(f, l.secOff + i * sizeof(Pe::SectionHeader));
            size_t end = (size_t)s.PointerToRawData + s.SizeOfRawData;
            if (s.SizeOfRawData && end > l.lastRawEnd) { l.lastRawEnd = end; l.lastRva = s.VirtualAddress + s.SizeOfRawData; }
        }
        return l;
    }

    // "" on success, else the refusal.
    std::string Why(const Bytes& f, const char* name = GBH_EXPORT_MANIFEST)
    {
        GbhManifest m;
        const char* why = "unset";
        bool ok = PeExport::ReadDataExport(f.data(), f.size(), name, &m, sizeof m, &why);
        if (ok) { CHECK(why == nullptr); return ""; }
        return why ? why : "(null why)";
    }
}

int main()
{
    Bytes file = ReadFile("bin/format/fixture_mod.dll");
    CHECK(!file.empty());
    if (file.empty()) return check::Done("pe");

    // The good DLL: the manifest comes out intact.
    {
        GbhManifest m;
        memset(&m, 0xCC, sizeof m);
        const char* why = "unset";
        CHECK(PeExport::ReadDataExport(file.data(), file.size(), GBH_EXPORT_MANIFEST, &m, sizeof m, &why));
        CHECK(why == nullptr);
        CHECK_EQ(std::string(m.magic), GBH_MANIFEST_MAGIC);
        CHECK_EQ(m.struct_size, (uint32_t)sizeof(GbhManifest));
        CHECK_EQ(m.abi_version, (uint32_t)GBHOOK_ABI_VERSION);
        CHECK_EQ(std::string(m.id), "gb.fixture");
        CHECK_EQ(std::string(m.target_md5), GBHOOK_TARGET_MD5);
        CHECK_EQ(m.exclusive_hooks[0][0], '\0');
    }

    Loc l = Locate(file);
    CHECK(l.manifestOff != 0 && l.manifestOff != SIZE_MAX);
    CHECK_EQ(Get<uint16_t>(file, l.ntOff + Pe::kOptionalHeaderOffset), (uint16_t)Pe::kOptional64Magic);

    // Arguments.
    {
        GbhManifest m;
        const char* why = nullptr;
        CHECK(!PeExport::ReadDataExport(nullptr, 0, GBH_EXPORT_MANIFEST, &m, sizeof m, &why));
        CHECK_EQ(std::string(why), "bad argument");
        CHECK(!PeExport::ReadDataExport(file.data(), file.size(), GBH_EXPORT_MANIFEST, nullptr, sizeof m, &why));
        CHECK(!PeExport::ReadDataExport(file.data(), file.size(), GBH_EXPORT_MANIFEST, &m, 0, &why));
        CHECK(!PeExport::ReadDataExport(file.data(), file.size(), GBH_EXPORT_MANIFEST, &m, sizeof m, nullptr));
    }

    // Headers.
    { Bytes f = file; Put<uint16_t>(f, 0, 0x5858);
      CHECK_EQ(Why(f), "not a PE file (bad DOS header)"); }
    { Bytes f = file; Put<int32_t>(f, offsetof(Pe::DosHeader, e_lfanew), -1);
      CHECK_EQ(Why(f), "bad e_lfanew"); }
    { Bytes f = file; Put<int32_t>(f, offsetof(Pe::DosHeader, e_lfanew), 0x7FFFFFF0);
      CHECK_EQ(Why(f), "not a PE file (bad NT header)"); }
    { Bytes f = file; f.resize(l.ntOff + 10);
      CHECK_EQ(Why(f), "not a PE file (bad NT header)"); }
    { Bytes f = file; Put<uint32_t>(f, l.ntOff, 0x4545);
      CHECK_EQ(Why(f), "not a PE file (bad NT header)"); }
    { Bytes f = file; Put<uint16_t>(f, l.ntOff + 4 + offsetof(Pe::FileHeader, Machine), 0x14C);
      CHECK_EQ(Why(f), "not x64 -- gbhook is 64-bit only"); }
    { Bytes f = file; Put<uint16_t>(f, l.ntOff + Pe::kOptionalHeaderOffset, 0x10B);
      CHECK_EQ(Why(f), "not a PE32+ image"); }
    { Bytes f = file; size_t o = l.ntOff + 4 + offsetof(Pe::FileHeader, Characteristics);
      Put<uint16_t>(f, o, (uint16_t)(Get<uint16_t>(f, o) & ~Pe::kFileDll));
      CHECK_EQ(Why(f), "not a DLL"); }
    { Bytes f = file; Put<uint16_t>(f, l.ntOff + 4 + offsetof(Pe::FileHeader, NumberOfSections), 0);
      CHECK_EQ(Why(f), "implausible section count"); }
    { Bytes f = file; Put<uint16_t>(f, l.ntOff + 4 + offsetof(Pe::FileHeader, NumberOfSections), 97);
      CHECK_EQ(Why(f), "implausible section count"); }
    // The largest optional-header size a 16-bit field can claim; the copy is shrunk below it first.
    { Bytes f = file; f.resize(60000); Put<uint16_t>(f, l.ntOff + 4 + offsetof(Pe::FileHeader, SizeOfOptionalHeader), 0xFFFF);
      CHECK_EQ(Why(f), "section table out of bounds"); }
    { Bytes f = file; f.resize(l.secOff + 10);
      CHECK_EQ(Why(f), "section table out of bounds"); }

    // The export directory.
    const size_t dirOff = l.ntOff + Pe::kOptionalHeaderOffset + offsetof(Pe::OptionalHeader64, DataDirectory);
    { Bytes f = file; Put<uint32_t>(f, dirOff, 0); Put<uint32_t>(f, dirOff + 4, 0);
      CHECK_EQ(Why(f), "no export directory -- did you forget GBHOOK_PLUGIN()?"); }
    { Bytes f = file; Put<uint32_t>(f, dirOff, 0x7FFFFFF0);
      CHECK_EQ(Why(f), "export directory RVA not in any section"); }
    { Bytes f = file; f.resize(l.lastRawEnd); Put<uint32_t>(f, dirOff, l.lastRva - 2);
      CHECK_EQ(Why(f), "export directory out of bounds"); }
    { Bytes f = file; Put<uint32_t>(f, l.expOff + offsetof(Pe::ExportDirectory, NumberOfNames), 0);
      CHECK_EQ(Why(f), "no named exports"); }
    { Bytes f = file; Put<uint32_t>(f, l.expOff + offsetof(Pe::ExportDirectory, AddressOfNames), 0x7FFFFFF0);
      CHECK_EQ(Why(f), "export tables not in any section"); }
    { Bytes f = file; f.resize(l.lastRawEnd);
      Put<uint32_t>(f, l.expOff + offsetof(Pe::ExportDirectory, AddressOfFunctions), l.lastRva - 2);
      CHECK_EQ(Why(f), "export tables out of bounds"); }

    // The name and the symbol.
    CHECK_EQ(Why(file, "GbhNothing"), "no 'GbhPluginManifest' export -- add GBHOOK_PLUGIN(...)");
    { Bytes f = file; f[l.nameStrOff] = 'X';
      CHECK_EQ(Why(f), "no 'GbhPluginManifest' export -- add GBHOOK_PLUGIN(...)"); }
    { Bytes f = file; Put<uint16_t>(f, l.ordEntryOff, 0xFFFF);
      CHECK_EQ(Why(f), "bad name ordinal"); }
    { Bytes f = file; Put<uint32_t>(f, l.funcEntryOff, l.expRva);
      CHECK_EQ(Why(f), "manifest export is a forwarder"); }
    { Bytes f = file; Put<uint32_t>(f, l.funcEntryOff, 0x7FFFFFF0);
      CHECK_EQ(Why(f), "manifest RVA not in any section"); }
    { Bytes f = file; f.resize(l.lastRawEnd); Put<uint32_t>(f, l.funcEntryOff, l.lastRva - 4);
      CHECK_EQ(Why(f), "manifest extends past end of file"); }

    return check::Done("pe");
}
