// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "Levels.h"
#include "Files.h"
#include "../core/Framework.h"
#include "../core/Seh.h"
#include "format/Dante.h"
#include "menu/LevelsPage.h"

#include <windows.h>

namespace
{
    constexpr uintptr_t kTableRva  = 0x7BD740;   // 20 x { int chapter, int index, const char* file }, firehouse.lvl on
    constexpr int       kTableRows = 20;
    constexpr int       kNameCap   = 64;

    struct LevelEntry { int32_t chapter, index; const char* file; };

    // Plain-C frame: the table is read-only data in the exe, guarded all the same.
    int CopyTable(char (*names)[kNameCap])
    {
        GBH_SEH_TRY
        {
            const LevelEntry* t = reinterpret_cast<const LevelEntry*>(gameBase + kTableRva);
            int n = 0;
            for (int i = 0; i < kTableRows; ++i)
            {
                if (!t[i].file) break;
                lstrcpynA(names[n++], t[i].file, kNameCap);
            }
            return n;
        }
        GBH_SEH_EXCEPT { return 0; }
    }
}

namespace Levels
{
    int Career(std::vector<std::string>& out)
    {
        out.clear();
        char names[kTableRows][kNameCap];
        const int n = CopyTable(names);
        if (n == 0) { Log::Write("LEVEL", "the career table could not be read"); return GBH_ERR_STATE; }
        for (int i = 0; i < n; ++i) out.push_back(LevelsPage::Stem(names[i]));
        return (int)out.size();
    }

    int Custom(std::vector<std::string>& out)
    {
        out.clear();
        std::vector<std::string> career;
        Career(career);   // unreadable just means nothing is excluded, same as the menu always did

        std::vector<std::string> found;
        const int n = Files::List("world", "*.lvl", found);
        if (n < 0) return n;

        out = LevelsPage::Custom(career, found).levels;
        return (int)out.size();
    }

    int Checkpoints(const char* stem, std::vector<std::string>& out)
    {
        out.clear();
        if (!stem || !*stem) return 0;
        std::vector<unsigned char> script;
        const int n = Files::Read(("world\\" + std::string(stem) + ".dante").c_str(), script);
        if (n <= 0) return 0;   // most levels have no script; that is not an error
        out = Dante::Checkpoints(reinterpret_cast<const char*>(script.data()), (size_t)n);
        return (int)out.size();
    }
}
