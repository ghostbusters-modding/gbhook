// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Pod.h"

#include <cstdio>
#include <cstring>

namespace
{
    constexpr uint32_t kMaxFiles = 10000000;   // the engine rejects a count at or above this

    void PutLe32(std::vector<uint8_t>& b, uint32_t v)
    {
        b.push_back((uint8_t)v); b.push_back((uint8_t)(v >> 8));
        b.push_back((uint8_t)(v >> 16)); b.push_back((uint8_t)(v >> 24));
    }

    uint32_t Le32(const uint8_t* p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }

    std::string DecodeChain(const uint8_t* hdr)
    {
        const uint8_t* s = hdr + Pod::kChainOff;
        size_t max = Pod::kHeader - Pod::kChainOff, n = 0;
        while (n < max && s[n] != '\0') ++n;
        return std::string((const char*)s, n);
    }
}

namespace Pod
{
    bool BufferSink::Put(const void* p, size_t n)
    {
        const uint8_t* b = (const uint8_t*)p;
        bytes.insert(bytes.end(), b, b + n);
        return true;
    }
    bool BufferSink::PatchAt(size_t off, const void* p, size_t n)
    {
        if (off + n > bytes.size()) return false;
        memcpy(bytes.data() + off, p, n);
        return true;
    }

    bool FileSink::Open(const char* path)
    {
        Close();
        fp = fopen(path, "wb+");
        return fp != nullptr;
    }
    void FileSink::Close()
    {
        if (fp) { fclose((FILE*)fp); fp = nullptr; }
    }
    bool FileSink::Put(const void* p, size_t n)
    {
        if (!fp) return false;
        if (n == 0) return true;
        // The body is written strictly forward, so seek to the end before appending after a header patch.
        fseek((FILE*)fp, 0, SEEK_END);
        return fwrite(p, 1, n, (FILE*)fp) == n;
    }
    bool FileSink::PatchAt(size_t off, const void* p, size_t n)
    {
        if (!fp) return false;
        if (fseek((FILE*)fp, (long)off, SEEK_SET) != 0) return false;
        bool ok = fwrite(p, 1, n, (FILE*)fp) == n;
        fseek((FILE*)fp, 0, SEEK_END);
        return ok;
    }
    size_t FileSink::Tell() const
    {
        if (!fp) return 0;
        fseek((FILE*)fp, 0, SEEK_END);
        long v = ftell((FILE*)fp);
        return v < 0 ? 0 : (size_t)v;
    }

    bool Write(Sink& out, const std::vector<Item>& items, uint32_t revision, const std::string& nextPod, std::string* why)
    {
        std::string sink;
        if (!why) why = &sink;

        if (nextPod.size() > kChainMax) { *why = "chain name '" + nextPod + "' is longer than the engine's buffer"; return false; }

        const uint8_t zero[kAlign] = { 0 };
        if (!out.Put(zero, 1) ) { *why = "sink write failed"; return false; }
        // A clean header of zeros; the real fields are patched in at the end.
        for (size_t i = 1; i < kHeader; ++i) out.Put(zero, 1);

        std::vector<IndexEntry> index;
        index.reserve(items.size());
        std::vector<uint8_t> buf;
        for (const Item& it : items)
        {
            buf.clear();
            if (!it.source || !it.source(buf)) { *why = "could not read '" + it.name + "'"; return false; }
            while (out.Tell() % kAlign) out.Put(zero, 1);

            IndexEntry e;
            e.name   = it.name;
            e.off    = (uint32_t)out.Tell();
            e.csize  = (uint32_t)buf.size();
            e.usize  = (uint32_t)buf.size();
            e.method = kStored;
            e.flags  = 0;
            if (!buf.empty() && !out.Put(buf.data(), buf.size())) { *why = "sink write failed"; return false; }
            index.push_back(e);
        }

        while (out.Tell() % kAlign) out.Put(zero, 1);
        const uint32_t indexOff = (uint32_t)out.Tell();

        // The name table: each name at its own offset, NUL-terminated, the whole table padded to 4.
        std::vector<uint8_t> names;
        std::vector<uint32_t> nameOff(index.size());
        for (size_t i = 0; i < index.size(); ++i)
        {
            nameOff[i] = (uint32_t)names.size();
            names.insert(names.end(), index[i].name.begin(), index[i].name.end());
            names.push_back(0);
        }
        while (names.size() % 4) names.push_back(0);

        std::vector<uint8_t> tail;
        for (size_t i = 0; i < index.size(); ++i)
        {
            PutLe32(tail, nameOff[i]);
            PutLe32(tail, index[i].csize);
            PutLe32(tail, index[i].off);
            PutLe32(tail, index[i].usize);
            PutLe32(tail, index[i].method);
            PutLe32(tail, index[i].flags);
        }
        tail.insert(tail.end(), names.begin(), names.end());
        if (!tail.empty() && !out.Put(tail.data(), tail.size())) { *why = "sink write failed"; return false; }

        std::vector<uint8_t> hdr;
        hdr.insert(hdr.end(), { 'P', 'O', 'D', '6' });
        PutLe32(hdr, (uint32_t)index.size());
        PutLe32(hdr, revision);
        PutLe32(hdr, indexOff);
        PutLe32(hdr, (uint32_t)names.size());
        if (!out.PatchAt(0, hdr.data(), hdr.size())) { *why = "sink patch failed"; return false; }

        std::vector<uint8_t> chain(kHeader - kChainOff, 0);
        memcpy(chain.data(), nextPod.data(), nextPod.size());
        if (!out.PatchAt(kChainOff, chain.data(), chain.size())) { *why = "sink patch failed"; return false; }

        why->clear();
        return true;
    }

    bool ReadHeader(const uint8_t* buf, size_t size, uint32_t* revision, std::string* nextPod, std::string* why)
    {
        std::string sink;
        if (!why) why = &sink;
        if (!buf || size < kHeader) { *why = "truncated header"; return false; }
        if (memcmp(buf, "POD6", 4) != 0) { *why = "not a POD6 archive (bad magic)"; return false; }
        if (revision) *revision = Le32(buf + 8);
        if (nextPod)  *nextPod  = DecodeChain(buf);
        why->clear();
        return true;
    }

    bool Read(const uint8_t* buf, size_t size, Archive& out, std::string* why)
    {
        std::string sink;
        if (!why) why = &sink;
        if (!buf || size < kHeader) { *why = "truncated header (need 0x80 bytes)"; return false; }
        if (memcmp(buf, "POD6", 4) != 0) { *why = "not a POD6 archive (bad magic)"; return false; }

        const uint32_t count    = Le32(buf + 4);
        out.revision            = Le32(buf + 8);
        out.indexOff            = Le32(buf + 0x0C);
        out.nameSize            = Le32(buf + 0x10);
        out.nextPod             = DecodeChain(buf);

        if (count >= kMaxFiles) { *why = "implausible file count"; return false; }
        const uint64_t idxBytes = (uint64_t)count * kEntry;
        if (out.indexOff > size || idxBytes > size - out.indexOff) { *why = "index out of bounds"; return false; }
        const uint64_t namesAt = (uint64_t)out.indexOff + idxBytes;
        if (namesAt > size || out.nameSize > size - namesAt) { *why = "name table out of bounds"; return false; }

        const uint8_t* names = buf + namesAt;
        out.entries.clear();
        out.entries.reserve(count);
        for (uint32_t i = 0; i < count; ++i)
        {
            const uint8_t* e = buf + out.indexOff + (uint64_t)i * kEntry;
            IndexEntry x;
            const uint32_t nameOff = Le32(e);
            x.csize  = Le32(e + 4);
            x.off    = Le32(e + 8);
            x.usize  = Le32(e + 12);
            x.method = Le32(e + 16);
            x.flags  = Le32(e + 20);
            if (nameOff >= out.nameSize) { *why = "entry name offset out of bounds"; return false; }
            size_t n = 0;
            while (nameOff + n < out.nameSize && names[nameOff + n] != '\0') ++n;
            x.name.assign((const char*)names + nameOff, n);
            out.entries.push_back(std::move(x));
        }
        why->clear();
        return true;
    }

    bool ReadEntry(const uint8_t* buf, size_t size, const IndexEntry& e, std::vector<uint8_t>& out, std::string* why)
    {
        std::string sink;
        if (!why) why = &sink;
        if (e.method == kDeflate) { *why = "deflate entries are not supported yet"; return false; }
        if (e.method != kStored)  { char b[64]; snprintf(b, sizeof b, "unknown compression method %u", e.method); *why = b; return false; }
        if (e.off > size || e.csize > size - e.off) { *why = "entry data out of bounds"; return false; }
        out.assign(buf + e.off, buf + e.off + e.csize);
        if (e.usize && out.size() != e.usize) { *why = "stored entry size mismatch"; return false; }
        why->clear();
        return true;
    }
}
