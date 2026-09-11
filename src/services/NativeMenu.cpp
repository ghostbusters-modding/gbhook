// gbhook: a mod loader for Ghostbusters: The Video Game Remastered
// Copyright (C) 2026 Colin Sullivan and contributors
// SPDX-License-Identifier: GPL-2.0-only
// A page is a fresh CLevelMenu wearing a copied vtable: slot 81 publishes our rows and never chains, slot 83 runs our
// action, slot 40 answers the title, slot 16 says the page is hidden, slot 0 says it is gone. Pages nest the way the
// game's own screens do: a child pushed onto the page that is activating, ESC popping one level.
#include "NativeMenu.h"
#include "Pump.h"
#include "../core/Framework.h"
#include "../core/HookBroker.h"
#include "../core/Seh.h"

#include <windows.h>
#include <cstddef>
#include <cstring>

namespace
{
    // ---- CMainMenu: the row broker ---------------------------------------------------------------------
    constexpr uintptr_t kHideFnRva   = 0x20F020;   // char __fastcall isRowHidden(this, idx): hides 0, 1 and 3 by index
    constexpr uintptr_t kDispatchRva = 0x20ECD0;   // void __fastcall activateRow(this, row)
    constexpr uintptr_t kRefillRva   = 0x20E900;   // slot 80: every label from the string table; hooked for ours
    constexpr int       kMainRows    = 7;
    constexpr uintptr_t kMainLabel0  = 0x170;      // row labels, engine Strings, stride 0x38
    constexpr uintptr_t kMainStride  = 0x38;

    // The menu reordered: the shipped index is the slot on screen, the entry is what that slot shows and does.
    // A retail slot forwards to the game's own case; a slot with none is claimable and hidden until claimed.
    struct Slot { const char* key; int retail; const char* fixed; };
    const Slot kLayout[kMainRows] = {
        { "CMainMenu_Career",      2,  nullptr },   // 0
        { "CMainMenu_Multiplayer", -1, nullptr },   // 1  GBH_ROW_ONLINE
        { nullptr,                 -1, "Mods" },    // 2  gbhook's page
        { nullptr,                 -1, nullptr },   // 3  GBH_ROW_FREE
        { "CMainMenu_Options",     4,  nullptr },   // 4
        { "CMainMenu_Extras",      5,  nullptr },   // 5
        { "CMainMenu_Exit",        6,  nullptr },   // 6
    };

    // ---- CLevelMenu: the page --------------------------------------------------------------------------
    constexpr uintptr_t kAllocRva     = 0x2C4760;   // operator new(size_t); the stock case and populate use the same
    constexpr uintptr_t kLevelCtorRva = 0x20F8C0;   // CLevelMenu::CLevelMenu(), zeroes the row fields
    constexpr uintptr_t kRowCtorRva   = 0x2251B0;   // row descriptor ctor
    constexpr uintptr_t kStrAssignRva = 0x323C00;   // String = const char*
    constexpr uintptr_t kPushRva      = 0x299650;   // parent+0x40 = child, child+0x10 = parent, parent state 5
    constexpr uintptr_t kOwnerLinkRva = 0x2985C0;   // rows -> owner, then parks the selection
    constexpr uintptr_t kLevelVtRva   = 0x7CA868;   // CLevelMenu::vftable, 116 slots
    constexpr uintptr_t kMainVtRva    = 0x7CA190;   // CMainMenu::vftable, probed for the hidden-row slot
    constexpr uintptr_t kLocalizeRva  = 0x291D20;   // String* localize(String* out, const char* key)
    constexpr uintptr_t kStrDtorRva   = 0x324480;   // String::~String
    constexpr int       kVtSlots      = 116;
    constexpr int       kSlotDtor     = 0;          // the deleting destructor, chained: the parent runs it after the close
    constexpr int       kSlotHide     = 16;         // +0x080 chained: the "page is hidden" signal, ESC or ours
    constexpr int       kSlotTitle    = 40;         // +0x140 replaced
    constexpr int       kSlotSelect   = 76;         // +0x260 select row, what the owner link finishes with
    constexpr int       kSlotPopulate = 81;         // +0x288 replaced, never chained: the stock one scans world\*.lvl
    constexpr int       kSlotActivate = 83;         // +0x298 replaced: the stock one loads the row as a .lvl
    constexpr uintptr_t kOffRowCount  = 0x130;
    constexpr uintptr_t kOffRowArray  = 0x138;
    constexpr uintptr_t kOffSelected  = 0x2C;
    constexpr uintptr_t kOffStateFn   = 0x338;      // vt slot 103, set state
    constexpr int       kStateClose   = 2;          // what the engine's own ESC asks of a child
    constexpr size_t    kLevelMenuSize = 0x140;
    constexpr size_t    kRowStride    = 0x38;
    constexpr uintptr_t kRowLabelOff  = 0x08;
    constexpr int       kScreenRows   = GBH_NATIVE_MAX_ROWS;   // always published in full; the count is fixed per open
    constexpr int       kMaxDepth     = 16;   // gbhook's own cap, a few KB of static rows per page

    typedef void* (__fastcall* tAlloc)(size_t);
    typedef void* (__fastcall* tCtor1)(void*);
    typedef void* (__fastcall* tStrAssign)(void*, const char*);
    typedef void  (__fastcall* tPush)(void*, void*);
    typedef void  (__fastcall* tState)(void*, int);
    typedef void  (__fastcall* tOwnerLink)(void*);
    typedef void* (__fastcall* tVecDtor)(void*, unsigned int);
    typedef void  (__fastcall* tSelect)(void*, int, int, int);
    typedef void  (__fastcall* tRowActivate)(void*, int);
    typedef char  (__fastcall* tIsRowHidden)(void*, int);
    typedef void  (__fastcall* tRefill)(void*);
    typedef const char* (__fastcall* tTitleFn)(void*);
    typedef void  (__fastcall* tHideFn)(void*);
    typedef void* (__fastcall* tDtorFn)(void*, unsigned int);
    // The engine's String is one refcounted pointer; padded so a callee assuming the 24-byte home space stays inside.
    struct GbString { const char* p; unsigned long long pad[2]; };
    typedef GbString* (__fastcall* tLocalize)(GbString*, const char*);
    typedef void      (__fastcall* tStrDtor)(GbString*);

    template <class T> T Fn(uintptr_t rva) { return (T)(void*)(gameBase + rva); }
    bool Localize(const char* key, char* out, size_t cap);

    struct Claim
    {
        GbhRowFn fn = nullptr;
        void*    user = nullptr;
        char     owner[64] = { 0 };
        char     label[64] = { 0 };
        bool     held = false;
    };
    Claim g_rows[kMainRows];
    bool  g_installed = false, g_installTried = false;
    void* g_rowSelf = nullptr;   // the CMainMenu, valid inside a claim callback only

    tRowActivate oRowActivate = nullptr;
    tIsRowHidden oIsRowHidden = nullptr;
    tRefill      oRefill      = nullptr;

    void**       g_vt         = nullptr;   // the copied CLevelMenu table, the broker's for the process
    int          g_hideSlot   = -1;
    tIsRowHidden oChildHidden = nullptr;
    tTitleFn     oChildTitle  = nullptr;
    tHideFn      oChildHide   = nullptr;
    tDtorFn      oChildDtor   = nullptr;

    struct SubRow { char label[GBH_NATIVE_LABEL_CAP]; int action; };

    // One open page. The stack's top is the page the player is on; ESC pops it and its parent page is live again.
    struct Page
    {
        void*             child;     // the CLevelMenu object, until the engine hides it
        void*             object;    // the same, until the engine destroys it
        void*             rowsMem;
        GbhNativeMenuDesc desc;
        char              title[96];
        SubRow            sub[kScreenRows];
        int               subN;
        SubRow            shown[kScreenRows];
        int               shownN;
    };
    Page  g_pages[kMaxDepth];
    int   g_depth       = 0;         // pages pushed and not yet hidden
    int   g_liveObjects = 0;         // pages the engine has not yet destroyed
    Page* g_building    = nullptr;   // the page whose build() is running
    void* g_activating  = nullptr;   // the page whose activate() is running: a push from there nests under it
    bool  g_closingAll  = false;
    void (*g_after)(void*) = nullptr;
    void* g_afterUser   = nullptr;

    Page* ByChild(void* self)
    {
        for (int i = 0; i < g_depth; ++i) if (g_pages[i].child == self) return &g_pages[i];
        return nullptr;
    }

    Page* ByObject(void* self)
    {
        for (int i = 0; i < kMaxDepth; ++i) if (g_pages[i].object == self) return &g_pages[i];
        return nullptr;
    }

    // ---- plain-C frames -------------------------------------------------------------------------------
    bool CallRow(GbhRowFn fn, int row, void* user)
    {
        GBH_SEH_TRY { fn(row, user); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    bool CallBuild(void (*fn)(void*), void* user)
    {
        GBH_SEH_TRY { fn(user); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    int CallActivate(int (*fn)(int, void*), int action, void* user, bool* faulted)
    {
        *faulted = false;
        GBH_SEH_TRY { return fn(action, user); }
        GBH_SEH_EXCEPT { *faulted = true; return GBH_NATIVE_CLOSE; }
    }

    bool AssignLabel(void* strAt, const char* text)
    {
        GBH_SEH_TRY { Fn<tStrAssign>(kStrAssignRva)(strAt, text); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    // CMainMenu and CLevelMenu share a base, so the index of CMainMenu's isRowHidden is the index of the same
    // virtual in CLevelMenu. Only a unique hit is trusted.
    int ProbeHideSlot()
    {
        int found = -1, hits = 0;
        GBH_SEH_TRY
        {
            void* const* mainVt = (void* const*)(gameBase + kMainVtRva);
            void* const  target = (void*)(gameBase + kHideFnRva);
            for (int i = 0; i < kVtSlots; ++i)
                if (mainVt[i] == target) { if (!hits) found = i; ++hits; }
        }
        GBH_SEH_EXCEPT { return -1; }
        if (hits != 1 || found == kSlotActivate || found == kSlotPopulate) return -1;
        return found;
    }

    // The engine frees the block with the count from the operator new[] cookie before it, so write that cookie.
    // Zeroed first: the row ctor leaves +0x20 and +0x30 alone, and the engine follows +0x20.
    void* AllocRows()
    {
        const size_t bytes = sizeof(unsigned long long) + kRowStride * (size_t)kScreenRows;
        void* base = nullptr;
        GBH_SEH_TRY
        {
            base = Fn<tAlloc>(kAllocRva)(bytes);
            if (!base) return nullptr;
            memset(base, 0, bytes);
            *(unsigned long long*)base = (unsigned long long)kScreenRows;
            char* mem = (char*)base + sizeof(unsigned long long);
            for (int i = 0; i < kScreenRows; ++i) Fn<tCtor1>(kRowCtorRva)(mem + (size_t)i * kRowStride);
            return mem;
        }
        GBH_SEH_EXCEPT { return nullptr; }
    }

    void WriteLabels(Page& p)
    {
        for (int i = 0; i < kScreenRows; ++i)
            AssignLabel((char*)p.rowsMem + (size_t)i * kRowStride + kRowLabelOff, i < p.subN ? p.sub[i].label : "");
    }

    void RebuildRows(Page& p)
    {
        p.subN = 0;
        if (!p.desc.build) return;
        g_building = &p;
        if (!CallBuild(p.desc.build, p.desc.user)) Log::Write("NMENU", "FAULT in a page's build callback");
        g_building = nullptr;
    }

    enum Shape { kUnchanged, kRelabel, kReshape };
    Shape CompareShown(const Page& p)
    {
        if (p.shownN != p.subN) return kReshape;
        bool same = true;
        for (int i = 0; i < p.subN; ++i)
        {
            if (p.shown[i].action != p.sub[i].action) return kReshape;
            if (strcmp(p.shown[i].label, p.sub[i].label) != 0) same = false;
        }
        return same ? kUnchanged : kRelabel;
    }
    void Snapshot(Page& p) { memcpy(p.shown, p.sub, sizeof p.shown); p.shownN = p.subN; }

    // Publish, in the stock order: array, count, labels, owner link.
    bool PublishRows(Page& p, void* self)
    {
        GBH_SEH_TRY
        {
            void** old = *(void***)((char*)self + kOffRowArray);
            if (old)
            {
                const unsigned long long n = ((unsigned long long*)old)[-1];
                if (n) { void** rvt = *(void***)old; ((tVecDtor)rvt[0])(old, 3); }
                *(void**)((char*)self + kOffRowArray) = nullptr;
                *(int*) ((char*)self + kOffRowCount)  = 0;
            }
            p.rowsMem = AllocRows();
            if (!p.rowsMem) return false;
            *(void**)((char*)self + kOffRowArray) = p.rowsMem;
            *(int*) ((char*)self + kOffRowCount)  = kScreenRows;
            WriteLabels(p);
            Fn<tOwnerLink>(kOwnerLinkRva)(self);
            return true;
        }
        GBH_SEH_EXCEPT { return false; }
    }

    // Re-label a live page in place. The block is never swapped: the screen may hold pointers into it.
    bool RelabelLive(Page& p, bool keepCursor)
    {
        GBH_SEH_TRY
        {
            if (*(void***)p.child != g_vt) return false;
            const int sel = keepCursor ? *(int*)((char*)p.child + kOffSelected) : -1;
            WriteLabels(p);
            Fn<tOwnerLink>(kOwnerLinkRva)(p.child);
            if (sel >= 0 && sel < p.subN)
            {
                void** vt = *(void***)p.child;
                ((tSelect)vt[kSlotSelect])(p.child, sel, 1, 0);
            }
            return true;
        }
        GBH_SEH_EXCEPT { return false; }
    }

    void ApplyLive(Page& p, bool keepCursor)
    {
        if (!p.child || !p.rowsMem) return;
        if (!RelabelLive(p, keepCursor)) { Log::Write("NMENU", "re-label skipped: the page is gone"); return; }
        Snapshot(p);
    }

    bool CloseChild(void* self)
    {
        GBH_SEH_TRY { void** vt = *(void***)self; ((tState)vt[kOffStateFn / 8])(self, kStateClose); return true; }
        GBH_SEH_EXCEPT { return false; }
    }

    bool Localize(const char* key, char* out, size_t cap)
    {
        GBH_SEH_TRY
        {
            GbString s = { nullptr, { 0, 0 } };
            Fn<tLocalize>(kLocalizeRva)(&s, key);
            out[0] = 0;
            if (s.p && *s.p) lstrcpynA(out, s.p, (int)cap);
            Fn<tStrDtor>(kStrDtorRva)(&s);
            return true;
        }
        GBH_SEH_EXCEPT { out[0] = 0; return false; }
    }

    // A leading '@' names a localisation key; a miss comes back as "???" and keeps the donor's title.
    void ResolveTitle(Page& p, const char* t)
    {
        p.title[0] = 0;
        if (!t || !*t) return;
        if (*t != '@') { lstrcpynA(p.title, t, (int)sizeof p.title); return; }
        Localize(t + 1, p.title, sizeof p.title);
        if (!p.title[0] || strcmp(p.title, "???") == 0)
        {
            Log::Writef("NMENU", "title key '%s' is not in the string table; the donor's title stays", t + 1);
            p.title[0] = 0;
        }
    }

    void CopyDesc(Page& p, const GbhNativeMenuDesc* desc)
    {
        size_t n = desc->struct_size;
        if (n < offsetof(GbhNativeMenuDesc, title)) n = offsetof(GbhNativeMenuDesc, title);
        if (n > sizeof p.desc) n = sizeof p.desc;
        memset(&p.desc, 0, sizeof p.desc);
        memcpy(&p.desc, desc, n);
    }

    void* NewChild(void* parent)
    {
        GBH_SEH_TRY
        {
            void* child = Fn<tAlloc>(kAllocRva)(kLevelMenuSize);
            if (!child) return nullptr;
            Fn<tCtor1>(kLevelCtorRva)(child);
            *(void***)child = g_vt;   // a transient object: written directly, not registered with the sweep
            Fn<tPush>(kPushRva)(parent, child);
            return child;
        }
        GBH_SEH_EXCEPT { return nullptr; }
    }

    // Closing the stack goes one page per pass: the next close waits until the engine has destroyed the top.
    bool CloseNext(void*)
    {
        if (!g_closingAll) return true;
        if (g_depth == 0) { g_closingAll = false; return true; }
        CloseChild(g_pages[g_depth - 1].child);
        return true;
    }

    // ---- the copied vtable's slots ---------------------------------------------------------------------
    void __fastcall hkPopulate(void* self)
    {
        Page* p = ByChild(self);
        if (!p) return;
        if (!PublishRows(*p, self)) { Log::Write("NMENU", "populate FAILED: the row block could not be published"); return; }
        Snapshot(*p);
        Log::Writef("NMENU", "page %d populated: %d row(s) shown of %d published", (int)(p - g_pages) + 1, p->subN, kScreenRows);
    }

    char __fastcall hkChildRowHidden(void* self, int idx)
    {
        if (Page* p = ByChild(self)) return (idx >= 0 && idx < p->subN) ? (char)0 : (char)1;
        return oChildHidden ? oChildHidden(self, idx) : (char)0;
    }

    const char* __fastcall hkChildTitle(void* self)
    {
        Page* p = ByChild(self);
        if (p && p->title[0]) return p->title;
        return oChildTitle ? oChildTitle(self) : "";
    }

    // Chain first, then forget the page. The hidden page is the top of the stack; its parent page is live again.
    void __fastcall hkChildHide(void* self)
    {
        if (oChildHide) oChildHide(self);
        Page* p = ByChild(self);
        if (!p) return;
        const int at = (int)(p - g_pages);
        Log::Writef("NMENU", "page %d hidden by the engine", at + 1);
        p->child = nullptr;
        p->rowsMem = nullptr;
        if (at == g_depth - 1) g_depth = at;
        else Log::Writef("NMENU", "page %d hidden under %d open page(s); unexpected", at + 1, g_depth - at - 1);
    }

    // The parent frees the page after the close. When the last of ours is gone, whatever waited for that runs.
    void* __fastcall hkChildDtor(void* self, unsigned int flags)
    {
        Page* p = ByObject(self);
        void* r = oChildDtor ? oChildDtor(self, flags) : nullptr;
        if (!p) return r;
        p->object = nullptr;
        if (g_liveObjects > 0) --g_liveObjects;
        Log::Writef("NMENU", "page object destroyed by the engine, %d left", g_liveObjects);
        if (g_closingAll && g_depth > 0) Pump::Park(CloseNext, nullptr, "close the next page");
        if (g_liveObjects == 0) g_closingAll = false;
        if (g_liveObjects == 0 && g_after)
        {
            void (*fn)(void*) = g_after;
            void* user = g_afterUser;
            g_after = nullptr; g_afterUser = nullptr;
            fn(user);
        }
        return r;
    }

    // The third argument is the row pointer; its offset in our block is the index, whatever the second means.
    void __fastcall hkChildActivate(void* self, int idx, void* rowPtr)
    {
        Page* p = ByChild(self);
        if (!p || g_closingAll) return;
        int row = idx;
        if (rowPtr && p->rowsMem)
        {
            const ptrdiff_t d = (char*)rowPtr - (char*)p->rowsMem;
            if (d >= 0 && (d % (ptrdiff_t)kRowStride) == 0)
            {
                const int i = (int)(d / (ptrdiff_t)kRowStride);
                if (i >= 0 && i < kScreenRows) row = i;
            }
        }
        if (row < 0 || row >= p->subN) return;          // a padding row
        if (p->sub[row].action == GBH_NATIVE_INERT) return;

        int verdict = GBH_NATIVE_STAY;
        const int depthBefore = g_depth;
        if (p->desc.activate)
        {
            bool faulted = false;
            g_activating = self;
            verdict = CallActivate(p->desc.activate, p->sub[row].action, p->desc.user, &faulted);
            g_activating = nullptr;
            if (faulted) Log::Write("NMENU", "FAULT in a page's activate callback; closing the page");
        }
        if (g_depth > depthBefore) return;               // the activation pushed a child page; this one stays as it is
        if (verdict == GBH_NATIVE_CLOSE_ALL)
        {
            g_closingAll = true;
            if (!CloseChild(self)) Log::Write("NMENU", "EXC asking the page to close");
            return;
        }
        if (verdict == GBH_NATIVE_CLOSE)
        {
            // Close the way ESC does: ask the child for state 2 and let its parent finish. Driving the parent
            // directly leaves an undeleted child attached and a menu that answers no key.
            if (!CloseChild(self)) Log::Write("NMENU", "EXC asking the page to close");
            return;
        }
        RebuildRows(*p);
        const Shape s = CompareShown(*p);
        if (s != kUnchanged) ApplyLive(*p, s == kRelabel);
    }

    // ---- the CMainMenu detours -------------------------------------------------------------------------
    void __fastcall hkRowActivate(void* self, int row)
    {
        if (row >= 0 && row < kMainRows && g_rows[row].held && g_rows[row].fn)
        {
            g_rowSelf = self;
            Log::Writef("NMENU", "row %d activated -> '%s'", row, g_rows[row].owner);
            if (!CallRow(g_rows[row].fn, row, g_rows[row].user))
            {
                Log::Writef("NMENU", "FAULT in '%s' row callback; the claim on row %d is dropped", g_rows[row].owner, row);
                g_rows[row].held = false;
            }
            g_rowSelf = nullptr;
            return;   // a claimed row never falls through to the shipped case
        }
        // A retail slot runs the game's own case for the row it shows; an unclaimed free slot does nothing.
        if (row >= 0 && row < kMainRows && kLayout[row].retail >= 0 && oRowActivate) oRowActivate(self, kLayout[row].retail);
    }

    char __fastcall hkIsRowHidden(void* self, int idx)
    {
        if (idx >= 0 && idx < kMainRows) return (g_rows[idx].held || kLayout[idx].retail >= 0) ? (char)0 : (char)1;
        return oIsRowHidden ? oIsRowHidden(self, idx) : (char)1;
    }

    // The stock refill resolves all seven labels from the string table on first show and on a language change;
    // every slot's own text goes on after it: a claimant's label, the fixed word, or the key localised.
    void __fastcall hkRefill(void* self)
    {
        if (oRefill) oRefill(self);
        for (int i = 0; i < kMainRows; ++i)
        {
            char text[96] = { 0 };
            if (g_rows[i].held && g_rows[i].label[0]) lstrcpynA(text, g_rows[i].label, (int)sizeof text);
            else if (kLayout[i].fixed)                lstrcpynA(text, kLayout[i].fixed, (int)sizeof text);
            else if (kLayout[i].key)                  Localize(kLayout[i].key, text, sizeof text);
            if (text[0] && strcmp(text, "???") != 0) AssignLabel((char*)self + kMainLabel0 + (size_t)i * kMainStride, text);
        }
    }

    bool EnsureVtable()
    {
        if (g_vt) return true;
        void* copy = HookBroker::VtableClone(nullptr, gameBase + kLevelVtRva, kVtSlots);
        if (!copy) return false;
        void* orig = nullptr;
        bool ok = HookBroker::VtableSlot(nullptr, copy, kSlotActivate, (void*)&hkChildActivate, &orig);
        ok = HookBroker::VtableSlot(nullptr, copy, kSlotPopulate, (void*)&hkPopulate, &orig) && ok;
        ok = HookBroker::VtableSlot(nullptr, copy, kSlotTitle, (void*)&hkChildTitle, &orig) && ok;
        if (ok) oChildTitle = (tTitleFn)orig;
        ok = HookBroker::VtableSlot(nullptr, copy, kSlotHide, (void*)&hkChildHide, &orig) && ok;
        if (ok) oChildHide = (tHideFn)orig;
        ok = HookBroker::VtableSlot(nullptr, copy, kSlotDtor, (void*)&hkChildDtor, &orig) && ok;
        if (ok) oChildDtor = (tDtorFn)orig;
        g_hideSlot = ProbeHideSlot();
        if (g_hideSlot >= 0)
        {
            if (HookBroker::VtableSlot(nullptr, copy, g_hideSlot, (void*)&hkChildRowHidden, &orig)) oChildHidden = (tIsRowHidden)orig;
            else g_hideSlot = -1;
        }
        if (!ok) { Log::Write("NMENU", "the page vtable could not be prepared"); return false; }
        g_vt = (void**)copy;
        Log::Writef("NMENU", "page vtable ready: activate %d, populate %d, title %d, hide %d, hidden-row %s",
                    kSlotActivate, kSlotPopulate, kSlotTitle, kSlotHide,
                    g_hideSlot >= 0 ? "found" : "not found (surplus rows draw blank)");
        return true;
    }
}

namespace NativeMenu
{
    void Install()
    {
        if (g_installed || !gameBase) return;
        g_installTried = true;

        // Deferred and armed together: a live dispatcher with a hidden row, or a visible row running the shipped
        // empty case, both look broken.
        GbhHook a = HookBroker::Install(nullptr, gameBase + kDispatchRva, (void*)&hkRowActivate, (void**)&oRowActivate, GBH_HOOK_DEFERRED | GBH_HOOK_EXCLUSIVE);
        GbhHook b = HookBroker::Install(nullptr, gameBase + kHideFnRva,   (void*)&hkIsRowHidden, (void**)&oIsRowHidden, GBH_HOOK_DEFERRED | GBH_HOOK_EXCLUSIVE);
        GbhHook c = HookBroker::Install(nullptr, gameBase + kRefillRva,   (void*)&hkRefill,      (void**)&oRefill,      GBH_HOOK_DEFERRED | GBH_HOOK_EXCLUSIVE);
        if (!a || !b) { Log::Write("NMENU", "the front-end row pair could not be hooked; rows stay as shipped"); return; }
        if (!c) Log::Write("NMENU", "the label refill could not be hooked; claimed rows keep their shipped text");
        const GbhHook set[3] = { a, b, c };
        if (!HookBroker::EnableBatch(set, c ? 3 : 2)) { Log::Write("NMENU", "the front-end row pair could not be enabled"); return; }
        g_installed = true;

        int pending = 0;
        for (int i = 0; i < kMainRows; ++i) if (g_rows[i].held) ++pending;
        Log::Writef("NMENU", "front-end row broker installed, %d claim(s) live", pending);
    }

    bool Active() { return g_installed; }

    int ClaimRow(const char* owner, int row, GbhRowFn fn, void* user)
    {
        if (row < 0 || row >= kMainRows || !fn) return GBH_ERR_ARG;
        if (g_installTried && !g_installed) return GBH_ERR_STATE;
        const char* who = (owner && *owner) ? owner : "gbhook";
        if (kLayout[row].retail >= 0)
        {
            Log::Writef("NMENU", "row %d is the game's own %s row; '%s' refused", row, kLayout[row].key, who);
            return GBH_ERR_CONFLICT;
        }
        if (g_rows[row].held)
        {
            Log::Writef("NMENU", "row %d already claimed by '%s'; '%s' refused", row, g_rows[row].owner, who);
            return GBH_ERR_CONFLICT;
        }
        g_rows[row].fn   = fn;
        g_rows[row].user = user;
        g_rows[row].held = true;
        lstrcpynA(g_rows[row].owner, who, (int)sizeof g_rows[row].owner);
        Log::Writef("NMENU", "row %d claimed by '%s'%s", row, who, g_installed ? "" : " (live once the broker installs)");
        return GBH_OK;
    }

    int SetRowLabel(const char* owner, int row, const char* label)
    {
        if (row < 0 || row >= kMainRows || !label) return GBH_ERR_ARG;
        if (!g_rows[row].held) return GBH_ERR_STATE;
        if (strcmp(g_rows[row].owner, (owner && *owner) ? owner : "gbhook") != 0) return GBH_ERR_CONFLICT;
        lstrcpynA(g_rows[row].label, label, (int)sizeof g_rows[row].label);
        return GBH_OK;
    }

    int OpenPage(const GbhNativeMenuDesc* desc)
    {
        if (!desc || !desc->build || !desc->activate) return GBH_ERR_ARG;
        if (!gameBase) return GBH_ERR_STATE;
        // The parent is the page that is activating, or the main menu inside a claimed row's callback.
        void* parent = g_activating ? g_activating : g_rowSelf;
        if (!parent || g_closingAll) return GBH_ERR_STATE;
        if (g_depth >= kMaxDepth) { Log::Writef("NMENU", "page refused: %d pages are already open", g_depth); return GBH_ERR_STATE; }
        if (!EnsureVtable()) return GBH_ERR_STATE;

        Page& p = g_pages[g_depth];
        memset(&p, 0, sizeof p);
        p.shownN = -1;
        CopyDesc(p, desc);
        RebuildRows(p);
        ResolveTitle(p, p.desc.title);

        void* child = NewChild(parent);
        if (!child) { Log::Write("NMENU", "the page could not be created or pushed"); return GBH_ERR_STATE; }
        p.child  = child;
        p.object = child;
        ++g_depth;
        ++g_liveObjects;
        // The rows are written a few frames later, when the engine's first-show asks slot 81.
        Log::Writef("NMENU", "page %d pushed (%d row(s), title '%s')", g_depth, p.subN, p.title[0] ? p.title : "the donor's");
        return GBH_OK;
    }

    int AddRow(const char* label, int action)
    {
        if (!g_building) return GBH_ERR_STATE;
        Page& p = *g_building;
        if (p.subN >= kScreenRows) return GBH_ERR_STATE;
        lstrcpynA(p.sub[p.subN].label, label ? label : "", GBH_NATIVE_LABEL_CAP);
        p.sub[p.subN].action = action;
        ++p.subN;
        return GBH_OK;
    }

    void Refresh()
    {
        if (g_depth == 0 || g_closingAll) return;
        Page& p = g_pages[g_depth - 1];
        RebuildRows(p);
        const Shape s = CompareShown(p);
        if (s != kUnchanged) ApplyLive(p, s == kRelabel);
    }

    bool PageOpen() { return g_depth > 0; }

    void AfterClose(void (*fn)(void*), void* user) { g_after = fn; g_afterUser = user; }

    void DropOwner(const char* owner)
    {
        if (!owner) return;
        for (int i = 0; i < kMainRows; ++i)
            if (g_rows[i].held && strcmp(g_rows[i].owner, owner) == 0) g_rows[i].held = false;
    }
}
