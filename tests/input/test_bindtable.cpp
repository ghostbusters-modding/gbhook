// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for input/BindTable: claims and clashes, the key edges, held state, enable, capture and focus loss.

#include "check.h"
#include "input/BindTable.h"

#include <cstdio>
#include <string>

namespace
{
    KeyChord::Key K(const char* text)
    {
        KeyChord::Key k;
        KeyChord::FromText(text, &k);
        return k;
    }

    constexpr int F = 0x46, G = 0x47, CTRL = 0x11, SHIFT = 0x10;
    BindTable::Table g_t;   // 30 KB; kept off the stack
}

int main()
{
    using BindTable::Add;
    BindTable::Table& t = g_t;

    // Names.
    CHECK(BindTable::ValidName("god"));
    CHECK(BindTable::ValidName("a.b-c_d"));
    CHECK(!BindTable::ValidName(""));
    CHECK(!BindTable::ValidName(nullptr));
    CHECK(!BindTable::ValidName("two words"));
    CHECK(!BindTable::ValidName("tab\there"));
    CHECK(BindTable::ValidName(std::string(31, 'x').c_str()));
    CHECK(!BindTable::ValidName(std::string(32, 'x').c_str()));
    CHECK(t.Add("gb.a", "has space", "", K("F"), nullptr, nullptr).status == Add::BadName);

    // Registration and clashes: the first claim on a chord wins, and the loser learns who holds it.
    const BindTable::AddResult f = t.Add("gb.a", "fly", "toggle flight", K("F"), nullptr, nullptr);
    CHECK(f.status == Add::Ok);
    CHECK_EQ(f.clashWith, -1);
    CHECK(t.Add("gb.a", "fly", "", K("G"), nullptr, nullptr).status == Add::Taken);
    const BindTable::AddResult lose = t.Add("gb.b", "flash", "", K("F"), nullptr, nullptr);
    CHECK(lose.status == Add::Ok);
    CHECK_EQ(lose.clashWith, f.index);
    CHECK_EQ(t.At(lose.index)->key.vk, 0);
    CHECK_EQ(std::string(t.At(lose.clashWith)->owner), "gb.a");
    CHECK_EQ(std::string(t.At(lose.clashWith)->name), "fly");
    const BindTable::AddResult cf = t.Add("gb.b", "find", "", K("CTRL+F"), nullptr, nullptr);
    CHECK_EQ(cf.clashWith, -1);   // F and CTRL+F are different claims
    CHECK(t.Add("gb.a", "same", "", K("ctrl+f"), nullptr, nullptr).clashWith == cf.index);
    CHECK_EQ(t.Find("gb.b", "fly"), -1);   // names are per owner
    CHECK_EQ(t.Find("gb.a", "fly"), f.index);
    const BindTable::AddResult un = t.Add("gb.b", "idle", std::string(200, 'h').c_str(), KeyChord::Key(), nullptr, nullptr);
    CHECK(un.status == Add::Ok);
    CHECK_EQ(std::string(t.At(un.index)->help).size(), (size_t)95);

    // The most specific chord fires; a key nobody claims passes.
    BindTable::Edge e = t.Down(F);
    CHECK_EQ(e.fired, f.index);
    CHECK(e.consume);
    CHECK(t.Up(F));
    t.Down(CTRL);
    e = t.Down(F);
    CHECK_EQ(e.fired, cf.index);
    CHECK(t.Held(cf.index));
    CHECK(t.Held(f.index));   // plain F is down too: held asks only that its chord's keys are down
    t.Up(F);
    CHECK(!t.Up(CTRL));
    t.Down(CTRL);
    t.Down(SHIFT);
    e = t.Down(F);                // CTRL+SHIFT held: CTRL+F is the best subset
    CHECK_EQ(e.fired, cf.index);
    t.ReleaseAll();
    e = t.Down(G);
    CHECK_EQ(e.fired, -1);
    CHECK(!e.consume);
    CHECK(!t.Up(G));

    // Auto-repeat fires nothing, and is eaten only when the first down was.
    e = t.Down(F);
    CHECK_EQ(e.fired, f.index);
    e = t.Down(F);
    CHECK_EQ(e.fired, -1);
    CHECK(e.consume);
    CHECK(t.Up(F));
    t.Down(G);
    e = t.Down(G);
    CHECK(!e.consume);
    CHECK(!t.Up(G));
    CHECK(!t.Up(F));    // an up with no down passes

    // Disabled: the chord stays claimed, the key reaches the engine, nothing fires, held answers false.
    CHECK(t.Enable(f.index, false));
    e = t.Down(F);
    CHECK_EQ(e.fired, -1);
    CHECK(!e.consume);
    CHECK(!t.Held(f.index));
    CHECK(t.Enable(f.index, true));   // on mid-press: no fire and no stuck key
    CHECK(t.Held(f.index));
    CHECK_EQ(t.Down(F).fired, -1);
    CHECK(!t.Up(F));
    CHECK(t.Add("gb.c", "grab", "", K("F"), nullptr, nullptr).clashWith == f.index);
    e = t.Down(F);                    // off mid-press: the up is still eaten
    CHECK(t.Enable(f.index, false));
    CHECK(t.Up(F));
    t.Enable(f.index, true);
    CHECK(!t.Enable(999, true));

    // Capture: only the holder fires, the others' bound keys are eaten, unclaimed keys pass.
    const BindTable::AddResult gm = t.Add("gb.menu", "nav", "", K("G"), nullptr, nullptr);
    CHECK(t.Capture("gb.menu", true));
    CHECK(t.Capture("gb.menu", true));
    CHECK(!t.Capture("gb.a", true));
    CHECK(!t.Capture("gb.a", false));
    CHECK(t.CapturedBy("gb.menu"));
    e = t.Down(F);
    CHECK_EQ(e.fired, -1);
    CHECK(e.consume);
    CHECK(!t.Held(f.index));
    CHECK(t.Up(F));
    e = t.Down(G);
    CHECK_EQ(e.fired, gm.index);
    CHECK(t.Held(gm.index));
    t.Up(G);
    e = t.Down(0x48);   // H: nobody's
    CHECK(!e.consume);
    t.Up(0x48);
    CHECK(t.Capture("gb.menu", false));
    CHECK(!t.CapturedBy("gb.menu"));
    CHECK_EQ(t.Down(F).fired, f.index);
    t.Up(F);

    // Focus loss clears what is down without firing, and the next press is a fresh edge.
    t.Down(CTRL);
    t.Down(F);
    CHECK(t.Held(cf.index));
    t.ReleaseAll();
    CHECK(!t.Held(cf.index));
    CHECK(!t.Up(F));
    CHECK_EQ(t.Down(F).fired, f.index);
    t.ReleaseAll();

    // A faulted action is off for good.
    t.Kill(f.index);
    t.Enable(f.index, true);
    CHECK(!t.Live(f.index));
    CHECK(!t.Down(F).consume);
    t.ReleaseAll();

    // Out-of-range keys pass.
    CHECK(!t.Down(0).consume);
    CHECK(!t.Down(300).consume);
    CHECK(!t.Up(-1));

    // The table fills at kMaxClaims.
    char name[16];
    int i = t.Count();
    for (; i < BindTable::kMaxClaims; ++i)
    {
        snprintf(name, sizeof name, "n%d", i);
        CHECK(t.Add("gb.fill", name, "", KeyChord::Key(), nullptr, nullptr).status == Add::Ok);
    }
    CHECK(t.Add("gb.fill", "onemore", "", KeyChord::Key(), nullptr, nullptr).status == Add::Full);
    CHECK_EQ(t.Count(), BindTable::kMaxClaims);

    return check::Done("bindtable");
}
