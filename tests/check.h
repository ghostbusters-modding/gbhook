// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
#pragma once
// The offline assertion harness: one executable per suite, exit 1 on any failed check.

#include <cstdio>
#include <sstream>
#include <string>

namespace check
{
    inline int g_passed = 0;
    inline int g_failed = 0;

    template <typename T>
    std::string Show(const T& v)
    {
        std::ostringstream s;
        s << v;
        return s.str();
    }
    inline std::string Show(const char* v)        { return v ? "\"" + std::string(v) + "\"" : "null"; }
    inline std::string Show(const std::string& v) { return "\"" + v + "\""; }
    inline std::string Show(bool v)               { return v ? "true" : "false"; }

    inline void Fail(const char* file, int line, const std::string& what)
    {
        ++g_failed;
        std::printf("  FAIL %s:%d: %s\n", file, line, what.c_str());
    }

    inline int Done(const char* suite)
    {
        std::printf("%s: %d passed, %d failed\n", suite, g_passed, g_failed);
        return g_failed ? 1 : 0;
    }
}

#define CHECK(cond) \
    do { if (cond) ++check::g_passed; else check::Fail(__FILE__, __LINE__, #cond); } while (0)

#define CHECK_EQ(actual, expected) \
    do { \
        auto a_ = (actual); \
        auto e_ = (expected); \
        if (a_ == e_) ++check::g_passed; \
        else check::Fail(__FILE__, __LINE__, std::string(#actual " == " #expected) + \
                         "\n       got " + check::Show(a_) + ", want " + check::Show(e_)); \
    } while (0)
