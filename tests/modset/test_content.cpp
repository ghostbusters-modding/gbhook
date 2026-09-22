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

    // The inclusion rule: every shipped asset root is in, whatever the case, and nothing else is.
    {
        const char* roots[] = { "animations", "art", "cinemats", "data", "fx", "materials",
                                "models", "physics", "sets", "skeletal", "sound", "world" };
        for (const char* r : roots) CHECK(Content::IsAssetRoot(r));
        CHECK(Content::IsAssetRoot("World"));
        CHECK(Content::IsAssetRoot("ART"));
        CHECK(!Content::IsAssetRoot("gen"));
        CHECK(!Content::IsAssetRoot("gbhook"));
        CHECK(!Content::IsAssetRoot("previews"));
        CHECK(!Content::IsAssetRoot("video"));
        CHECK(!Content::IsAssetRoot(""));
    }

    // A file is packed only under an asset root and only with an extension the engine reads.
    {
        using Content::Kind;
        CHECK_EQ((int)Content::Classify("world\\harbor1a.lvl"),          (int)Kind::Asset);
        CHECK_EQ((int)Content::Classify("world\\en\\ui.txt"),            (int)Kind::Asset);
        CHECK_EQ((int)Content::Classify("Art\\Props\\CRATE.TEX"),        (int)Kind::Asset);
        CHECK_EQ((int)Content::Classify("data/biped1/fiend.cib"),         (int)Kind::Asset);   // slash tolerated
        CHECK_EQ((int)Content::Classify("physics\\p.phys2b"),             (int)Kind::Asset);
        CHECK_EQ((int)Content::Classify("skeletal\\gb.bfm"),              (int)Kind::Asset);

        // Outside the roots: the mod's own paperwork, generators, build output, a copy unzipped in place.
        CHECK_EQ((int)Content::Classify("README.md"),                       (int)Kind::NotAssetRoot);
        CHECK_EQ((int)Content::Classify("harbor1a.lvl"),                    (int)Kind::NotAssetRoot);
        CHECK_EQ((int)Content::Classify("gen\\.build\\cits\\data\\gb.cib"), (int)Kind::NotAssetRoot);
        CHECK_EQ((int)Content::Classify("build\\x64\\Release\\Mod.pdb"),  (int)Kind::NotAssetRoot);
        CHECK_EQ((int)Content::Classify("ImmortalMod\\world\\x.lvl"),     (int)Kind::NotAssetRoot);
        CHECK_EQ((int)Content::Classify(".stage\\lvlF\\props\\models\\bigboss"), (int)Kind::NotAssetRoot);
        CHECK_EQ((int)Content::Classify("gbhook\\Mod.dll"),               (int)Kind::NotAssetRoot);
        CHECK_EQ((int)Content::Classify("previews\\modinfo.ini"),         (int)Kind::NotAssetRoot);

        // Under a root but not a type the engine has: a stray source file, a backup, no extension at all.
        CHECK_EQ((int)Content::Classify("art\\crate.png"),                (int)Kind::NotAssetType);
        CHECK_EQ((int)Content::Classify("world\\harbor1a.lvl.bak"),       (int)Kind::NotAssetType);
        CHECK_EQ((int)Content::Classify("models\\bigboss"),               (int)Kind::NotAssetType);
        CHECK_EQ((int)Content::Classify("sets\\.gitkeep"),                (int)Kind::NotAssetType);
        CHECK_EQ((int)Content::Classify("world\\gen.py\\harbor.lvl"),     (int)Kind::Asset);   // a dot in a folder is not an extension
        CHECK_EQ((int)Content::Classify("data\\ui.tex\\notes"),           (int)Kind::NotAssetType);
    }

    return check::Done("content");
}
