# gbhook: a mod loader for Ghostbusters: The Video Game Remastered
# Copyright (C) 2026 Colin Sullivan and contributors
# SPDX-License-Identifier: GPL-2.0-only
#
# make test    build and run every suite under g++ (the src/format layer has no windows.h)
# make cross   compile and link the Windows half into a dinput8.dll with mingw, as a check only
# make clean   remove bin/

.RECIPEPREFIX = >
.PHONY: test cross clean

CXX   ?= g++
CROSS ?= x86_64-w64-mingw32-g++
ROOT  := ..
BIN   := bin

CXXFLAGS   := -std=c++20 -Wall -Wextra -I$(ROOT)/src -I$(ROOT)/sdk/include
CROSSFLAGS := $(CXXFLAGS) -Wno-cast-function-type -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS -I$(ROOT)/third_party/minhook/include -static

FORMAT_SRC := $(wildcard $(ROOT)/src/format/*.cpp)
# Core files with no windows.h, testable here alongside the format layer.
PURE_SRC   := $(FORMAT_SRC) $(ROOT)/src/core/PatchRegistry.cpp $(ROOT)/src/core/VtableRegistry.cpp
PURE_HDR   := $(wildcard $(ROOT)/src/format/*.h) $(ROOT)/src/core/Memory.h $(ROOT)/src/core/PatchRegistry.h $(ROOT)/src/core/VtableRegistry.h
WIN_SRC    := $(wildcard $(ROOT)/src/core/*.cpp) $(FORMAT_SRC)
SUITES     := $(patsubst test_%.cpp,$(BIN)/test_%,$(wildcard test_*.cpp))

test: $(SUITES)
> @set -e; for t in $(SUITES); do ./$$t; done

$(BIN)/test_%: test_%.cpp check.h $(PURE_SRC) $(PURE_HDR) | $(BIN)
> $(CXX) $(CXXFLAGS) -o $@ $< $(PURE_SRC)

cross: | $(BIN)
> $(CROSS) $(CROSSFLAGS) -shared -o $(BIN)/dinput8.dll $(WIN_SRC) stub/MinHookStub.cpp $(ROOT)/src/core/GbHook.def
> @echo "cross: linked $(BIN)/dinput8.dll"

$(BIN):
> mkdir -p $(BIN)

clean:
> rm -rf $(BIN)
