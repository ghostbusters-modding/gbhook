// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for pod/Pod: a POD6 built here reads back through the same code, and its layout matches pod.py.

#include "check.h"
#include "pod/Pod.h"

#include <cstdio>

#include <cstring>
#include <string>
#include <vector>

namespace
{
    std::vector<uint8_t> Str(const char* s) { return std::vector<uint8_t>(s, s + strlen(s)); }

    Pod::Item Item(const char* name, const std::vector<uint8_t>& data)
    {
        std::vector<uint8_t> copy = data;
        return { name, [copy](std::vector<uint8_t>& out) { out = copy; return true; } };
    }

    template <typename T> T Le(const std::vector<uint8_t>& b, size_t off) { T v; memcpy(&v, &b[off], sizeof v); return v; }
}

int main()
{
    // A two-file archive round-trips: names, order, bytes, and the store method.
    {
        std::vector<Pod::Item> items = {
            Item("world\\a.lvl", Str("hello")),
            Item("art\\b.tex",   Str("second file, longer")),
        };
        Pod::BufferSink sink;
        std::string why = "unset";
        CHECK(Pod::Write(sink, items, 1, "", &why));
        CHECK_EQ(why, "");

        const std::vector<uint8_t>& b = sink.bytes;
        CHECK(b.size() > Pod::kHeader);
        CHECK_EQ(std::string((const char*)b.data(), 4), "POD6");
        CHECK_EQ(Le<uint32_t>(b, 4), (uint32_t)2);           // count
        CHECK_EQ(Le<uint32_t>(b, 8), (uint32_t)1);           // revision
        const uint32_t indexOff = Le<uint32_t>(b, 0x0C);
        CHECK_EQ(indexOff % Pod::kAlign, (uint32_t)0);
        CHECK_EQ(b[0x14], '\0');                             // empty chain

        Pod::Archive a;
        CHECK(Pod::Read(b.data(), b.size(), a, &why));
        CHECK_EQ(why, "");
        CHECK_EQ(a.revision, (uint32_t)1);
        CHECK_EQ(a.nextPod, "");
        CHECK_EQ(a.entries.size(), (size_t)2);
        CHECK_EQ(a.entries[0].name, "world\\a.lvl");
        CHECK_EQ(a.entries[1].name, "art\\b.tex");
        CHECK_EQ(a.entries[0].method, (uint32_t)0);
        CHECK_EQ(a.entries[0].usize, (uint32_t)5);
        CHECK_EQ(a.entries[0].csize, (uint32_t)5);           // stored: csize == usize
        CHECK_EQ(a.entries[0].off % Pod::kAlign, (uint32_t)0);

        std::vector<uint8_t> got;
        CHECK(Pod::ReadEntry(b.data(), b.size(), a.entries[0], got, &why));
        CHECK_EQ(std::string(got.begin(), got.end()), "hello");
        CHECK(Pod::ReadEntry(b.data(), b.size(), a.entries[1], got, &why));
        CHECK_EQ(std::string(got.begin(), got.end()), "second file, longer");
    }

    // The chain field is written and read, and refused past the engine's buffer.
    {
        Pod::BufferSink sink;
        std::string why;
        CHECK(Pod::Write(sink, { Item("x", Str("y")) }, 1000, "MODS.POD", &why));
        Pod::Archive a;
        CHECK(Pod::Read(sink.bytes.data(), sink.bytes.size(), a, &why));
        CHECK_EQ(a.nextPod, "MODS.POD");
        CHECK_EQ(a.revision, (uint32_t)1000);

        uint32_t rev = 0; std::string next;
        CHECK(Pod::ReadHeader(sink.bytes.data(), sink.bytes.size(), &rev, &next, &why));
        CHECK_EQ(rev, (uint32_t)1000);
        CHECK_EQ(next, "MODS.POD");

        Pod::BufferSink s2;
        CHECK(!Pod::Write(s2, { Item("x", Str("y")) }, 1, std::string(80, 'A'), &why));
        CHECK(why.find("chain") != std::string::npos);
    }

    // An empty archive is valid: header, no files, an empty index.
    {
        Pod::BufferSink sink;
        std::string why;
        CHECK(Pod::Write(sink, {}, 1, "", &why));
        Pod::Archive a;
        CHECK(Pod::Read(sink.bytes.data(), sink.bytes.size(), a, &why));
        CHECK_EQ(a.entries.size(), (size_t)0);
    }

    // A source that fails aborts the write cleanly.
    {
        Pod::BufferSink sink;
        std::string why;
        std::vector<Pod::Item> items = { { "bad", [](std::vector<uint8_t>&) { return false; } } };
        CHECK(!Pod::Write(sink, items, 1, "", &why));
        CHECK(why.find("bad") != std::string::npos);
    }

    // Read refuses the malformed: bad magic, a short header, an index out of bounds.
    {
        std::string why;
        Pod::Archive a;
        std::vector<uint8_t> junk(200, 0);
        CHECK(!Pod::Read(junk.data(), junk.size(), a, &why));
        CHECK(why.find("POD6") != std::string::npos);

        Pod::BufferSink sink;
        CHECK(Pod::Write(sink, { Item("x", Str("y")) }, 1, "", &why));
        std::vector<uint8_t> b = sink.bytes;
        CHECK(!Pod::Read(b.data(), 0x40, a, &why));          // shorter than a header
        CHECK(why.find("header") != std::string::npos);

        b = sink.bytes;
        uint32_t huge = 0x7FFFFFFF;
        memcpy(&b[0x0C], &huge, 4);                          // index offset past the end
        CHECK(!Pod::Read(b.data(), b.size(), a, &why));

        b = sink.bytes;
        memcpy(&b[4], &huge, 4);                             // an absurd file count
        CHECK(!Pod::Read(b.data(), b.size(), a, &why));
    }

    // Deflate is refused with a clear reason until it is implemented.
    {
        Pod::BufferSink sink;
        std::string why;
        CHECK(Pod::Write(sink, { Item("x", Str("hello")) }, 1, "", &why));
        Pod::Archive a;
        CHECK(Pod::Read(sink.bytes.data(), sink.bytes.size(), a, &why));
        a.entries[0].method = Pod::kDeflate;
        std::vector<uint8_t> got;
        CHECK(!Pod::ReadEntry(sink.bytes.data(), sink.bytes.size(), a.entries[0], got, &why));
        CHECK(why.find("deflate") != std::string::npos);
    }

    // FileSink writes the same bytes as BufferSink, and reads back through a real file.
    {
        std::vector<Pod::Item> items = { Item("world\\a.lvl", Str("hello")), Item("x", Str("")) };
        Pod::BufferSink buf;
        std::string why;
        CHECK(Pod::Write(buf, items, 7, "N.POD", &why));

        const char* path = "bin/pod/filesink.POD";
        Pod::FileSink fs;
        CHECK(fs.Open(path));
        CHECK(Pod::Write(fs, items, 7, "N.POD", &why));
        fs.Close();

        FILE* f = fopen(path, "rb");
        CHECK(f != nullptr);
        std::vector<uint8_t> disk;
        if (f) { int c; while ((c = fgetc(f)) != EOF) disk.push_back((uint8_t)c); fclose(f); }
        CHECK_EQ(disk.size(), buf.bytes.size());
        CHECK(disk == buf.bytes);

        Pod::Archive a;
        CHECK(Pod::Read(disk.data(), disk.size(), a, &why));
        CHECK_EQ(a.entries.size(), (size_t)2);
        CHECK_EQ(a.revision, (uint32_t)7);
        CHECK_EQ(a.nextPod, "N.POD");
    }

    return check::Done("pod");
}
