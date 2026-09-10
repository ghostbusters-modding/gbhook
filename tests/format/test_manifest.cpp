// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for format/Manifest: what this framework refuses to load, with the exact wording.

#include "check.h"
#include "format/Manifest.h"

#include <cstring>

static GbhManifest Good()
{
    GbhManifest m;
    memset(&m, 0, sizeof m);
    memcpy(m.magic, GBH_MANIFEST_MAGIC, 9);
    m.struct_size  = sizeof m;
    m.abi_version  = GBHOOK_ABI_VERSION;
    strcpy(m.id, "gb.x");
    strcpy(m.target_md5, GBHOOK_TARGET_MD5);
    return m;
}

int main()
{
    CHECK_EQ(Manifest::Validate(Good()), "");

    { GbhManifest m = Good(); m.magic[0] = 'X';
      CHECK_EQ(Manifest::Validate(m), "manifest magic mismatch (stale SDK?)"); }

    { GbhManifest m = Good(); m.abi_version = 2;
      CHECK_EQ(Manifest::Validate(m), "built for ABI 2, this gbhook speaks ABI 1 -- rebuild the mod"); }

    // A smaller struct is an older SDK that appended nothing; a larger one knows fields we do not.
    { GbhManifest m = Good(); m.struct_size = sizeof m - 8;
      CHECK_EQ(Manifest::Validate(m), ""); }
    { GbhManifest m = Good(); m.struct_size = sizeof m + 8;
      CHECK_EQ(Manifest::Validate(m), "manifest is newer than this framework"); }

    { GbhManifest m = Good(); m.id[0] = '\0';
      CHECK_EQ(Manifest::Validate(m), "empty id in manifest"); }

    { GbhManifest m = Good(); strcpy(m.target_md5, "abc");
      CHECK_EQ(Manifest::Validate(m), "built for ghost.exe abc, this framework targets " GBHOOK_TARGET_MD5
                                      " -- an offset table applied to the wrong build crashes unreadably"); }
    { GbhManifest m = Good(); m.target_md5[0] = '\0';
      CHECK_EQ(Manifest::Validate(m), "built for ghost.exe (unset), this framework targets " GBHOOK_TARGET_MD5
                                      " -- an offset table applied to the wrong build crashes unreadably"); }

    // Field reads an array that fills its slot without a terminator.
    { char raw[4] = { 'a', 'b', 'c', 'd' };
      CHECK_EQ(Manifest::Field(raw, 4), "abcd");
      char t[4] = { 'a', '\0', 'c', 'd' };
      CHECK_EQ(Manifest::Field(t, 4), "a"); }

    // Exclusive hooks stop at the first empty entry.
    { GbhManifest m = Good();
      strcpy(m.exclusive_hooks[0], "ghost+0x1");
      strcpy(m.exclusive_hooks[1], "ghost+0x2");
      strcpy(m.exclusive_hooks[3], "ghost+0x4");
      std::vector<std::string> h = Manifest::ExclusiveHooks(m);
      CHECK_EQ(h.size(), (size_t)2);
      CHECK_EQ(h[1], "ghost+0x2"); }

    return check::Done("manifest");
}
