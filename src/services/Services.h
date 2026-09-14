// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// Named C tables: one mod publishes, others find. svc/ServiceTable is the pure table; this is the lock and the log.
// A table is a pointer into the publisher's module and lives for the process, so nothing is ever copied or freed.

#include <cstdint>

namespace Services
{
    void Init();

    // `owner` is a mod id, or nullptr for the framework. GbhStatus: GBH_ERR_CONFLICT when the name is taken.
    int         Publish(const char* owner, const char* name, const void* table, uint32_t size);
    const void* Find(const char* name, uint32_t* size);   // nullptr until published; size may be null
    int         Count();
    const char* NameAt(int i);
    const char* OwnerOf(const char* name);                // nullptr when unknown; "" for the framework

    // `services`: every published table, its owner and size.
    void RegisterCommands();
    void LogSummary();
}
