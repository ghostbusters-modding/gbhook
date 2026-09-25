// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// Suite for svc/ServiceTable: first publisher wins, exact-match find, owner attribution, the fixed cap.

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

    // Nothing published yet: a miss answers null and zeroes the size out.
    CHECK_EQ(t.Count(), 0);
    CHECK(t.Find("gb.menu.ui", &size) == nullptr);
    CHECK_EQ(size, 0u);
    CHECK(t.Find(nullptr, nullptr) == nullptr);
    CHECK(t.NameAt(0) == nullptr);
    CHECK(t.OwnerOf("gb.menu.ui") == nullptr);

    // Publish, then find with the size and without it.
    CHECK(t.Publish("gb.menu", "gb.menu.ui", &g_menu, sizeof(MenuUi), &why));
    CHECK_EQ(t.Count(), 1);
    CHECK(t.Find("gb.menu.ui", &size) == &g_menu);
    CHECK_EQ(size, (uint32_t)sizeof(MenuUi));
    CHECK(t.Find("gb.menu.ui", nullptr) == &g_menu);

    // Exact and case-sensitive: neither a prefix nor a case variant is the same service.
    CHECK(t.Find("gb.menu", nullptr) == nullptr);
    CHECK(t.Find("GB.MENU.UI", nullptr) == nullptr);
    CHECK(t.Find("gb.menu.ui.", nullptr) == nullptr);

    // A republish under the same name is a conflict that names the holder, and Find still says who holds it.
    CHECK(!t.Publish("gb.other", "gb.menu.ui", &g_net, sizeof(NetState), &why));
    CHECK_EQ(why, "'gb.menu.ui' is already published by gb.menu");
    CHECK(t.Find("gb.menu.ui", nullptr) == &g_menu);
    CHECK(!t.Publish("gb.menu", "gb.menu.ui", &g_menu, sizeof(MenuUi), &why));   // the owner cannot either
    CHECK_EQ(t.Count(), 1);

    // A second name is fine, and the same table may sit under two names.
    CHECK(t.Publish("gbcoop", "gb.coop.net", &g_net, sizeof(NetState), &why));
    CHECK(t.Publish(nullptr, "gb.camera", g_cam, sizeof g_cam, &why));
    CHECK(t.Publish("gbcoop", "gb.coop.net.alias", &g_net, sizeof(NetState), &why));
    CHECK_EQ(t.Count(), 4);
    CHECK(t.Find("gb.coop.net", &size) == &g_net);
    CHECK_EQ(size, (uint32_t)sizeof(NetState));
    CHECK(t.Find("gb.camera", &size) == g_cam);
    CHECK_EQ(size, (uint32_t)sizeof g_cam);

    // Owner attribution: the mod id verbatim, "" for the framework, null for a name nobody published.
    CHECK_EQ(std::string(t.OwnerOf("gb.menu.ui")), "gb.menu");
    CHECK_EQ(std::string(t.OwnerOf("gb.coop.net")), "gbcoop");
    CHECK_EQ(std::string(t.OwnerOf("gb.camera")), "");
    CHECK(t.OwnerOf("gb.nobody") == nullptr);
    CHECK(t.OwnerOf(nullptr) == nullptr);
    CHECK(t.OwnerOf("") == nullptr);

    // A conflict against a framework table names gbhook.
    CHECK(!t.Publish("gb.other", "gb.camera", g_cam, sizeof g_cam, &why));
    CHECK_EQ(why, "'gb.camera' is already published by gbhook");

    // NameAt walks publication order and refuses anything out of range.
    CHECK_EQ(std::string(t.NameAt(0)), "gb.menu.ui");
    CHECK_EQ(std::string(t.NameAt(1)), "gb.coop.net");
    CHECK_EQ(std::string(t.NameAt(2)), "gb.camera");
    CHECK_EQ(std::string(t.NameAt(3)), "gb.coop.net.alias");
    CHECK(t.NameAt(4) == nullptr);
    CHECK(t.NameAt(-1) == nullptr);

    // Refusals: empty name, overlong name, null table, a size that cannot hold struct_size. None of them lands.
    CHECK(!t.Publish("gb.x", "", &g_menu, sizeof(MenuUi), &why));
    CHECK_EQ(why, "empty service name");
    CHECK(!t.Publish("gb.x", nullptr, &g_menu, sizeof(MenuUi), &why));
    CHECK_EQ(why, "empty service name");
    const std::string longName(ServiceTable::kNameCap, 'n');
    CHECK(!t.Publish("gb.x", longName.c_str(), &g_menu, sizeof(MenuUi), &why));
    CHECK_EQ(why, "service name is too long");
    CHECK(!t.Publish("gb.x", "gb.x.null", nullptr, sizeof(MenuUi), &why));
    CHECK_EQ(why, "null table");
    CHECK(!t.Publish("gb.x", "gb.x.tiny", &g_menu, 3, &why));
    CHECK_EQ(why, "size is smaller than the struct_size field");
    CHECK(t.Publish("gb.x", "gb.x.min", &g_menu, 4, &why));   // exactly the field is the floor
    CHECK_EQ(t.Count(), 5);
    CHECK(t.Find("gb.x.null", nullptr) == nullptr);
    CHECK(t.Find("gb.x.tiny", nullptr) == nullptr);

    // The longest name that fits is kNameCap - 1 characters.
    const std::string edge(ServiceTable::kNameCap - 1, 'e');
    CHECK(t.Publish("gb.x", edge.c_str(), &g_menu, 4, &why));
    CHECK(t.Find(edge.c_str(), nullptr) == &g_menu);

    // A why of null is accepted on every path.
    CHECK(!t.Publish("gb.x", "", &g_menu, 4, nullptr));
    CHECK(!t.Publish("gb.x", "gb.menu.ui", &g_menu, 4, nullptr));

    // The cap: fill to kMaxEntries, the next one is refused, and everything already in stays findable.
    while (t.Count() < ServiceTable::kMaxEntries)
    {
        char name[32];
        snprintf(name, sizeof name, "gb.fill.%d", t.Count());
        CHECK(t.Publish("gb.fill", name, &g_menu, 4, &why));
    }
    CHECK(t.Full());
    CHECK_EQ(t.Count(), ServiceTable::kMaxEntries);
    CHECK(!t.Publish("gb.late", "gb.late.table", &g_net, sizeof(NetState), &why));
    CHECK_EQ(why, "the service table is full");
    CHECK(t.Find("gb.late.table", nullptr) == nullptr);
    CHECK(t.Find("gb.menu.ui", nullptr) == &g_menu);
    CHECK(t.Find("gb.fill.10", nullptr) == &g_menu);
    CHECK_EQ(std::string(t.NameAt(ServiceTable::kMaxEntries - 1)), "gb.fill.63");
    CHECK(t.NameAt(ServiceTable::kMaxEntries) == nullptr);

    // A conflict is still reported as one when the table is full.
    CHECK(!t.Publish("gb.late", "gb.menu.ui", &g_net, sizeof(NetState), &why));
    CHECK_EQ(why, "'gb.menu.ui' is already published by gb.menu");

    return check::Done("servicetable");
}
