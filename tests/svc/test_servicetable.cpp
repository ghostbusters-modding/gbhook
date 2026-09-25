// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for svc/ServiceTable: names under the publisher's id, first publisher wins, the fixed cap.

#include "check.h"
#include "svc/ServiceTable.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace
{
    // The shape every expected table shares: struct_size first, then function pointers.
    struct MenuUi  { uint32_t struct_size; void (*open)(); void (*close)(); };
    struct NetState { uint32_t struct_size; int (*peers)(); };

    MenuUi   g_menu = { sizeof(MenuUi), nullptr, nullptr };
    NetState g_net  = { sizeof(NetState), nullptr };
    uint32_t g_cam[8] = { sizeof(g_cam) };
}

int main()
{
    ServiceTable t;
    std::string  why;
    uint32_t     size = 0xFFFFFFFFu;

    // Qualify: the command rule. A framework name stays bare.
    CHECK_EQ(ServiceTable::Qualify("GbGui", "UI"), "gbgui.ui");
    CHECK_EQ(ServiceTable::Qualify(nullptr, "Camera"), "camera");
    CHECK_EQ(ServiceTable::Qualify("", "camera"), "camera");

    // Nothing published yet: a miss answers null and zeroes the size out.
    CHECK_EQ(t.Count(), 0);
    CHECK(t.Find("gbgui.ui", &size) == nullptr);
    CHECK_EQ(size, 0u);
    CHECK(t.Find(nullptr, nullptr) == nullptr);
    CHECK(t.NameAt(0) == nullptr);
    CHECK(t.OwnerOf("gbgui.ui") == nullptr);

    // A mod publishes its own part; everyone finds the qualified name, in any case.
    CHECK(t.Publish("gbgui", "ui", &g_menu, sizeof(MenuUi), &why));
    CHECK_EQ(t.Count(), 1);
    CHECK(t.Find("gbgui.ui", &size) == &g_menu);
    CHECK_EQ(size, (uint32_t)sizeof(MenuUi));
    CHECK(t.Find("gbgui.ui", nullptr) == &g_menu);
    CHECK(t.Find("GbGui.UI", nullptr) == &g_menu);

    // Neither the bare part, a prefix, nor a longer name is the same service.
    CHECK(t.Find("ui", nullptr) == nullptr);
    CHECK(t.Find("gbgui", nullptr) == nullptr);
    CHECK(t.Find("gbgui.ui.", nullptr) == nullptr);

    // Another mod's `ui` is its own service, so a name can never be squatted.
    CHECK(t.Publish("other", "ui", &g_net, sizeof(NetState), &why));
    CHECK(t.Find("other.ui", nullptr) == &g_net);
    CHECK(t.Find("gbgui.ui", nullptr) == &g_menu);

    // The owner republishing, in any case, is a conflict that names the holder.
    CHECK(!t.Publish("gbgui", "UI", &g_menu, sizeof(MenuUi), &why));
    CHECK_EQ(why, "'gbgui.ui' is already published by gbgui");
    CHECK_EQ(t.Count(), 2);

    // A dotted part is fine, and the same table may sit under two names.
    CHECK(t.Publish("gbcoop", "net", &g_net, sizeof(NetState), &why));
    CHECK(t.Publish(nullptr, "camera", g_cam, sizeof g_cam, &why));
    CHECK(t.Publish("gbcoop", "net.alias", &g_net, sizeof(NetState), &why));
    CHECK_EQ(t.Count(), 5);
    CHECK(t.Find("gbcoop.net", &size) == &g_net);
    CHECK_EQ(size, (uint32_t)sizeof(NetState));
    CHECK(t.Find("gbcoop.net.alias", nullptr) == &g_net);
    CHECK(t.Find("camera", &size) == g_cam);
    CHECK_EQ(size, (uint32_t)sizeof g_cam);

    // Owner attribution: the mod id verbatim, "" for the framework, null for a name nobody published.
    CHECK_EQ(std::string(t.OwnerOf("gbgui.ui")), "gbgui");
    CHECK_EQ(std::string(t.OwnerOf("GBCOOP.NET")), "gbcoop");
    CHECK_EQ(std::string(t.OwnerOf("camera")), "");
    CHECK(t.OwnerOf("nobody.ui") == nullptr);
    CHECK(t.OwnerOf(nullptr) == nullptr);
    CHECK(t.OwnerOf("") == nullptr);

    // A conflict against a framework table names gbhook.
    CHECK(!t.Publish(nullptr, "camera", g_cam, sizeof g_cam, &why));
    CHECK_EQ(why, "'camera' is already published by gbhook");

    // NameAt walks publication order with the qualified names.
    CHECK_EQ(std::string(t.NameAt(0)), "gbgui.ui");
    CHECK_EQ(std::string(t.NameAt(1)), "other.ui");
    CHECK_EQ(std::string(t.NameAt(2)), "gbcoop.net");
    CHECK_EQ(std::string(t.NameAt(3)), "camera");
    CHECK_EQ(std::string(t.NameAt(4)), "gbcoop.net.alias");
    CHECK(t.NameAt(5) == nullptr);
    CHECK(t.NameAt(-1) == nullptr);

    // Refusals: empty, blanks or quotes, overlong once qualified, null table, too small. None of them lands.
    CHECK(!t.Publish("x", "", &g_menu, sizeof(MenuUi), &why));
    CHECK_EQ(why, "empty service name");
    CHECK(!t.Publish("x", nullptr, &g_menu, sizeof(MenuUi), &why));
    CHECK_EQ(why, "empty service name");
    CHECK(!t.Publish("x", "two words", &g_menu, sizeof(MenuUi), &why));
    CHECK_EQ(why, "a service name cannot contain blanks or quotes");
    CHECK(!t.Publish("x", "say\"hi", &g_menu, sizeof(MenuUi), &why));
    CHECK_EQ(why, "a service name cannot contain blanks or quotes");
    const std::string longName(ServiceTable::kNameCap - 2, 'n');   // fits alone, not after "x."
    CHECK(!t.Publish("x", longName.c_str(), &g_menu, sizeof(MenuUi), &why));
    CHECK_EQ(why, "service name is too long");
    CHECK(!t.Publish("x", "null", nullptr, sizeof(MenuUi), &why));
    CHECK_EQ(why, "null table");
    CHECK(!t.Publish("x", "tiny", &g_menu, 3, &why));
    CHECK_EQ(why, "size is smaller than the struct_size field");
    CHECK(t.Publish("x", "min", &g_menu, 4, &why));   // exactly the field is the floor
    CHECK_EQ(t.Count(), 6);
    CHECK(t.Find("x.null", nullptr) == nullptr);
    CHECK(t.Find("x.tiny", nullptr) == nullptr);

    // The longest qualified name that fits is kNameCap - 1 characters.
    const std::string edge(ServiceTable::kNameCap - 3, 'e');
    CHECK(t.Publish("x", edge.c_str(), &g_menu, 4, &why));
    CHECK(t.Find(("x." + edge).c_str(), nullptr) == &g_menu);

    // A why of null is accepted on every path.
    CHECK(!t.Publish("x", "", &g_menu, 4, nullptr));
    CHECK(!t.Publish("gbgui", "ui", &g_menu, 4, nullptr));

    // The cap: fill to kMaxEntries, the next one is refused, and everything already in stays findable.
    while (t.Count() < ServiceTable::kMaxEntries)
    {
        char name[32];
        snprintf(name, sizeof name, "t%d", t.Count());
        CHECK(t.Publish("fill", name, &g_menu, 4, &why));
    }
    CHECK(t.Full());
    CHECK_EQ(t.Count(), ServiceTable::kMaxEntries);
    CHECK(!t.Publish("late", "table", &g_net, sizeof(NetState), &why));
    CHECK_EQ(why, "the service table is full");
    CHECK(t.Find("late.table", nullptr) == nullptr);
    CHECK(t.Find("gbgui.ui", nullptr) == &g_menu);
    CHECK(t.Find("fill.t10", nullptr) == &g_menu);
    CHECK_EQ(std::string(t.NameAt(ServiceTable::kMaxEntries - 1)), "fill.t63");
    CHECK(t.NameAt(ServiceTable::kMaxEntries) == nullptr);

    // A conflict is still reported as one when the table is full.
    CHECK(!t.Publish("gbgui", "ui", &g_net, sizeof(NetState), &why));
    CHECK_EQ(why, "'gbgui.ui' is already published by gbgui");

    return check::Done("servicetable");
}
