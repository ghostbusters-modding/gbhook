// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Writes a fixed POD with our writer, for crosscheck.py to open with the engine-verified reader.

#include "pod/Pod.h"
#include <cstdio>
#include <cstring>

static Pod::Item Item(const char* n, const char* s)
{
    std::vector<uint8_t> d(s, s + strlen(s));
    return { n, [d](std::vector<uint8_t>& o) { o = d; return true; } };
}

int main(int argc, char** argv)
{
    if (argc < 2) return 2;
    std::vector<Pod::Item> items = {
        Item("world\\haunt1.lvl", "level bytes here"),
        Item("art\\logo.tex", ""),
        Item("data\\scripts\\boot.dante", "0123456789ABCDEF0123"),
    };
    Pod::BufferSink sink;
    std::string why;
    if (!Pod::Write(sink, items, 1, "MODS.POD", &why)) { fprintf(stderr, "%s\n", why.c_str()); return 1; }
    FILE* f = fopen(argv[1], "wb");
    if (!f) return 1;
    fwrite(sink.bytes.data(), 1, sink.bytes.size(), f);
    fclose(f);
    return 0;
}
