// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#include "ModSet.h"
#include "format/Manifest.h"

#include <algorithm>
#include <cctype>

namespace
{
    std::string BareName(const std::string& p)
    {
        size_t s = p.find_last_of("/\\");
        return s == std::string::npos ? p : p.substr(s + 1);
    }

    std::string Lower(std::string s)
    {
        for (char& c : s) c = (char)tolower((unsigned char)c);
        return s;
    }

    // Judgement over one folder on its own.
    ModSet::Record Judge(const ModSet::Candidate& c)
    {
        ModSet::Record r;
        r.root   = c.root;
        r.folder = c.folder;

        if (!c.hasModIni) { r.refusal = "gbhook/ has no mod.ini"; return r; }

        r.mod      = c.ini.mod;
        r.warnings = c.ini.warnings;
        if (!c.ini.refusal.empty()) { r.refusal = c.ini.refusal; return r; }

        if (!r.mod.plugin.empty())
        {
            const std::string& p = r.mod.plugin;
            switch (c.binary)
            {
            case ModSet::Binary::None:       r.refusal = "plugin '" + p + "' was not read"; return r;
            case ModSet::Binary::Missing:    r.refusal = "plugin '" + p + "' is not in gbhook/"; return r;
            case ModSet::Binary::Unreadable: r.refusal = "plugin '" + p + "': " + c.binaryWhy; return r;
            case ModSet::Binary::Ok:         break;
            }

            const std::string why = Manifest::Validate(c.manifest);
            if (!why.empty()) { r.refusal = "plugin '" + p + "': " + why; return r; }

            // The two facts stated in both places: a mismatch means one was edited after the build.
            const std::string mid = Manifest::Field(c.manifest.id, sizeof c.manifest.id);
            if (mid != r.mod.id)
            {
                r.refusal = "mod.ini says id '" + r.mod.id + "' but " + p + " says '" + mid + "' -- one was edited after the build";
                return r;
            }
            if ((int)c.manifest.abi_version != r.mod.abi)
            {
                r.refusal = "mod.ini says abi " + std::to_string(r.mod.abi) + " but " + p + " says " +
                            std::to_string(c.manifest.abi_version) + " -- one was edited after the build";
                return r;
            }
            r.exclusiveHooks = Manifest::ExclusiveHooks(c.manifest);
        }

        if (c.hasModInfo && !r.mod.version.empty())
            r.warnings.push_back("version in mod.ini is ignored: previews/modinfo.ini states it");

        r.accepted = true;
        return r;
    }
}

namespace ModSet
{
    Result Resolve(const std::vector<Candidate>& found)
    {
        std::vector<Record> recs;
        recs.reserve(found.size());
        for (const Candidate& c : found) recs.push_back(Judge(c));

        // Duplicate ids: the first folder keeps the id.
        for (size_t i = 0; i < recs.size(); ++i)
        {
            if (!recs[i].accepted) continue;
            for (size_t j = i + 1; j < recs.size(); ++j)
                if (recs[j].accepted && recs[j].mod.id == recs[i].mod.id)
                {
                    recs[j].accepted = false;
                    recs[j].refusal  = "duplicate id '" + recs[i].mod.id + "', already claimed by folder '" + recs[i].folder + "'";
                }
        }

        // requires, to a fixed point: a refusal here can cascade.
        for (bool changed = true; changed;)
        {
            changed = false;
            for (Record& r : recs)
            {
                if (!r.accepted) continue;
                for (const std::string& need : r.mod.requires_)
                {
                    bool present = false, refused = false;
                    for (const Record& x : recs)
                        if (x.mod.id == need) { if (x.accepted) present = true; else refused = true; }
                    if (present) continue;
                    r.accepted = false;
                    r.refusal  = "requires '" + need + "', which " + (refused ? "was refused" : "is not present");
                    changed    = true;
                    break;
                }
            }
        }

        // Set-wide conflicts among the accepted. Both stay; the log names them while both are inert.
        Result out;
        for (size_t i = 0; i < recs.size(); ++i)
        {
            if (!recs[i].accepted) continue;
            for (size_t j = i + 1; j < recs.size(); ++j)
            {
                if (!recs[j].accepted) continue;
                for (const std::string& a : recs[i].exclusiveHooks)
                    for (const std::string& b : recs[j].exclusiveHooks)
                        if (a == b)
                            out.conflicts.push_back("hook " + a + " claimed by both '" + recs[i].mod.id + "' and '" +
                                                    recs[j].mod.id + "' -- whichever loads first wins and the other is refused at install time");
                for (const std::string& a : recs[i].mod.content)
                    for (const std::string& b : recs[j].mod.content)
                        if (Lower(BareName(a)) == Lower(BareName(b)))
                            out.conflicts.push_back("content " + BareName(a) + " shipped by both '" + recs[i].mod.id + "' and '" +
                                                    recs[j].mod.id + "' -- mount order decides, silently");
            }
        }

        // The code order: stage, priority, id. Two runs of one install must behave identically.
        std::vector<Record> accepted, refused;
        for (Record& r : recs) (r.accepted ? accepted : refused).push_back(r);
        std::stable_sort(accepted.begin(), accepted.end(), [](const Record& a, const Record& b)
        {
            if (a.mod.stage != b.mod.stage)       return a.mod.stage < b.mod.stage;
            if (a.mod.priority != b.mod.priority) return a.mod.priority < b.mod.priority;
            return a.mod.id < b.mod.id;
        });
        for (size_t i = 0; i < accepted.size(); ++i) accepted[i].order = (int)i;

        // requires states a dependency but orders nothing, so a dependency that inits later is named.
        for (Record& r : accepted)
            for (const std::string& need : r.mod.requires_)
                for (const Record& dep : accepted)
                    if (dep.mod.id == need && dep.order > r.order)
                        r.warnings.push_back("requires '" + need + "', which initialises after it -- give this mod a higher priority or a later stage");

        out.records = std::move(accepted);
        out.records.insert(out.records.end(), refused.begin(), refused.end());
        return out;
    }
}
