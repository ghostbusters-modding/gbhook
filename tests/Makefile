# gbhook: a mod loader for Ghostbusters: The Video Game Remastered
# Copyright (C) 2026 Colin Sullivan and contributors
# SPDX-License-Identifier: GPL-2.0-only
#
# make test    lint the pure packages, then build and run every suite under g++
# make cross   compile and link the Windows half into a dinput8.dll with mingw, as a check only
# make clean   remove bin/

.RECIPEPREFIX = >
.PHONY: test lint cross clean

CXX   ?= g++
CROSS ?= x86_64-w64-mingw32-g++
ROOT  := ..
BIN   := bin

CXXFLAGS   := -std=c++20 -Wall -Wextra -I. -I$(ROOT)/src -I$(ROOT)/sdk/include
CROSSFLAGS := $(CXXFLAGS) -Wno-cast-function-type -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_NO_WARNINGS -I$(ROOT)/third_party/minhook/include -static

# Pure packages: no windows.h, built and tested here. A package may include only the ones listed as its deps.
PURE_PKGS     := format registry
DEPS_format   :=
DEPS_registry :=

pkg_src   = $(wildcard $(ROOT)/src/$(1)/*.cpp)
pkg_hdr   = $(wildcard $(ROOT)/src/$(1)/*.h)
PURE_SRC := $(foreach p,$(PURE_PKGS),$(call pkg_src,$(p)))
WIN_SRC  := $(wildcard $(ROOT)/src/core/*.cpp) $(PURE_SRC)
SUITES   := $(foreach p,$(PURE_PKGS),$(patsubst $(p)/test_%.cpp,$(BIN)/$(p)/test_%,$(wildcard $(p)/test_*.cpp)))

test: lint $(SUITES)
> @set -e; for t in $(SUITES); do ./$$t; done

lint:
> @bad=$$(grep -rlE '^\s*#\s*include\s*[<"]windows\.h' $(foreach p,$(PURE_PKGS),$(ROOT)/src/$(p)) || true); if [ -n "$$bad" ]; then echo "lint: windows.h inside a pure package:"; echo "$$bad"; exit 1; fi; echo "lint: pure packages clean"

# A suite links its own package and that package's deps, nothing else.
define suite_rule
$(BIN)/$(1)/test_%: $(1)/test_%.cpp check.h $(call pkg_src,$(1)) $(call pkg_hdr,$(1)) $(foreach d,$(DEPS_$(1)),$(call pkg_src,$(d)) $(call pkg_hdr,$(d))) | $(BIN)/$(1)
> $$(CXX) $$(CXXFLAGS) -o $$@ $$< $(call pkg_src,$(1)) $(foreach d,$(DEPS_$(1)),$(call pkg_src,$(d)))
$(BIN)/$(1):
> mkdir -p $(BIN)/$(1)
endef
$(foreach p,$(PURE_PKGS),$(eval $(call suite_rule,$(p))))

cross: | $(BIN)
> $(CROSS) $(CROSSFLAGS) -shared -o $(BIN)/dinput8.dll $(WIN_SRC) stub/MinHookStub.cpp $(ROOT)/src/core/GbHook.def
> @echo "cross: linked $(BIN)/dinput8.dll"

$(BIN):
> mkdir -p $(BIN)

clean:
> rm -rf $(BIN)
