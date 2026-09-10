// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for modset/Content: the build/mount/stand-down decision per mod.

#include "check.h"
#include "modset/Content.h"

using Content::Action;
using Content::Decide;
using Content::Input;

namespace
{
    Content::Input WithLoose()
    {
        Input in;
        in.id = "gb.harbor";
        in.loose = { { "world\\harbor.lvl", 10, 100 }, { "sets\\harbor.bst", 20, 200 } };
        return in;
    }
}

int main()
{
    // A code-only mod, no loose files: nothing to do.
    {
        Input in; in.id = "gb.code";
        Content::Verdict v = Decide(in);
        CHECK_EQ((int)v.action, (int)Action::Skip);
    }

    // Loose files, no cache: build, and report the hash that keys it.
    {
        Content::Verdict v = Decide(WithLoose());
        CHECK_EQ((int)v.action, (int)Action::Build);
        CHECK(!v.hash.empty());
        CHECK_EQ(v.hash, TreeHash::Of(WithLoose().loose));
    }

    // Cache hash matches the current tree: mount the cached POD, no rebuild.
    {
        Input in = WithLoose();
        in.cachedHash = TreeHash::Of(in.loose);
        Content::Verdict v = Decide(in);
        CHECK_EQ((int)v.action, (int)Action::MountCached);
        CHECK_EQ(v.hash, in.cachedHash);
    }

    // A changed tree invalidates a stale cache: build again.
    {
        Input in = WithLoose();
        in.cachedHash = "deadbeefdeadbeef";
        Content::Verdict v = Decide(in);
        CHECK_EQ((int)v.action, (int)Action::Build);
        CHECK(v.hash != in.cachedHash);
    }

    // The manual switch wins over everything.
    {
        Input in = WithLoose();
        in.cachedHash = TreeHash::Of(in.loose);
        in.manualDisabled = true;
        Content::Verdict v = Decide(in);
        CHECK_EQ((int)v.action, (int)Action::Disabled);
        CHECK(v.reason.find("disabled") != std::string::npos);
    }

    // Every loose path already in the chain: GBMM deployed it, so stand down.
    {
        Input in = WithLoose();
        std::vector<std::string> chain = { "world\\harbor.lvl", "sets\\harbor.bst", "art\\unrelated.tex" };
        in.chainPaths = &chain;
        Content::Verdict v = Decide(in);
        CHECK_EQ((int)v.action, (int)Action::Disabled);
        CHECK(v.reason.find("chain") != std::string::npos);
    }

    // Only some paths in the chain: not a GBMM deployment, so build and mount ours.
    {
        Input in = WithLoose();
        std::vector<std::string> chain = { "world\\harbor.lvl" };   // missing the .bst
        in.chainPaths = &chain;
        Content::Verdict v = Decide(in);
        CHECK_EQ((int)v.action, (int)Action::Build);
    }

    // The chain match is case- and separator-insensitive, the way the engine compares names.
    {
        Input in = WithLoose();
        std::vector<std::string> chain = { "WORLD/HARBOR.LVL", "sets/Harbor.BST" };
        in.chainPaths = &chain;
        Content::Verdict v = Decide(in);
        CHECK_EQ((int)v.action, (int)Action::Disabled);
    }

    return check::Done("content");
}
