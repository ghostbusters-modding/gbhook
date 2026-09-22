// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Decides, per mod, whether to build its POD, mount it, or stand down because a chained POD already has it.
// Pure: the Windows layer feeds it facts and carries out the verdict. tests/modset/test_content.cpp is the spec.

#include "pod/TreeHash.h"

#include <string>
#include <vector>

namespace Content
{
    // What a loose path is to the engine. Only Asset is packed; the other two say why a file was left out.
    enum class Kind { Asset, NotAssetRoot, NotAssetType };

    // The rule: the top folder is one of the engine's asset roots and the extension is one it reads. Both lists
    // are the shipped archives' inventory, so docs, generators, build output and zips never reach a cache POD.
    bool IsAssetRoot(const std::string& topFolder);
    Kind Classify(const std::string& relpath);

    // What the discovery layer gathered about one mod's content, on disk.
    struct Input
    {
        std::string                id;              // the mod id, for the cache key and log lines
        std::vector<TreeHash::File> loose;          // its asset files, Classify() == Asset only; empty for a code-only mod
        std::string                cachedHash;      // the hash the last built cache POD was made from, "" if none
        // File paths (engine backslash form) reachable through the PATCH.POD chain right now.
        const std::vector<std::string>* chainPaths = nullptr;
    };

    enum class Action { Skip, Build, MountCached, InChain };

    struct Verdict
    {
        Action      action = Action::Skip;
        std::string hash;        // Build/MountCached: the tree hash that keys the cache
        std::string reason;      // a log line: why this action
    };

    // The rule, in one place:
    //  - no loose files            -> Skip (code-only; content is somebody else's concern)
    //  - every loose path already in the chain -> InChain (GBMM deployed it; we would only lose to it anyway)
    //  - cache hash matches         -> MountCached
    //  - otherwise                  -> Build
    Verdict Decide(const Input& in);
}
