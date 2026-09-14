// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for attr/Catalogue: the keys, the word grammar and the display forms.

#include "check.h"
#include "attr/Catalogue.h"

#include <cstring>
#include <string>

using namespace Catalogue;

namespace
{
    struct Row { const char* key; GbhAttrType type; bool writable; bool resettable; };

    // The catalogue as promised: every key, its type, and whether an engine native can set it.
    const Row kExpected[] = {
        { "god",     GBH_ATTR_BOOL,  true,  false },
        { "giant",   GBH_ATTR_BOOL,  true,  false },
        { "torpedo", GBH_ATTR_BOOL,  true,  false },
        { "hunt",    GBH_ATTR_BOOL,  true,  false },
        { "gravity", GBH_ATTR_FLOAT, true,  true  },
        { "time",    GBH_ATTR_FLOAT, true,  true  },
        { "fov",     GBH_ATTR_FLOAT, false, false },
        { "camdist", GBH_ATTR_FLOAT, false, false },
        { "cammode", GBH_ATTR_INT,   false, false },
    };
    constexpr int kExpectedCount = (int)(sizeof kExpected / sizeof kExpected[0]);

    bool ParseOk(const Entry& e, const char* value, float current, Parsed* out)
    {
        const char* why = nullptr;
        const bool ok = Parse(e, value, current, out, &why);
        if (ok) CHECK(why == nullptr);
        else    CHECK(why != nullptr && *why);
        return ok;
    }

    std::string Fmt(const Entry& e, float v)
    {
        char buf[32];
        Format(e, v, buf, sizeof buf);
        return buf;
    }
}

int main()
{
    // ---- the table ---------------------------------------------------------------
    CHECK_EQ(Count(), kExpectedCount);
    CHECK(Count() <= kMaxEntries);
    for (int i = 0; i < kExpectedCount; ++i)
    {
        const Entry* e = At(i);
        CHECK(e != nullptr);
        if (!e) continue;
        CHECK_EQ(std::string(e->key), kExpected[i].key);
        CHECK_EQ((int)e->type, (int)kExpected[i].type);
        CHECK_EQ(e->writable, kExpected[i].writable);
        CHECK_EQ(e->resettable, kExpected[i].resettable);
        CHECK(e->writable || !e->resettable);
        CHECK(Find(e->key) == e);
        // Every string fits the ABI struct it is copied into.
        GbhAttrInfo info;
        CHECK(strlen(e->key)  < sizeof info.key);
        CHECK(strlen(e->unit) < sizeof info.unit);
        CHECK(strlen(e->help) < sizeof info.help);
    }
    CHECK(At(-1) == nullptr);
    CHECK(At(Count()) == nullptr);

    // Lookup is case-insensitive. An unknown, empty or null key is nullptr.
    CHECK(Find("GOD") == Find("god"));
    CHECK(Find("Gravity") == Find("gravity"));
    CHECK(Find("noclip") == nullptr);
    CHECK(Find("letterbox") == nullptr);
    CHECK(Find("") == nullptr);
    CHECK(Find(nullptr) == nullptr);

    const Entry& god     = *Find("god");
    const Entry& gravity = *Find("gravity");
    const Entry& time    = *Find("time");
    const Entry& fov     = *Find("fov");
    const Entry& cammode = *Find("cammode");

    // Ranges and units as the setters accept them.
    CHECK_EQ(gravity.min, -100.0f);
    CHECK_EQ(gravity.max, 50.0f);
    CHECK_EQ(time.min, 0.05f);
    CHECK_EQ(time.max, 4.0f);
    CHECK_EQ(std::string(time.unit), "x");
    CHECK_EQ(std::string(fov.unit), "deg");
    CHECK_EQ(std::string(god.unit), "");
    CHECK(god.min == 0.0f && god.max == 0.0f);

    // ---- bool words ---------------------------------------------------------------
    Parsed p;
    const char* onWords[]  = { "on", "1", "true", "ON", "True" };
    const char* offWords[] = { "off", "0", "false", "OFF", "False" };
    for (const char* w : onWords)  { CHECK(ParseOk(god, w, 0.0f, &p)); CHECK_EQ(p.value, 1.0f); CHECK(!p.reset); }
    for (const char* w : offWords) { CHECK(ParseOk(god, w, 1.0f, &p)); CHECK_EQ(p.value, 0.0f); CHECK(!p.reset); }

    // toggle flips whatever is current.
    CHECK(ParseOk(god, "toggle", 0.0f, &p)); CHECK_EQ(p.value, 1.0f);
    CHECK(ParseOk(god, "toggle", 1.0f, &p)); CHECK_EQ(p.value, 0.0f);
    CHECK(ParseOk(god, "TOGGLE", 1.0f, &p)); CHECK_EQ(p.value, 0.0f);

    // A bool takes no number beyond 1 and 0, no empty word, and no reset.
    const char* why = nullptr;
    CHECK(!Parse(god, "2", 0.0f, &p, &why));      CHECK_EQ(std::string(why), "expected on|off|toggle");
    CHECK(!Parse(god, "yes", 0.0f, &p, &why));    CHECK_EQ(std::string(why), "expected on|off|toggle");
    CHECK(!Parse(god, "", 0.0f, &p, &why));       CHECK_EQ(std::string(why), "expected on|off|toggle");
    CHECK(!Parse(god, "reset", 0.0f, &p, &why));  CHECK_EQ(std::string(why), "no reset for this key");
    CHECK(!Parse(god, nullptr, 0.0f, &p, &why));  CHECK(why != nullptr);
    CHECK(!Parse(god, "2", 0.0f, &p, nullptr));   // a null `why` is tolerated

    // ---- numbers ------------------------------------------------------------------
    CHECK(ParseOk(gravity, "-32", 0.0f, &p));   CHECK_EQ(p.value, -32.0f); CHECK(!p.reset);
    CHECK(ParseOk(gravity, "-1.6", 0.0f, &p));  CHECK_EQ(p.value, -1.6f);
    CHECK(ParseOk(gravity, "50", 0.0f, &p));    CHECK_EQ(p.value, 50.0f);
    CHECK(ParseOk(gravity, "-100", 0.0f, &p));  CHECK_EQ(p.value, -100.0f);
    CHECK(ParseOk(gravity, "+5", 0.0f, &p));    CHECK_EQ(p.value, 5.0f);
    CHECK(ParseOk(time, "0.05", 1.0f, &p));     CHECK_EQ(p.value, 0.05f);
    CHECK(ParseOk(time, "4", 1.0f, &p));        CHECK_EQ(p.value, 4.0f);
    CHECK(ParseOk(time, "2.5", 1.0f, &p));      CHECK_EQ(p.value, 2.5f);

    // Out of range is refused at the edge, either side.
    CHECK(!Parse(gravity, "50.01", 0.0f, &p, &why));  CHECK_EQ(std::string(why), "out of range");
    CHECK(!Parse(gravity, "-100.5", 0.0f, &p, &why)); CHECK_EQ(std::string(why), "out of range");
    CHECK(!Parse(time, "0", 1.0f, &p, &why));         CHECK_EQ(std::string(why), "out of range");
    CHECK(!Parse(time, "0.04", 1.0f, &p, &why));      CHECK_EQ(std::string(why), "out of range");
    CHECK(!Parse(time, "4.5", 1.0f, &p, &why));       CHECK_EQ(std::string(why), "out of range");
    CHECK(!Parse(time, "-1", 1.0f, &p, &why));        CHECK_EQ(std::string(why), "out of range");

    // Not a number: junk, a trailing tail, a bool word, an empty word.
    CHECK(!Parse(gravity, "moon", 0.0f, &p, &why));   CHECK_EQ(std::string(why), "expected a number");
    CHECK(!Parse(gravity, "12abc", 0.0f, &p, &why));  CHECK_EQ(std::string(why), "expected a number");
    CHECK(!Parse(gravity, "toggle", 0.0f, &p, &why)); CHECK_EQ(std::string(why), "expected a number");
    CHECK(!Parse(gravity, "on", 0.0f, &p, &why));     CHECK_EQ(std::string(why), "expected a number");
    CHECK(!Parse(gravity, "", 0.0f, &p, &why));       CHECK_EQ(std::string(why), "expected a number");
    CHECK(!Parse(gravity, "inf", 0.0f, &p, &why));    CHECK_EQ(std::string(why), "expected a number");
    CHECK(!Parse(gravity, "nan", 0.0f, &p, &why));    CHECK_EQ(std::string(why), "expected a number");

    // An unbounded float takes any finite number. An int must be integral.
    CHECK(ParseOk(fov, "1234.5", 0.0f, &p));           CHECK_EQ(p.value, 1234.5f);
    CHECK(ParseOk(cammode, "13", 0.0f, &p));           CHECK_EQ(p.value, 13.0f);
    CHECK(!Parse(cammode, "1.5", 0.0f, &p, &why));     CHECK_EQ(std::string(why), "expected an integer");
    CHECK(!Parse(cammode, "orbit", 0.0f, &p, &why));   CHECK_EQ(std::string(why), "expected an integer");

    // ---- reset --------------------------------------------------------------------
    CHECK(ParseOk(gravity, "reset", 0.0f, &p)); CHECK(p.reset);
    CHECK(ParseOk(time, "RESET", 0.0f, &p));    CHECK(p.reset);
    CHECK(!Parse(fov, "reset", 0.0f, &p, &why));     CHECK_EQ(std::string(why), "no reset for this key");
    CHECK(!Parse(cammode, "reset", 0.0f, &p, &why)); CHECK_EQ(std::string(why), "no reset for this key");
    for (int i = 0; i < Count(); ++i)
    {
        const Entry& e = *At(i);
        CHECK_EQ(Parse(e, "reset", 0.0f, &p, &why), e.resettable);
    }

    // ---- display forms ------------------------------------------------------------
    CHECK_EQ(Fmt(god, 1.0f), "ON");
    CHECK_EQ(Fmt(god, 0.0f), "OFF");
    CHECK_EQ(Fmt(god, 7.0f), "ON");
    CHECK_EQ(Fmt(gravity, -32.0f), "-32.00");
    CHECK_EQ(Fmt(gravity, -1.6f), "-1.60");
    CHECK_EQ(Fmt(time, 1.0f), "1.00x");
    CHECK_EQ(Fmt(time, 0.25f), "0.25x");
    CHECK_EQ(Fmt(fov, 23.0f), "23.00 deg");
    CHECK_EQ(Fmt(*Find("camdist"), 4.5f), "4.50");
    CHECK_EQ(Fmt(cammode, 13.0f), "13");
    CHECK_EQ(Fmt(cammode, 0.0f), "0");

    // Format reports the full length, so a short buffer is detectable and still NUL-terminated.
    {
        char small[4];
        CHECK_EQ(Format(gravity, -32.0f, small, sizeof small), 6);
        CHECK_EQ(std::string(small), "-32");
        CHECK_EQ(Format(god, 1.0f, small, sizeof small), 2);
        CHECK_EQ(std::string(small), "ON");
        CHECK_EQ(Format(god, 1.0f, nullptr, 0), 0);
    }

    return check::Done("catalogue");
}
