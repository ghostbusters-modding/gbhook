// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// POD6 read and write, matching gbtvgr-py/pod.py byte for byte. tests/pod/test_pod.cpp is the spec.
// Writes method 0 (stored) only; deflate is deferred. docs/CONTENT.md and formats/pod_format.md.

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Pod
{
    constexpr uint32_t kHeader   = 0x80;   // the engine reads exactly this many header bytes
    constexpr uint32_t kAlign    = 16;     // file data sits on 16-byte boundaries
    constexpr uint32_t kEntry    = 24;     // bytes per index entry
    constexpr uint32_t kChainOff = 0x14;
    constexpr uint32_t kChainMax = 79;     // the engine strcpy's the chain name into an 80-byte buffer
    constexpr uint32_t kStored   = 0;
    constexpr uint32_t kDeflate  = 8;

    struct IndexEntry
    {
        std::string name;                  // engine paths use a backslash separator
        uint32_t    csize = 0, off = 0, usize = 0, method = 0, flags = 0;
    };

    struct Archive
    {
        uint32_t    revision = 0, indexOff = 0, nameSize = 0;
        std::string nextPod;               // the header's chain field, "" at the end
        std::vector<IndexEntry> entries;   // index order
    };

    // ---- writing ----------------------------------------------------------
    // One file to archive. `source` fills the buffer with its bytes, so the caller streams one file at a time.
    struct Item
    {
        std::string                                    name;
        std::function<bool(std::vector<uint8_t>&)>     source;
    };

    // A byte sink: append, patch the header once at the end, and report the position.
    struct Sink
    {
        virtual ~Sink() = default;
        virtual bool   Put(const void* p, size_t n)              = 0;
        virtual bool   PatchAt(size_t off, const void* p, size_t n) = 0;
        virtual size_t Tell() const                              = 0;
    };

    // A Sink backed by a byte vector, for the offline suite.
    struct BufferSink : Sink
    {
        std::vector<uint8_t> bytes;
        bool   Put(const void* p, size_t n) override;
        bool   PatchAt(size_t off, const void* p, size_t n) override;
        size_t Tell() const override { return bytes.size(); }
    };

    bool Write(Sink& out, const std::vector<Item>& items, uint32_t revision, const std::string& nextPod, std::string* why);

    // A Sink backed by a stdio file, for building a POD straight to the cache. Pure: stdio, no windows.h.
    struct FileSink : Sink
    {
        void*  fp = nullptr;   // FILE*, kept opaque so the header pulls in no <cstdio>
        bool   Open(const char* path);
        void   Close();
        bool   Put(const void* p, size_t n) override;
        bool   PatchAt(size_t off, const void* p, size_t n) override;
        size_t Tell() const override;
    };

    // ---- reading ----------------------------------------------------------
    // Parses the header and index of a POD6 image. Every field is bounds-checked; a corrupt cache never faults.
    bool Read(const uint8_t* buf, size_t size, Archive& out, std::string* why);

    // The bytes of one entry. Stored is copied; deflate is refused until it is implemented.
    bool ReadEntry(const uint8_t* buf, size_t size, const IndexEntry& e, std::vector<uint8_t>& out, std::string* why);

    // Just the chain field and revision from a header, for the chain walk. `buf` need only cover kHeader bytes.
    bool ReadHeader(const uint8_t* buf, size_t size, uint32_t* revision, std::string* nextPod, std::string* why);
}
