#pragma once
// ============================================================================
//  GBApi.generated.h -- Dante VM script API for Ghostbusters: TVG Remastered
//  AUTO-GENERATED -- DO NOT EDIT BY HAND.
//
//    source binary : ghost.exe
//    md5           : 0b89556c07e5b737efe444351227e747
//    functions     : 966 across 114 classes
//
//  Calls go through each function's script thunk using the VM's own calling
//  convention, so virtual dispatch, hidden singletons and by-value returns all
//  work exactly as they do for the game's own scripts. The thunk also null-checks
//  'self' and reports a script error rather than crashing.
// ============================================================================
#include <cstdint>
#include <cstring>
#include "../Structs/Types.h"

extern char* gameBase;

namespace GB {
namespace vm {

// The VM hands every native a single pointer to an array of 8-byte argument
// slots, laid out as [return value][this, if an instance method][parameters].
// Scalars and pointers occupy one slot; a vector occupies three (x, y, z).
struct Block {
    uint64_t s[32];
    Block() { std::memset(s, 0, sizeof s); }
    void set_p(int i, const void* v) { s[i] = reinterpret_cast<uint64_t>(v); }
    void set_b(int i, bool v)        { s[i] = v ? 1u : 0u; }
    void set_i(int i, int v)         { s[i] = static_cast<uint32_t>(v); }
    void set_f(int i, float v)       { uint32_t w; std::memcpy(&w, &v, 4); s[i] = w; }
    void set_v(int i, Vector3 v)     { set_f(i, v.x); set_f(i + 1, v.y); set_f(i + 2, v.z); }
    void*   get_p(int i) const { return reinterpret_cast<void*>(s[i]); }
    bool    get_b(int i) const { return (s[i] & 0xFF) != 0; }
    int     get_i(int i) const { return static_cast<int>(static_cast<uint32_t>(s[i])); }
    float   get_f(int i) const { uint32_t w = static_cast<uint32_t>(s[i]); float f; std::memcpy(&f, &w, 4); return f; }
    Vector3 get_v(int i) const { Vector3 v; v.x = get_f(i); v.y = get_f(i + 1); v.z = get_f(i + 2); return v; }
};

inline void call(uintptr_t thunkRva, Block& b) {
    using Fn = void(__fastcall*)(void*);
    reinterpret_cast<Fn>(gameBase + thunkRva)(b.s);
}

} // namespace vm

// ------------------------------------------------------------ CActor
namespace CActor {
    // void addTeamToAllies(ETeam type)
    //   thunk 0x16CC0  (impl 0x15B40)
    inline void addTeamToAllies(void* self, int type) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, type);
        vm::call(0x16CC0, _gb);
    }
    // @CActor getActorGrabbingMe()
    //   thunk 0x16BE0  (via indirect)
    inline void* getActorGrabbingMe(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x16BE0, _gb);
        return _gb.get_p(0);
    }
    // int getAttachedTetherCount()
    //   thunk 0x16EC0  (via call)
    inline int getAttachedTetherCount(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x16EC0, _gb);
        return _gb.get_i(0);
    }
    // ETeam getCurrentTeam()
    //   thunk 0x16C90  (via none)
    inline int getCurrentTeam(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x16C90, _gb);
        return _gb.get_i(0);
    }
    // bool hasBeenScanned()
    //   thunk 0x16F60  (via call)
    inline bool hasBeenScanned(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x16F60, _gb);
        return _gb.get_b(0);
    }
    // bool isActorGrabbingMe()
    //   thunk 0x16C20  (via call)
    inline bool isActorGrabbingMe(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x16C20, _gb);
        return _gb.get_b(0);
    }
    // int isHideable()
    //   thunk 0x16D20  (via call)
    inline int isHideable(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x16D20, _gb);
        return _gb.get_i(0);
    }
    // bool isPartiallyTethered()
    //   thunk 0x16E80  (via call)
    inline bool isPartiallyTethered(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x16E80, _gb);
        return _gb.get_b(0);
    }
    // bool isSnared()
    //   thunk 0x16D90  (via call)
    inline bool isSnared(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x16D90, _gb);
        return _gb.get_b(0);
    }
    // bool isTethered()
    //   thunk 0x16E00  (via call)
    inline bool isTethered(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x16E00, _gb);
        return _gb.get_b(0);
    }
    // bool isTetheredTo(@CActor who)
    //   thunk 0x16E40  (via call)
    inline bool isTetheredTo(void* self, void* who) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, who);
        vm::call(0x16E40, _gb);
        return _gb.get_b(0);
    }
    // void releaseAllSnaressOnMeFromScript()
    //   thunk 0x16DD0  (impl 0x160F0)
    inline void releaseAllSnaressOnMeFromScript(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x16DD0, _gb);
    }
    // void releaseAllTethersOnMe()
    //   thunk 0x16F00  (impl 0x16700)
    inline void releaseAllTethersOnMe(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x16F00, _gb);
    }
    // void releaseEldestTether()
    //   thunk 0x16F30  (impl 0x167A0)
    inline void releaseEldestTether(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x16F30, _gb);
    }
    // void removeTeamFromAllies(ETeam type)
    //   thunk 0x16CF0  (impl 0x15B70)
    inline void removeTeamFromAllies(void* self, int type) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, type);
        vm::call(0x16CF0, _gb);
    }
    // void setCurrentTeam(ETeam type)
    //   thunk 0x16C60  (impl 0x15B20)
    inline void setCurrentTeam(void* self, int type) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, type);
        vm::call(0x16C60, _gb);
    }
    // void setPossessedStatus(bool flag)
    //   thunk 0x16BB0  (impl 0x15880)
    inline void setPossessedStatus(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x16BB0, _gb);
    }
    // void setScannable(bool flag)
    //   thunk 0x16FA0  (impl 0x16B90)
    inline void setScannable(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x16FA0, _gb);
    }
    // void setVisibleInUVLightOnly(bool flag)
    //   thunk 0x16D60  (impl 0x15ED0)
    inline void setVisibleInUVLightOnly(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x16D60, _gb);
    }
} // namespace CActor

// -------------------------------------------------------- CActorBase
namespace CActorBase {
    // void attachToActor(@CActor attachParent)
    //   thunk 0x2BD780  (via indirect)
    inline void attachToActor(void* self, void* attachParent) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, attachParent);
        vm::call(0x2BD780, _gb);
    }
    // void attachToActorTag(@CActor attachParent, string tagName, bool useCurrentRelativePosition = false)
    //   thunk 0x2BD730  (impl 0x2BEE80)
    inline void attachToActorTag(void* self, void* attachParent, const char* tagName, bool useCurrentRelativePosition = false) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, attachParent);
        _gb.set_p(2, tagName);
        _gb.set_b(3, useCurrentRelativePosition);
        vm::call(0x2BD730, _gb);
    }
    // Vector b2i(Vector point)
    //   thunk 0x2BD7B0  (via call)
    inline Vector3 b2i(void* self, Vector3 point) {
        vm::Block _gb;
        _gb.set_p(3, self);
        _gb.set_v(4, point);
        vm::call(0x2BD7B0, _gb);
        return _gb.get_v(0);
    }
    // Vector b2w(Vector point)
    //   thunk 0x2BD960  (via call)
    inline Vector3 b2w(void* self, Vector3 point) {
        vm::Block _gb;
        _gb.set_p(3, self);
        _gb.set_v(4, point);
        vm::call(0x2BD960, _gb);
        return _gb.get_v(0);
    }
    // @CRoom dante_getRoom()
    //   thunk 0x2BDBD0  (via none)
    inline void* dante_getRoom(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2BDBD0, _gb);
        return _gb.get_p(0);
    }
    // void detachMe()
    //   thunk 0x2BE2F0  (via indirect)
    inline void detachMe(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x2BE2F0, _gb);
    }
    // void enable(bool flag)
    //   thunk 0x2BE320  (impl 0x2DA340)
    inline void enable(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x2BE320, _gb);
    }
    // @CActor getAttachParent()
    //   thunk 0x2BE350  (via none)
    inline void* getAttachParent(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2BE350, _gb);
        return _gb.get_p(0);
    }
    // vector getBodyVelocity()
    //   thunk 0x2BE380  (via call)
    inline Vector3 getBodyVelocity(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x2BE380, _gb);
        return _gb.get_v(0);
    }
    // float getBoundingSphereRadius()
    //   thunk 0x2BE3D0  (via none)
    inline float getBoundingSphereRadius(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2BE3D0, _gb);
        return _gb.get_f(0);
    }
    // vector getBoundsCenter()
    //   thunk 0x2BE400  (via none)
    inline Vector3 getBoundsCenter(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x2BE400, _gb);
        return _gb.get_v(0);
    }
    // @CActor getFirstAttachedChild()
    //   thunk 0x2BE440  (via none)
    inline void* getFirstAttachedChild(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2BE440, _gb);
        return _gb.get_p(0);
    }
    // void getHurt(@SDamageInfo info)
    //   thunk 0x2BDB20  (impl 0x2C0FC0)
    inline void getHurt(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0x2BDB20, _gb);
    }
    // vector getInertialVelocity()
    //   thunk 0x2BE470  (via indirect)
    inline Vector3 getInertialVelocity(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x2BE470, _gb);
        return _gb.get_v(0);
    }
    // string getName()
    //   thunk 0x2BE4C0  (impl 0x2BE531)
    inline const char* getName(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2BE4C0, _gb);
        return static_cast<const char*>(_gb.get_p(0));
    }
    // @CActor getNextAttachedSibling()
    //   thunk 0x2BE540  (via none)
    inline void* getNextAttachedSibling(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2BE540, _gb);
        return _gb.get_p(0);
    }
    // Vector getOrient()
    //   thunk 0x2BDB50  (via none)
    inline Vector3 getOrient(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x2BDB50, _gb);
        return _gb.get_v(0);
    }
    // Vector getPos()
    //   thunk 0x2BDB90  (via none)
    inline Vector3 getPos(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x2BDB90, _gb);
        return _gb.get_v(0);
    }
    // Vector i2b(Vector point)
    //   thunk 0x2BDC10  (via call)
    inline Vector3 i2b(void* self, Vector3 point) {
        vm::Block _gb;
        _gb.set_p(3, self);
        _gb.set_v(4, point);
        vm::call(0x2BDC10, _gb);
        return _gb.get_v(0);
    }
    // Vector i2w(Vector point)
    //   thunk 0x2BDDC0  (via call)
    inline Vector3 i2w(void* self, Vector3 point) {
        vm::Block _gb;
        _gb.set_p(3, self);
        _gb.set_v(4, point);
        vm::call(0x2BDDC0, _gb);
        return _gb.get_v(0);
    }
    // bool isEnabled()
    //   thunk 0x2BE570  (via none)
    inline bool isEnabled(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2BE570, _gb);
        return _gb.get_b(0);
    }
    // bool isHidden()
    //   thunk 0x2BE5A0  (via indirect)
    inline bool isHidden(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2BE5A0, _gb);
        return _gb.get_b(0);
    }
    // bool isOfType(String typeString)
    //   thunk 0x2BDF00  (via call)
    inline bool isOfType(void* self, const char* typeString) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, typeString);
        vm::call(0x2BDF00, _gb);
        return _gb.get_b(0);
    }
    // void positionAttachedActors()
    //   thunk 0x2BE700  (via indirect)
    inline void positionAttachedActors(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x2BE700, _gb);
    }
    // void setProcessRadius(float radius)
    //   thunk 0x2BE760  (via indirect)
    inline void setProcessRadius(void* self, float radius) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, radius);
        vm::call(0x2BE760, _gb);
    }
    // int startSfxTracked(string sampleName)
    //   thunk 0x2BE790  (via indirect)
    inline int startSfxTracked(void* self, const char* sampleName) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, sampleName);
        vm::call(0x2BE790, _gb);
        return _gb.get_i(0);
    }
    // int startTrackedEffect(string effectName)
    //   thunk 0x2BE7D0  (via indirect)
    inline int startTrackedEffect(void* self, const char* effectName) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, effectName);
        vm::call(0x2BE7D0, _gb);
        return _gb.get_i(0);
    }
    // Vector w2b(Vector point)
    //   thunk 0x2BDF40  (via call)
    inline Vector3 w2b(void* self, Vector3 point) {
        vm::Block _gb;
        _gb.set_p(3, self);
        _gb.set_v(4, point);
        vm::call(0x2BDF40, _gb);
        return _gb.get_v(0);
    }
    // Vector w2i(Vector point)
    //   thunk 0x2BE100  (via call)
    inline Vector3 w2i(void* self, Vector3 point) {
        vm::Block _gb;
        _gb.set_p(3, self);
        _gb.set_v(4, point);
        vm::call(0x2BE100, _gb);
        return _gb.get_v(0);
    }
    // void warpTo(Vector pos, Vector orient)
    //   thunk 0x2BE240  (via call)
    inline void warpTo(void* self, Vector3 pos, Vector3 orient) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, pos);
        _gb.set_v(4, orient);
        vm::call(0x2BE240, _gb);
    }
    // void warpToActor(@CActor destActor)
    //   thunk 0x2BE810  (impl 0x2C44B0)
    inline void warpToActor(void* self, void* destActor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, destActor);
        vm::call(0x2BE810, _gb);
    }
    // bool wasVisible()
    //   thunk 0x2BE2C0  (via none)
    inline bool wasVisible(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2BE2C0, _gb);
        return _gb.get_b(0);
    }
    // @CActor whoIsCarryingMe()
    //   thunk 0x2BE840  (via indirect)
    inline void* whoIsCarryingMe(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2BE840, _gb);
        return _gb.get_p(0);
    }
} // namespace CActorBase

// ------------------------------------------------------- CActorGroup
namespace CActorGroup {
    // int add(@CActor actor)
    //   thunk 0x48EA20  (via call)
    inline int add(void* self, void* actor) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, actor);
        vm::call(0x48EA20, _gb);
        return _gb.get_i(0);
    }
    // void constructor()
    //   thunk 0x48EA60  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x48EA60, _gb);
    }
    // void destructor()
    //   thunk 0x48EB00  (via none)
    inline void destructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x48EB00, _gb);
    }
    // int find(@CActor actor)
    //   thunk 0x48EB10  (via none)
    inline int find(void* self, void* actor) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, actor);
        vm::call(0x48EB10, _gb);
        return _gb.get_i(0);
    }
    // @CActor get(int index)
    //   thunk 0x48EA80  (via call)
    inline void* get(void* self, int index) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, index);
        vm::call(0x48EA80, _gb);
        return _gb.get_p(0);
    }
    // int n()
    //   thunk 0x48EB60  (via none)
    inline int n(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x48EB60, _gb);
        return _gb.get_i(0);
    }
    // void removeAll()
    //   thunk 0x48EB80  (via none)
    inline void removeAll(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x48EB80, _gb);
    }
    // void removeByIndex(int index)
    //   thunk 0x48EAD0  (via none)
    inline void removeByIndex(void* self, int index) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, index);
        vm::call(0x48EAD0, _gb);
    }
    // bool removeByPointer(@CActor actor)
    //   thunk 0x48EBA0  (via call)
    inline bool removeByPointer(void* self, void* actor) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, actor);
        vm::call(0x48EBA0, _gb);
        return _gb.get_b(0);
    }
    // void sortByDistanceToPoint(vector point)
    //   thunk 0x48EC20  (via call)
    inline void sortByDistanceToPoint(void* self, Vector3 point) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, point);
        vm::call(0x48EC20, _gb);
    }
} // namespace CActorGroup

// ----------------------------------------------------- CAiSplinePath
namespace CAiSplinePath {
    // bool isOccupied()
    //   thunk 0x196E0  (via call)
    inline bool isOccupied(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x196E0, _gb);
        return _gb.get_b(0);
    }
} // namespace CAiSplinePath

// --------------------------------------------------------- CAniModel
namespace CAniModel {
    // float getFrame()
    //   thunk 0x1CA40  (via call)
    inline float getFrame(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x1CA40, _gb);
        return _gb.get_f(0);
    }
    // float getLastFrame()
    //   thunk 0x1CA80  (via call)
    inline float getLastFrame(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x1CA80, _gb);
        return _gb.get_f(0);
    }
    // bool isPlaying()
    //   thunk 0x1C9D0  (via call)
    inline bool isPlaying(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x1C9D0, _gb);
        return _gb.get_b(0);
    }
    // void play(bool forward = true, EPlayPattern pattern = ePlayPatternNone, float newSpeed = 1.0f, float rampTime = 0.0f)
    //   thunk 0x1C950  (via call)
    inline void play(void* self, bool forward, int pattern, float newSpeed = 1.0f, float rampTime = 0.0f) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, forward);
        _gb.set_i(2, pattern);
        _gb.set_f(3, newSpeed);
        _gb.set_f(4, rampTime);
        vm::call(0x1C950, _gb);
    }
    // void setControlEnable(bool enable)
    //   thunk 0x1CB50  (via indirect)
    inline void setControlEnable(void* self, bool enable) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, enable);
        vm::call(0x1CB50, _gb);
    }
    // void setFrame(float frame)
    //   thunk 0x1CA10  (impl 0x1C4B0)
    inline void setFrame(void* self, float frame) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, frame);
        vm::call(0x1CA10, _gb);
    }
    // void setState(int state)
    //   thunk 0x1CAC0  (impl 0x1C560)
    inline void setState(void* self, int state) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, state);
        vm::call(0x1CAC0, _gb);
    }
    // void setStateWithVel(int state, vector debrisVel)
    //   thunk 0x1CAF0  (via call)
    inline void setStateWithVel(void* self, int state, Vector3 debrisVel) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, state);
        _gb.set_v(2, debrisVel);
        vm::call(0x1CAF0, _gb);
    }
    // void stop(float rampTime = 0.0f)
    //   thunk 0x1C9A0  (impl 0x1C460)
    inline void stop(void* self, float rampTime = 0.0f) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, rampTime);
        vm::call(0x1C9A0, _gb);
    }
} // namespace CAniModel

// -------------------------------------------------------- CArchitect
namespace CArchitect {
    // void flareDown()
    //   thunk 0x24D40  (impl 0x21AE0)
    inline void flareDown(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x24D40, _gb);
    }
    // void flareUp()
    //   thunk 0x24D10  (impl 0x21AD0)
    inline void flareUp(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x24D10, _gb);
    }
    // void lightningStrike()
    //   thunk 0x24E90  (impl 0x21EB0)
    inline void lightningStrike(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x24E90, _gb);
    }
    // void meteorStrike()
    //   thunk 0x24E30  (impl 0x21E30)
    inline void meteorStrike(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x24E30, _gb);
    }
    // void moveLeft()
    //   thunk 0x24D70  (impl 0x21D50)
    inline void moveLeft(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x24D70, _gb);
    }
    // void moveOut()
    //   thunk 0x24E00  (impl 0x21E00)
    inline void moveOut(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x24E00, _gb);
    }
    // void moveRight()
    //   thunk 0x24DA0  (impl 0x21D90)
    inline void moveRight(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x24DA0, _gb);
    }
    // void moveUnder()
    //   thunk 0x24DD0  (impl 0x21DD0)
    inline void moveUnder(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x24DD0, _gb);
    }
    // void vortex()
    //   thunk 0x24E60  (impl 0x21E70)
    inline void vortex(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x24E60, _gb);
    }
} // namespace CArchitect

// --------------------------------------------------------- CAsteroid
namespace CAsteroid {
    // bool isInFlight()
    //   thunk 0x25F80  (via call)
    inline bool isInFlight(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x25F80, _gb);
        return _gb.get_b(0);
    }
    // void setIgnoreDamageUntilLaunched(bool flag = true)
    //   thunk 0x25F50  (impl 0x25F20)
    inline void setIgnoreDamageUntilLaunched(void* self, bool flag = true) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x25F50, _gb);
    }
} // namespace CAsteroid

// ------------------------------------------------------------ CBiped
namespace CBiped {
    // void activateStatue()
    //   thunk 0x37560  (impl 0x33E90)
    inline void activateStatue(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x37560, _gb);
    }
    // void fillInBipedCCSOD(@SBipedCombatComponentInfo info)
    //   thunk 0x37660  (impl 0x37030)
    inline void fillInBipedCCSOD(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0x37660, _gb);
    }
    // bool hasShieldReady()
    //   thunk 0x37620  (via call)
    inline bool hasShieldReady(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x37620, _gb);
        return _gb.get_b(0);
    }
    // void setAsAmbusher()
    //   thunk 0x37590  (impl 0x34B30)
    inline void setAsAmbusher(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x37590, _gb);
    }
    // void setAsAmbusherGroup()
    //   thunk 0x375C0  (impl 0x34BD0)
    inline void setAsAmbusherGroup(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x375C0, _gb);
    }
    // void setCowerMode(bool flag)
    //   thunk 0x375F0  (impl 0x34C60)
    inline void setCowerMode(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x375F0, _gb);
    }
} // namespace CBiped

// ------------------------------------------------------- CBipedLarge
namespace CBipedLarge {
    // void activateSpecialCaseVisualCrap(bool flag)
    //   thunk 0x3D860  (impl 0x3AF20)
    inline void activateSpecialCaseVisualCrap(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x3D860, _gb);
    }
    // void fillInBipedLargeCCSOD(@SBipedLargeCombatComponentInfo info)
    //   thunk 0x3D890  (impl 0x3D5B0)
    inline void fillInBipedLargeCCSOD(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0x3D890, _gb);
    }
} // namespace CBipedLarge

// -------------------------------------------------------- CBirdFlock
namespace CBirdFlock {
    // vector getFlyToPos()
    //   thunk 0x41250  (via call)
    inline Vector3 getFlyToPos(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x41250, _gb);
        return _gb.get_v(0);
    }
    // void leaveGround()
    //   thunk 0x41220  (impl 0x40B20)
    inline void leaveGround(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x41220, _gb);
    }
    // void setFlyToPos(vector pos)
    //   thunk 0x412A0  (via call)
    inline void setFlyToPos(void* self, Vector3 pos) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, pos);
        vm::call(0x412A0, _gb);
    }
} // namespace CBirdFlock

// ------------------------------------------------------- CBlackSlime
namespace CBlackSlime {
    // void closePortal()
    //   thunk 0x43430  (impl 0x42610)
    inline void closePortal(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x43430, _gb);
    }
    // void openPortal()
    //   thunk 0x43400  (impl 0x42550)
    inline void openPortal(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x43400, _gb);
    }
    // void reset()
    //   thunk 0x433D0  (impl 0x42510)
    inline void reset(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x433D0, _gb);
    }
    // void toggleAIObstacle(bool enable)
    //   thunk 0x43460  (impl 0x43040)
    inline void toggleAIObstacle(void* self, bool enable) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, enable);
        vm::call(0x43460, _gb);
    }
} // namespace CBlackSlime

// --------------------------------------------------------- CBlinkers
namespace CBlinkers {
    // void attackLeft(@CWayPoint wpStart)
    //   thunk 0x46130  (impl 0x45F60)
    inline void attackLeft(void* self, void* wpStart) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, wpStart);
        vm::call(0x46130, _gb);
    }
    // void attackRight(@CWayPoint wpStart)
    //   thunk 0x46160  (impl 0x45FD0)
    inline void attackRight(void* self, void* wpStart) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, wpStart);
        vm::call(0x46160, _gb);
    }
    // void moveLeft(@CWayPoint wpStart, @CWayPoint wpEnd)
    //   thunk 0x46190  (impl 0x46040)
    inline void moveLeft(void* self, void* wpStart, void* wpEnd) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, wpStart);
        _gb.set_p(2, wpEnd);
        vm::call(0x46190, _gb);
    }
    // void moveRight(@CWayPoint wpStart, @CWayPoint wpEnd)
    //   thunk 0x461C0  (impl 0x46060)
    inline void moveRight(void* self, void* wpStart, void* wpEnd) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, wpStart);
        _gb.set_p(2, wpEnd);
        vm::call(0x461C0, _gb);
    }
    // void moveUnder(@CWayPoint wpStart, @CWayPoint wpEnd)
    //   thunk 0x461F0  (impl 0x46080)
    inline void moveUnder(void* self, void* wpStart, void* wpEnd) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, wpStart);
        _gb.set_p(2, wpEnd);
        vm::call(0x461F0, _gb);
    }
    // void stopSummon()
    //   thunk 0x46250  (impl 0x46110)
    inline void stopSummon(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x46250, _gb);
    }
    // void summon(int count)
    //   thunk 0x46220  (impl 0x460D0)
    inline void summon(void* self, int count) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, count);
        vm::call(0x46220, _gb);
    }
} // namespace CBlinkers

// ------------------------------------------------- CBoneSimActorBase
namespace CBoneSimActorBase {
    // void followCameraPath(int boneIndex)
    //   thunk 0x3AE8E0  (impl 0x3AEC60)
    inline void followCameraPath(void* self, int boneIndex) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, boneIndex);
        vm::call(0x3AE8E0, _gb);
    }
    // void setMaxWind(vector windVelocity)
    //   thunk 0x3AE910  (via none)
    inline void setMaxWind(void* self, Vector3 windVelocity) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, windVelocity);
        vm::call(0x3AE910, _gb);
    }
    // void setMinWind(vector windVelocity)
    //   thunk 0x3AE950  (via none)
    inline void setMinWind(void* self, Vector3 windVelocity) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, windVelocity);
        vm::call(0x3AE950, _gb);
    }
    // void stopFollowingCameraPath()
    //   thunk 0x3AE990  (via none)
    inline void stopFollowingCameraPath(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x3AE990, _gb);
    }
} // namespace CBoneSimActorBase

// -------------------------------------------------------- CBookStack
namespace CBookStack {
    // void ejectBooks(bool reverseDirection)
    //   thunk 0x4D3A0  (impl 0x4B960)
    inline void ejectBooks(void* self, bool reverseDirection) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, reverseDirection);
        vm::call(0x4D3A0, _gb);
    }
    // void followCameraPath()
    //   thunk 0x4D430  (impl 0x4BCF0)
    inline void followCameraPath(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x4D430, _gb);
    }
    // void shiftBooks(float dir)
    //   thunk 0x4D400  (impl 0x4BB80)
    inline void shiftBooks(void* self, float dir) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, dir);
        vm::call(0x4D400, _gb);
    }
    // void shootOutBooks()
    //   thunk 0x4D3D0  (impl 0x4B9B0)
    inline void shootOutBooks(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x4D3D0, _gb);
    }
} // namespace CBookStack

// -------------------------------------------------------- CBookshelf
namespace CBookshelf {
    // void knockMeOver()
    //   thunk 0x48D70  (impl 0x47D80)
    inline void knockMeOver(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x48D70, _gb);
    }
    // void moveForward(float distance)
    //   thunk 0x48DA0  (impl 0x47F90)
    inline void moveForward(void* self, float distance) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, distance);
        vm::call(0x48DA0, _gb);
    }
} // namespace CBookshelf

// --------------------------------------------------------- CBoxActor
namespace CBoxActor {
    // void activate(bool flag)
    //   thunk 0x355460  (impl 0x3592D0)
    inline void activate(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x355460, _gb);
    }
    // void addForceFromActor(float force, @CActor actor)
    //   thunk 0x355430  (impl 0x355730)
    inline void addForceFromActor(void* self, float force, void* actor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, force);
        _gb.set_p(2, actor);
        vm::call(0x355430, _gb);
    }
    // void addForceFromActorAtPos(float force, @CActor actor)
    //   thunk 0x355400  (impl 0x3557F0)
    inline void addForceFromActorAtPos(void* self, float force, void* actor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, force);
        _gb.set_p(2, actor);
        vm::call(0x355400, _gb);
    }
    // void addVelocity(Vector velocity)
    //   thunk 0x3554D0  (via indirect)
    inline void addVelocity(void* self, Vector3 velocity) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, velocity);
        vm::call(0x3554D0, _gb);
    }
    // void createJoint()
    //   thunk 0x355530  (via indirect)
    inline void createJoint(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x355530, _gb);
    }
    // void enableAnchor(bool enable)
    //   thunk 0x355560  (impl 0x356FD0)
    inline void enableAnchor(void* self, bool enable) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, enable);
        vm::call(0x355560, _gb);
    }
    // void immobilize()
    //   thunk 0x355590  (via indirect)
    inline void immobilize(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x355590, _gb);
    }
    // void mobilize()
    //   thunk 0x3555C0  (via indirect)
    inline void mobilize(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x3555C0, _gb);
    }
    // void removeAnchor()
    //   thunk 0x3555F0  (via call)
    inline void removeAnchor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x3555F0, _gb);
    }
    // void removeJoint(@Joint joint)
    //   thunk 0x355660  (via indirect)
    inline void removeJoint(void* self, void* joint) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, joint);
        vm::call(0x355660, _gb);
    }
    // void setVelocity(Vector velocity)
    //   thunk 0x355690  (via indirect)
    inline void setVelocity(void* self, Vector3 velocity) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, velocity);
        vm::call(0x355690, _gb);
    }
} // namespace CBoxActor

// ---------------------------------------------------------- CBoxTest
namespace CBoxTest {
    // void messWithGravity()
    //   thunk 0x494840  (via none)
    inline void messWithGravity(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x494840, _gb);
    }
    // void messWithGravity2()
    //   thunk 0x4947E0  (via none)
    inline void messWithGravity2(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x4947E0, _gb);
    }
    // void messWithGravity3()
    //   thunk 0x494810  (via none)
    inline void messWithGravity3(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x494810, _gb);
    }
} // namespace CBoxTest

// -------------------------------------------------------- CBreakable
namespace CBreakable {
    // float getHitPoints()
    //   thunk 0x51010  (via call)
    inline float getHitPoints(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x51010, _gb);
        return _gb.get_f(0);
    }
    // float getMaxHitPoints()
    //   thunk 0x51050  (via call)
    inline float getMaxHitPoints(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x51050, _gb);
        return _gb.get_f(0);
    }
    // void setHitPoints(float newHitPoints)
    //   thunk 0x51090  (impl 0x50FF0)
    inline void setHitPoints(void* self, float newHitPoints) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, newHitPoints);
        vm::call(0x51090, _gb);
    }
} // namespace CBreakable

// ---------------------------------------------------------- CBreaker
namespace CBreaker {
    // void dante_getHurt(@SDamageInfo info)
    //   thunk 0x3B18A0  (impl 0x3B2B00)
    inline void dante_getHurt(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0x3B18A0, _gb);
    }
    // void die()
    //   thunk 0x3B18E0  (impl 0x3B27B0)
    inline void die(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x3B18E0, _gb);
    }
    // void enable(bool flag)
    //   thunk 0x3B1BC0  (impl 0x3B5910)
    inline void enable(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x3B1BC0, _gb);
    }
    // void fixIt()
    //   thunk 0x3B1B40  (impl 0x3B5910)
    inline void fixIt(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x3B1B40, _gb);
    }
    // float getCurrentDamage()
    //   thunk 0x3B1900  (via none)
    inline float getCurrentDamage(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x3B1900, _gb);
        return _gb.get_f(0);
    }
    // float getDistanceFromActor(@CActor a)
    //   thunk 0x3B1930  (via call)
    inline float getDistanceFromActor(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, a);
        vm::call(0x3B1930, _gb);
        return _gb.get_f(0);
    }
    // float getHealth()
    //   thunk 0x3B19B0  (via none)
    inline float getHealth(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x3B19B0, _gb);
        return _gb.get_f(0);
    }
    // float getHealthPct()
    //   thunk 0x3B1970  (via none)
    inline float getHealthPct(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x3B1970, _gb);
        return _gb.get_f(0);
    }
    // string getName()
    //   thunk 0x3B1A10  (impl 0x3B1A7D)
    inline const char* getName(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x3B1A10, _gb);
        return static_cast<const char*>(_gb.get_p(0));
    }
    // vector getOrient()
    //   thunk 0x3B1A90  (via none)
    inline Vector3 getOrient(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x3B1A90, _gb);
        return _gb.get_v(0);
    }
    // vector getPosAtCenter()
    //   thunk 0x3B1AD0  (via none)
    inline Vector3 getPosAtCenter(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x3B1AD0, _gb);
        return _gb.get_v(0);
    }
    // bool isEnabled()
    //   thunk 0x3B19E0  (via none)
    inline bool isEnabled(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x3B19E0, _gb);
        return _gb.get_b(0);
    }
    // void setCurrentDamage(float amount)
    //   thunk 0x3B1B80  (via call)
    inline void setCurrentDamage(void* self, float amount) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, amount);
        vm::call(0x3B1B80, _gb);
    }
} // namespace CBreaker

// ------------------------------------------------- CBurningBushActor
namespace CBurningBushActor {
    // void ignite()
    //   thunk 0x52C70  (impl 0x52880)
    inline void ignite(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x52C70, _gb);
    }
} // namespace CBurningBushActor

// ------------------------------------------------------------ CCable
namespace CCable {
    // void detachEndpoint(ECableEndpoint whichEnd)
    //   thunk 0x579D0  (impl 0x56690)
    inline void detachEndpoint(void* self, int whichEnd) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, whichEnd);
        vm::call(0x579D0, _gb);
    }
    // vector getEndpointWPos(ECableEndpoint whichEnd)
    //   thunk 0x57A00  (via call)
    inline Vector3 getEndpointWPos(void* self, int whichEnd) {
        vm::Block _gb;
        _gb.set_p(3, self);
        _gb.set_i(4, whichEnd);
        vm::call(0x57A00, _gb);
        return _gb.get_v(0);
    }
} // namespace CCable

// -------------------------------------------------- CCameraPathActor
namespace CCameraPathActor {
    // void addFollower(@CActor child)
    //   thunk 0x347DA0  (via indirect)
    inline void addFollower(void* self, void* child) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, child);
        vm::call(0x347DA0, _gb);
    }
    // float getFollowerKey(@CActor child)
    //   thunk 0x347DF0  (via call)
    inline float getFollowerKey(void* self, void* child) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, child);
        vm::call(0x347DF0, _gb);
        return _gb.get_f(0);
    }
    // float getFollowerSpeed(@CActor child)
    //   thunk 0x347E30  (via none)
    inline float getFollowerSpeed(void* self, void* child) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, child);
        vm::call(0x347E30, _gb);
        return _gb.get_f(0);
    }
    // float getFollowerTime(@CActor child)
    //   thunk 0x347EA0  (via none)
    inline float getFollowerTime(void* self, void* child) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, child);
        vm::call(0x347EA0, _gb);
        return _gb.get_f(0);
    }
    // bool hasFollower(@CActor a)
    //   thunk 0x347F10  (via none)
    inline bool hasFollower(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, a);
        vm::call(0x347F10, _gb);
        return _gb.get_b(0);
    }
    // bool isFollowerPaused(@CActor child)
    //   thunk 0x347F60  (via none)
    inline bool isFollowerPaused(void* self, void* child) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, child);
        vm::call(0x347F60, _gb);
        return _gb.get_b(0);
    }
    // void moveFollowerToEnd(@CActor child)
    //   thunk 0x347FE0  (impl 0x34BDC0)
    inline void moveFollowerToEnd(void* self, void* child) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, child);
        vm::call(0x347FE0, _gb);
    }
    // void moveFollowerToKey(@CActor child, float key)
    //   thunk 0x348010  (impl 0x34BE50)
    inline void moveFollowerToKey(void* self, void* child, float key) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, child);
        _gb.set_f(2, key);
        vm::call(0x348010, _gb);
    }
    // void moveFollowerToNextKey(@CActor child)
    //   thunk 0x348040  (impl 0x34BF80)
    inline void moveFollowerToNextKey(void* self, void* child) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, child);
        vm::call(0x348040, _gb);
    }
    // void moveFollowerToPrevKey(@CActor child)
    //   thunk 0x348070  (impl 0x34C000)
    inline void moveFollowerToPrevKey(void* self, void* child) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, child);
        vm::call(0x348070, _gb);
    }
    // void moveFollowerToStart(@CActor child)
    //   thunk 0x3480A0  (via none)
    inline void moveFollowerToStart(void* self, void* child) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, child);
        vm::call(0x3480A0, _gb);
    }
    // void pauseFollower(@CActor child)
    //   thunk 0x348100  (via none)
    inline void pauseFollower(void* self, void* child) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, child);
        vm::call(0x348100, _gb);
    }
    // void removeFollower(@CActor child)
    //   thunk 0x348160  (via indirect)
    inline void removeFollower(void* self, void* child) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, child);
        vm::call(0x348160, _gb);
    }
    // void resumeFollower(@CActor child)
    //   thunk 0x3481B0  (via none)
    inline void resumeFollower(void* self, void* child) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, child);
        vm::call(0x3481B0, _gb);
    }
    // void setFollowerSpeed(@CActor child, float speed)
    //   thunk 0x348210  (via none)
    inline void setFollowerSpeed(void* self, void* child, float speed) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, child);
        _gb.set_f(2, speed);
        vm::call(0x348210, _gb);
    }
    // void setFollowerTime(@CActor child, float time)
    //   thunk 0x348270  (impl 0x34D240)
    inline void setFollowerTime(void* self, void* child, float time) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, child);
        _gb.set_f(2, time);
        vm::call(0x348270, _gb);
    }
} // namespace CCameraPathActor

// ------------------------------------------------------- CCarEffects
namespace CCarEffects {
    // void NotifyWhenNearActor(@CActor actor, float nearEnuf)
    //   thunk 0x6A530  (impl 0x68BB0)
    inline void NotifyWhenNearActor(void* self, void* actor, float nearEnuf) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, actor);
        _gb.set_f(2, nearEnuf);
        vm::call(0x6A530, _gb);
    }
    // void NotifyWhenNearPosition(vector wPos, float nearEnuf)
    //   thunk 0x6A4D0  (via call)
    inline void NotifyWhenNearPosition(void* self, Vector3 wPos, float nearEnuf) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wPos);
        _gb.set_f(4, nearEnuf);
        vm::call(0x6A4D0, _gb);
    }
    // void addForceToChassis(vector wPos, vector iForce, float frequency = 1.0f)
    //   thunk 0x6A7F0  (via call)
    inline void addForceToChassis(void* self, Vector3 wPos, Vector3 iForce, float frequency = 1.0f) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wPos);
        _gb.set_v(4, iForce);
        _gb.set_f(7, frequency);
        vm::call(0x6A7F0, _gb);
    }
    // void closeDoor(int doorIndex)
    //   thunk 0x699E0  (impl 0x67FE0)
    inline void closeDoor(void* self, int doorIndex) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, doorIndex);
        vm::call(0x699E0, _gb);
    }
    // void damageChassis(float damage)
    //   thunk 0x697D0  (impl 0x67940)
    inline void damageChassis(void* self, float damage) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, damage);
        vm::call(0x697D0, _gb);
    }
    // void damageDoor(int doorIndex, float damage)
    //   thunk 0x69830  (impl 0x67BC0)
    inline void damageDoor(void* self, int doorIndex, float damage) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, doorIndex);
        _gb.set_f(2, damage);
        vm::call(0x69830, _gb);
    }
    // void damageWheel(int wheelIndex, float damage)
    //   thunk 0x69800  (impl 0x67A50)
    inline void damageWheel(void* self, int wheelIndex, float damage) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, wheelIndex);
        _gb.set_f(2, damage);
        vm::call(0x69800, _gb);
    }
    // void detachAllDoorsFromChassis()
    //   thunk 0x69680  (impl 0x677D0)
    inline void detachAllDoorsFromChassis(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x69680, _gb);
    }
    // void detachAllWheelsFromChassis()
    //   thunk 0x69650  (impl 0x67790)
    inline void detachAllWheelsFromChassis(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x69650, _gb);
    }
    // void detachAllWheelsFromSet()
    //   thunk 0x69620  (impl 0x67760)
    inline void detachAllWheelsFromSet(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x69620, _gb);
    }
    // void detachDoorFromChassis(int doorIndex)
    //   thunk 0x695F0  (impl 0x67740)
    inline void detachDoorFromChassis(void* self, int doorIndex) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, doorIndex);
        vm::call(0x695F0, _gb);
    }
    // void detachFromCameraPath()
    //   thunk 0x69F60  (impl 0x68600)
    inline void detachFromCameraPath(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x69F60, _gb);
    }
    // void detachWheelFromChassis(int wheelIndex)
    //   thunk 0x695C0  (impl 0x67720)
    inline void detachWheelFromChassis(void* self, int wheelIndex) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, wheelIndex);
        vm::call(0x695C0, _gb);
    }
    // void detachWheelFromSet(int wheelIndex)
    //   thunk 0x695A0  (via none)
    inline void detachWheelFromSet(int wheelIndex) {
        vm::Block _gb;
        _gb.set_i(0, wheelIndex);
        vm::call(0x695A0, _gb);
    }
    // void detonate(float timer)
    //   thunk 0x6A8C0  (impl 0x690F0)
    inline void detonate(void* self, float timer) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, timer);
        vm::call(0x6A8C0, _gb);
    }
    // void enableAllLights(bool yah)
    //   thunk 0x6A690  (impl 0x68CD0)
    inline void enableAllLights(void* self, bool yah) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        vm::call(0x6A690, _gb);
    }
    // void enableAutoDetonation(bool yah)
    //   thunk 0x6A660  (impl 0x68CA0)
    inline void enableAutoDetonation(void* self, bool yah) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        vm::call(0x6A660, _gb);
    }
    // void enableCameraPathStops(bool yah, float relativeDistance = 0.1f)
    //   thunk 0x6A310  (impl 0x68A40)
    inline void enableCameraPathStops(void* self, bool yah, float relativeDistance = 0.1f) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        _gb.set_f(2, relativeDistance);
        vm::call(0x6A310, _gb);
    }
    // void enableExplosiveFuelTank(bool yah)
    //   thunk 0x6A630  (impl 0x68C30)
    inline void enableExplosiveFuelTank(void* self, bool yah) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        vm::call(0x6A630, _gb);
    }
    // void enableHeadLights(bool yah)
    //   thunk 0x6A6C0  (impl 0x68D70)
    inline void enableHeadLights(void* self, bool yah) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        vm::call(0x6A6C0, _gb);
    }
    // void enableTailLights(bool yah)
    //   thunk 0x6A6F0  (impl 0x68E00)
    inline void enableTailLights(void* self, bool yah) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        vm::call(0x6A6F0, _gb);
    }
    // int getAttachedTetherCount()
    //   thunk 0x6A7B0  (via call)
    inline int getAttachedTetherCount(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x6A7B0, _gb);
        return _gb.get_i(0);
    }
    // float getCameraPathTime()
    //   thunk 0x6A2D0  (via call)
    inline float getCameraPathTime(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x6A2D0, _gb);
        return _gb.get_f(0);
    }
    // @CActor getChassisActor()
    //   thunk 0x69C90  (via call)
    inline void* getChassisActor(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x69C90, _gb);
        return _gb.get_p(0);
    }
    // float getDistanceToCameraPathTime(float time)
    //   thunk 0x6A1C0  (via call)
    inline float getDistanceToCameraPathTime(void* self, float time) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_f(2, time);
        vm::call(0x6A1C0, _gb);
        return _gb.get_f(0);
    }
    // float getFuelTankHealth()
    //   thunk 0x6A5F0  (via call)
    inline float getFuelTankHealth(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x6A5F0, _gb);
        return _gb.get_f(0);
    }
    // float getMotiveAcceleration()
    //   thunk 0x69B70  (via call)
    inline float getMotiveAcceleration(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x69B70, _gb);
        return _gb.get_f(0);
    }
    // float getMotiveSpeed()
    //   thunk 0x69BE0  (via call)
    inline float getMotiveSpeed(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x69BE0, _gb);
        return _gb.get_f(0);
    }
    // @CActor getMoveToActor()
    //   thunk 0x6A490  (via call)
    inline void* getMoveToActor(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x6A490, _gb);
        return _gb.get_p(0);
    }
    // vector getMoveToPosition()
    //   thunk 0x6A440  (via call)
    inline Vector3 getMoveToPosition(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x6A440, _gb);
        return _gb.get_v(0);
    }
    // @CActor getNotifyWhenNearActor()
    //   thunk 0x6A5B0  (via call)
    inline void* getNotifyWhenNearActor(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x6A5B0, _gb);
        return _gb.get_p(0);
    }
    // vector getNotifyWhenNearPosition()
    //   thunk 0x6A560  (via call)
    inline Vector3 getNotifyWhenNearPosition(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x6A560, _gb);
        return _gb.get_v(0);
    }
    // vector getPositionAtCameraPathTime(float time)
    //   thunk 0x6A200  (via call)
    inline Vector3 getPositionAtCameraPathTime(void* self, float time) {
        vm::Block _gb;
        _gb.set_p(3, self);
        _gb.set_f(4, time);
        vm::call(0x6A200, _gb);
        return _gb.get_v(0);
    }
    // float getSteer()
    //   thunk 0x69C50  (via call)
    inline float getSteer(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x69C50, _gb);
        return _gb.get_f(0);
    }
    // float getTimeNearCameraPathPosition(vector wPos)
    //   thunk 0x6A260  (via call)
    inline float getTimeNearCameraPathPosition(void* self, Vector3 wPos) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_v(2, wPos);
        vm::call(0x6A260, _gb);
        return _gb.get_f(0);
    }
    // float getTimeToCameraPathDistance(float distance)
    //   thunk 0x6A180  (via call)
    inline float getTimeToCameraPathDistance(void* self, float distance) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_f(2, distance);
        vm::call(0x6A180, _gb);
        return _gb.get_f(0);
    }
    // @CActor getWheelActor(int wheelIndex)
    //   thunk 0x69CD0  (via call)
    inline void* getWheelActor(void* self, int wheelIndex) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, wheelIndex);
        vm::call(0x69CD0, _gb);
        return _gb.get_p(0);
    }
    // bool isChassisWeldedToSet()
    //   thunk 0x69AD0  (via call)
    inline bool isChassisWeldedToSet(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x69AD0, _gb);
        return _gb.get_b(0);
    }
    // int isMoveTo()
    //   thunk 0x6A400  (via call)
    inline int isMoveTo(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x6A400, _gb);
        return _gb.get_i(0);
    }
    // void launchAtTarget(vector wTgtPos, float flightTime)
    //   thunk 0x69890  (via call)
    inline void launchAtTarget(void* self, Vector3 wTgtPos, float flightTime) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wTgtPos);
        _gb.set_f(4, flightTime);
        vm::call(0x69890, _gb);
    }
    // void launchDoorAtTarget(int doorIndex, vector wTgtPos, float flightTime)
    //   thunk 0x698F0  (via call)
    inline void launchDoorAtTarget(void* self, int doorIndex, Vector3 wTgtPos, float flightTime) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, doorIndex);
        _gb.set_v(2, wTgtPos);
        _gb.set_f(5, flightTime);
        vm::call(0x698F0, _gb);
    }
    // void launchWheelAtTarget(int wheelIndex, vector wTgtPos, float flightTime)
    //   thunk 0x69950  (via call)
    inline void launchWheelAtTarget(void* self, int wheelIndex, Vector3 wTgtPos, float flightTime) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, wheelIndex);
        _gb.set_v(2, wTgtPos);
        _gb.set_f(5, flightTime);
        vm::call(0x69950, _gb);
    }
    // void lockChassisInPlace(bool whenAtRest = true)
    //   thunk 0x69A10  (impl 0x68000)
    inline void lockChassisInPlace(void* self, bool whenAtRest = true) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, whenAtRest);
        vm::call(0x69A10, _gb);
    }
    // void lockChassisInPlaceWhenAtRest()
    //   thunk 0x69A40  (impl 0x68020)
    inline void lockChassisInPlaceWhenAtRest(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x69A40, _gb);
    }
    // void lockChassisInPlaceWhenOnGround()
    //   thunk 0x69A70  (impl 0x68040)
    inline void lockChassisInPlaceWhenOnGround(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x69A70, _gb);
    }
    // void moveCancel()
    //   thunk 0x6A340  (impl 0x68AB0)
    inline void moveCancel(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x6A340, _gb);
    }
    // void moveToActor(@CActor actor)
    //   thunk 0x6A3D0  (impl 0x68B00)
    inline void moveToActor(void* self, void* actor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, actor);
        vm::call(0x6A3D0, _gb);
    }
    // void moveToPosition(vector wPos)
    //   thunk 0x6A370  (via call)
    inline void moveToPosition(void* self, Vector3 wPos) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wPos);
        vm::call(0x6A370, _gb);
    }
    // void openDoor(int doorIndex)
    //   thunk 0x699B0  (impl 0x67FC0)
    inline void openDoor(void* self, int doorIndex) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, doorIndex);
        vm::call(0x699B0, _gb);
    }
    // void reassemble()
    //   thunk 0x69B10  (impl 0x68160)
    inline void reassemble(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x69B10, _gb);
    }
    // void reattachToCameraPath()
    //   thunk 0x69F90  (impl 0x68690)
    inline void reattachToCameraPath(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x69F90, _gb);
    }
    // void setCameraPathEventDistance(float distance, float tol = 1.0f)
    //   thunk 0x6A0C0  (impl 0x688E0)
    inline void setCameraPathEventDistance(void* self, float distance, float tol = 1.0f) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, distance);
        _gb.set_f(2, tol);
        vm::call(0x6A0C0, _gb);
    }
    // void setCameraPathEventTime(float timeTag)
    //   thunk 0x6A090  (impl 0x688B0)
    inline void setCameraPathEventTime(void* self, float timeTag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, timeTag);
        vm::call(0x6A090, _gb);
    }
    // void setCameraPathHeadingModeToAligned()
    //   thunk 0x69F00  (via none)
    inline void setCameraPathHeadingModeToAligned(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x69F00, _gb);
    }
    // void setCameraPathHeadingModeToFree()
    //   thunk 0x69F40  (via none)
    inline void setCameraPathHeadingModeToFree(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x69F40, _gb);
    }
    // void setCameraPathHeadingModeToPath()
    //   thunk 0x69F20  (via none)
    inline void setCameraPathHeadingModeToPath(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x69F20, _gb);
    }
    // void setCameraPathMaxAltitude(float maxAlt)
    //   thunk 0x6A0F0  (impl 0x68900)
    inline void setCameraPathMaxAltitude(void* self, float maxAlt) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, maxAlt);
        vm::call(0x6A0F0, _gb);
    }
    // void setCameraPathMaxBankAngle(float maxDeg)
    //   thunk 0x6A120  (impl 0x68910)
    inline void setCameraPathMaxBankAngle(void* self, float maxDeg) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, maxDeg);
        vm::call(0x6A120, _gb);
    }
    // void setCameraPathMaxPitchAngle(float maxDeg)
    //   thunk 0x6A150  (impl 0x68930)
    inline void setCameraPathMaxPitchAngle(void* self, float maxDeg) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, maxDeg);
        vm::call(0x6A150, _gb);
    }
    // void setCameraPathTimeFactor(float pct)
    //   thunk 0x69EC0  (via none)
    inline void setCameraPathTimeFactor(float pct) {
        vm::Block _gb;
        _gb.set_f(0, pct);
        vm::call(0x69EC0, _gb);
    }
    // void setCameraPathTimeFactorRate(float rate)
    //   thunk 0x69EE0  (via none)
    inline void setCameraPathTimeFactorRate(float rate) {
        vm::Block _gb;
        _gb.set_f(0, rate);
        vm::call(0x69EE0, _gb);
    }
    // void setChargeAtChassis(vector iAccel, float duration)
    //   thunk 0x69770  (via call)
    inline void setChargeAtChassis(void* self, Vector3 iAccel, float duration) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, iAccel);
        _gb.set_f(4, duration);
        vm::call(0x69770, _gb);
    }
    // void setChargeAtDoor(int doorIndex, vector iAccel, float duration)
    //   thunk 0x696B0  (via call)
    inline void setChargeAtDoor(void* self, int doorIndex, Vector3 iAccel, float duration) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, doorIndex);
        _gb.set_v(2, iAccel);
        _gb.set_f(5, duration);
        vm::call(0x696B0, _gb);
    }
    // void setChargeAtWheel(int wheelIndex, vector iAccel, float duration)
    //   thunk 0x69710  (via call)
    inline void setChargeAtWheel(void* self, int wheelIndex, Vector3 iAccel, float duration) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, wheelIndex);
        _gb.set_v(2, iAccel);
        _gb.set_f(5, duration);
        vm::call(0x69710, _gb);
    }
    // void setChassisHeightContraint(float ht)
    //   thunk 0x6A890  (impl 0x690A0)
    inline void setChassisHeightContraint(void* self, float ht) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, ht);
        vm::call(0x6A890, _gb);
    }
    // void setFastCar(bool yah)
    //   thunk 0x6A780  (impl 0x68ED0)
    inline void setFastCar(void* self, bool yah) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        vm::call(0x6A780, _gb);
    }
    // void setInvincible(bool yah)
    //   thunk 0x6A750  (impl 0x68EB0)
    inline void setInvincible(void* self, bool yah) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        vm::call(0x6A750, _gb);
    }
    // void setMotiveAcceleration(float acceleration, float deceleration = 0.0f)
    //   thunk 0x69B40  (impl 0x68250)
    inline void setMotiveAcceleration(void* self, float acceleration, float deceleration = 0.0f) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, acceleration);
        _gb.set_f(2, deceleration);
        vm::call(0x69B40, _gb);
    }
    // void setMotiveSpeed(float ft)
    //   thunk 0x69BB0  (impl 0x682A0)
    inline void setMotiveSpeed(void* self, float ft) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, ft);
        vm::call(0x69BB0, _gb);
    }
    // void setSteer(float s)
    //   thunk 0x69C20  (impl 0x68340)
    inline void setSteer(void* self, float s) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, s);
        vm::call(0x69C20, _gb);
    }
    // void setTetherable(bool yah)
    //   thunk 0x6A720  (impl 0x68EA0)
    inline void setTetherable(void* self, bool yah) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        vm::call(0x6A720, _gb);
    }
    // void shake(float factor = 1.0f)
    //   thunk 0x6A860  (impl 0x69010)
    inline void shake(void* self, float factor = 1.0f) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, factor);
        vm::call(0x6A860, _gb);
    }
    // void switchCameraPath(@CCameraPathActor path, float timeTag=0.0)
    //   thunk 0x69E90  (impl 0x684C0)
    inline void switchCameraPath(void* self, void* path, float timeTag = 0.0) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, path);
        _gb.set_f(2, timeTag);
        vm::call(0x69E90, _gb);
    }
    // void synchronizeSteeringWithCameraPath(bool yah)
    //   thunk 0x69FC0  (via none)
    inline void synchronizeSteeringWithCameraPath(bool yah) {
        vm::Block _gb;
        _gb.set_b(0, yah);
        vm::call(0x69FC0, _gb);
    }
    // void synchronizeWheelSpeedWithCameraPath(int mode)
    //   thunk 0x69FE0  (via none)
    inline void synchronizeWheelSpeedWithCameraPath(int mode) {
        vm::Block _gb;
        _gb.set_i(0, mode);
        vm::call(0x69FE0, _gb);
    }
    // void wakeUp()
    //   thunk 0x69860  (impl 0x67D30)
    inline void wakeUp(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x69860, _gb);
    }
    // void warpToCameraPathAtTime(float timeTag)
    //   thunk 0x6A060  (impl 0x687D0)
    inline void warpToCameraPathAtTime(void* self, float timeTag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, timeTag);
        vm::call(0x6A060, _gb);
    }
    // void warpToCameraPathNearPosition(vector wPos)
    //   thunk 0x6A000  (via call)
    inline void warpToCameraPathNearPosition(void* self, Vector3 wPos) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wPos);
        vm::call(0x6A000, _gb);
    }
    // void weldChassisToSet(bool yah)
    //   thunk 0x69AA0  (impl 0x68070)
    inline void weldChassisToSet(void* self, bool yah) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        vm::call(0x69AA0, _gb);
    }
} // namespace CCarEffects

// ------------------------------------------------------ CCardCatalog
namespace CCardCatalog {
    // void shootDrawer(int which)
    //   thunk 0x5B590  (impl 0x5B320)
    inline void shootDrawer(void* self, int which) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, which);
        vm::call(0x5B590, _gb);
    }
} // namespace CCardCatalog

// -------------------------------------------------------- CCharacter
namespace CCharacter {
    // void beginWalkTo(@SScriptWalkInfo info)
    //   thunk 0x85330  (impl 0x7F970)
    inline void beginWalkTo(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0x85330, _gb);
    }
    // int canSeeActor(@CActor actorToSee)
    //   thunk 0x84F10  (via call)
    inline int canSeeActor(void* self, void* actorToSee) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, actorToSee);
        vm::call(0x84F10, _gb);
        return _gb.get_i(0);
    }
    // void clearAITaskList()
    //   thunk 0x85540  (impl 0x7FBC0)
    inline void clearAITaskList(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x85540, _gb);
    }
    // void clearCommand()
    //   thunk 0x85510  (impl 0x7FBA0)
    inline void clearCommand(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x85510, _gb);
    }
    // void clearFollowActor()
    //   thunk 0x853F0  (impl 0x7F9D0)
    inline void clearFollowActor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x853F0, _gb);
    }
    // bool debounce(ELogicalControl button)
    //   thunk 0x85170  (via call)
    inline bool debounce(void* self, int button) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, button);
        vm::call(0x85170, _gb);
        return _gb.get_b(0);
    }
    // void debugAi(int flag)
    //   thunk 0x855D0  (via none)
    inline void debugAi(int flag) {
        vm::Block _gb;
        _gb.set_i(0, flag);
        vm::call(0x855D0, _gb);
    }
    // void faceActor(@CActor actorToFace)
    //   thunk 0x84F50  (impl 0x7CA10)
    inline void faceActor(void* self, void* actorToFace) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, actorToFace);
        vm::call(0x84F50, _gb);
    }
    // void freezeAI(bool flag)
    //   thunk 0x852A0  (impl 0x7F8E0)
    inline void freezeAI(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x852A0, _gb);
    }
    // float getAnimationLength(string animationName)
    //   thunk 0x84B30  (via call)
    inline float getAnimationLength(void* self, const char* animationName) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, animationName);
        vm::call(0x84B30, _gb);
        return _gb.get_f(0);
    }
    // float getHitPoints()
    //   thunk 0x84D90  (via call)
    inline float getHitPoints(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x84D90, _gb);
        return _gb.get_f(0);
    }
    // float getHitPointsPct()
    //   thunk 0x84E10  (via call)
    inline float getHitPointsPct(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x84E10, _gb);
        return _gb.get_f(0);
    }
    // float getMaxHitPoints()
    //   thunk 0x84DD0  (via call)
    inline float getMaxHitPoints(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x84DD0, _gb);
        return _gb.get_f(0);
    }
    // @CActor getSyncMasterPtr()
    //   thunk 0x85700  (via call)
    inline void* getSyncMasterPtr(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x85700, _gb);
        return _gb.get_p(0);
    }
    // @CActor getSyncSlavePtr()
    //   thunk 0x85740  (via call)
    inline void* getSyncSlavePtr(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x85740, _gb);
        return _gb.get_p(0);
    }
    // bool isDead()
    //   thunk 0x84D20  (via call)
    inline bool isDead(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x84D20, _gb);
        return _gb.get_b(0);
    }
    // int isFacing(vector posToCheck, float tolerance)
    //   thunk 0x84F80  (via call)
    inline int isFacing(void* self, Vector3 posToCheck, float tolerance) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_v(2, posToCheck);
        _gb.set_f(5, tolerance);
        vm::call(0x84F80, _gb);
        return _gb.get_i(0);
    }
    // bool isPlayingBigHurtAni()
    //   thunk 0x85030  (via call)
    inline bool isPlayingBigHurtAni(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x85030, _gb);
        return _gb.get_b(0);
    }
    // bool isPlayingTurnAni()
    //   thunk 0x84FF0  (via call)
    inline bool isPlayingTurnAni(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x84FF0, _gb);
        return _gb.get_b(0);
    }
    // bool isPossessed()
    //   thunk 0x84CE0  (via call)
    inline bool isPossessed(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x84CE0, _gb);
        return _gb.get_b(0);
    }
    // bool isPressed(ELogicalControl button)
    //   thunk 0x851B0  (via call)
    inline bool isPressed(void* self, int button) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, button);
        vm::call(0x851B0, _gb);
        return _gb.get_b(0);
    }
    // bool isSoftBodyActive()
    //   thunk 0x84CA0  (via call)
    inline bool isSoftBodyActive(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x84CA0, _gb);
        return _gb.get_b(0);
    }
    // bool isTalking()
    //   thunk 0x848A0  (via call)
    inline bool isTalking(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x848A0, _gb);
        return _gb.get_b(0);
    }
    // bool justPressed(ELogicalControl button)
    //   thunk 0x85230  (via call)
    inline bool justPressed(void* self, int button) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, button);
        vm::call(0x85230, _gb);
        return _gb.get_b(0);
    }
    // bool justReleased(ELogicalControl button)
    //   thunk 0x851F0  (via call)
    inline bool justReleased(void* self, int button) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, button);
        vm::call(0x851F0, _gb);
        return _gb.get_b(0);
    }
    // void lookAt(@CActor target, float lookatPct = 1.0f, float transitionTime = 0.75f)
    //   thunk 0x850A0  (impl 0x7EB10)
    inline void lookAt(void* self, void* target, float lookatPct = 1.0f, float transitionTime = 0.75f) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, target);
        _gb.set_f(2, lookatPct);
        _gb.set_f(3, transitionTime);
        vm::call(0x850A0, _gb);
    }
    // void pauseMorph(bool shouldPause)
    //   thunk 0x84BF0  (impl 0x77700)
    inline void pauseMorph(void* self, bool shouldPause) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, shouldPause);
        vm::call(0x84BF0, _gb);
    }
    // void pickup(@CActor object)
    //   thunk 0x85070  (impl 0x7D600)
    inline void pickup(void* self, void* object) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, object);
        vm::call(0x85070, _gb);
    }
    // int prepareToStartTalking(String dbEntryTag)
    //   thunk 0x84730  (via call)
    inline int prepareToStartTalking(void* self, const char* dbEntryTag) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, dbEntryTag);
        vm::call(0x84730, _gb);
        return _gb.get_i(0);
    }
    // void promptPlayer()
    //   thunk 0x849A0  (impl 0x76A60)
    inline void promptPlayer(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x849A0, _gb);
    }
    // void queueChitChat(@CDialogDatabaseEntry tag)
    //   thunk 0x848E0  (impl 0x76360)
    inline void queueChitChat(void* self, void* tag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, tag);
        vm::call(0x848E0, _gb);
    }
    // void queueWalkTo(@SScriptWalkInfo info)
    //   thunk 0x85360  (impl 0x7F980)
    inline void queueWalkTo(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0x85360, _gb);
    }
    // void resetChitChatQueue()
    //   thunk 0x84910  (impl 0x763C0)
    inline void resetChitChatQueue(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x84910, _gb);
    }
    // void restoreHitPointsToMax()
    //   thunk 0x84EB0  (impl 0x7B4C0)
    inline void restoreHitPointsToMax(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x84EB0, _gb);
    }
    // void setActorToControl(@CActor actorToControl)
    //   thunk 0x849D0  (via indirect)
    inline void setActorToControl(void* self, void* actorToControl) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, actorToControl);
        vm::call(0x849D0, _gb);
    }
    // void setAnimation(String animationName, bool useSkelFileExit = false)
    //   thunk 0x84AA0  (impl 0x77440)
    inline void setAnimation(void* self, const char* animationName, bool useSkelFileExit = false) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, animationName);
        _gb.set_b(2, useSkelFileExit);
        vm::call(0x84AA0, _gb);
    }
    // void setCombatComponentScriptOverrideMode(bool flag)
    //   thunk 0x855F0  (impl 0x7FC30)
    inline void setCombatComponentScriptOverrideMode(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x855F0, _gb);
    }
    // void setCommandFollowActor(@CActor actorToFollow, ECommandPosture posture, float xOffset, float zOffset, int run, int canAttack, int squad)
    //   thunk 0x85480  (via call)
    inline void setCommandFollowActor(void* self, void* actorToFollow, int posture, float xOffset, float zOffset, int run, int canAttack, int squad) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, actorToFollow);
        _gb.set_i(2, posture);
        _gb.set_f(3, xOffset);
        _gb.set_f(4, zOffset);
        _gb.set_i(5, run);
        _gb.set_i(6, canAttack);
        _gb.set_i(7, squad);
        vm::call(0x85480, _gb);
    }
    // void setCommandGoToPoint(@SScriptWalkInfo info, ECommandPosture posture)
    //   thunk 0x85450  (impl 0x7FA30)
    inline void setCommandGoToPoint(void* self, void* info, int posture) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        _gb.set_i(2, posture);
        vm::call(0x85450, _gb);
    }
    // void setCommandIdle()
    //   thunk 0x85420  (impl 0x7F9F0)
    inline void setCommandIdle(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x85420, _gb);
    }
    // void setConversationDialog(@CDialogDatabaseEntry converserTag, @CDialogDatabaseEntry converseeTag)
    //   thunk 0x84940  (impl 0x76970)
    inline void setConversationDialog(void* self, void* converserTag, void* converseeTag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, converserTag);
        _gb.set_p(2, converseeTag);
        vm::call(0x84940, _gb);
    }
    // void setDefaultAnimation()
    //   thunk 0x84B00  (via indirect)
    inline void setDefaultAnimation(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x84B00, _gb);
    }
    // void setEmotion(EEmotion emotion, float weight)
    //   thunk 0x84C20  (via none)
    inline void setEmotion(void* self, int emotion, float weight) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, emotion);
        _gb.set_f(2, weight);
        vm::call(0x84C20, _gb);
    }
    // void setFleeMinHealthRatio(float ratio)
    //   thunk 0x854E0  (impl 0x7FB80)
    inline void setFleeMinHealthRatio(void* self, float ratio) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, ratio);
        vm::call(0x854E0, _gb);
    }
    // void setFleePointWildCard(string newName)
    //   thunk 0x855A0  (impl 0x7FC00)
    inline void setFleePointWildCard(void* self, const char* newName) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, newName);
        vm::call(0x855A0, _gb);
    }
    // void setFollowActor(@CActor actor)
    //   thunk 0x853C0  (impl 0x7F9B0)
    inline void setFollowActor(void* self, void* actor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, actor);
        vm::call(0x853C0, _gb);
    }
    // void setFollowSplineInfo(@SScriptWalkInfo info, bool shouldWalk)
    //   thunk 0x85390  (impl 0x7F990)
    inline void setFollowSplineInfo(void* self, void* info, bool shouldWalk) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        _gb.set_b(2, shouldWalk);
        vm::call(0x85390, _gb);
    }
    // void setHitPoints(float newHitPoints)
    //   thunk 0x84E50  (impl 0x7B490)
    inline void setHitPoints(void* self, float newHitPoints) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, newHitPoints);
        vm::call(0x84E50, _gb);
    }
    // void setInvulnerableFlag(bool flag)
    //   thunk 0x84EE0  (impl 0x7B4E0)
    inline void setInvulnerableFlag(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x84EE0, _gb);
    }
    // void setMaxHitPoints(float newHitPoints)
    //   thunk 0x84E80  (impl 0x7B4B0)
    inline void setMaxHitPoints(void* self, float newHitPoints) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, newHitPoints);
        vm::call(0x84E80, _gb);
    }
    // void setNewSpawnName(string prefix)
    //   thunk 0x85650  (impl 0x83530)
    inline void setNewSpawnName(void* self, const char* prefix) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, prefix);
        vm::call(0x85650, _gb);
    }
    // void setPatrolRoute(string route)
    //   thunk 0x85570  (impl 0x7FBE0)
    inline void setPatrolRoute(void* self, const char* route) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, route);
        vm::call(0x85570, _gb);
    }
    // void setRimLight(bool flag)
    //   thunk 0x84C40  (impl 0x77780)
    inline void setRimLight(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x84C40, _gb);
    }
    // void setScriptAiControl(bool flag)
    //   thunk 0x852D0  (impl 0x7F910)
    inline void setScriptAiControl(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x852D0, _gb);
    }
    // void setScriptAnimationControl(bool flag)
    //   thunk 0x84A70  (impl 0x77420)
    inline void setScriptAnimationControl(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x84A70, _gb);
    }
    // void setSimEnable(int flag)
    //   thunk 0x84C70  (impl 0x89940)
    inline void setSimEnable(void* self, int flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, flag);
        vm::call(0x84C70, _gb);
    }
    // void setVictim(@CActor victim)
    //   thunk 0x850E0  (impl 0x7EB40)
    inline void setVictim(void* self, void* victim) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, victim);
        vm::call(0x850E0, _gb);
    }
    // void setVictimDefault()
    //   thunk 0x85140  (impl 0x7EC50)
    inline void setVictimDefault(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x85140, _gb);
    }
    // void setVictimDisable()
    //   thunk 0x85110  (impl 0x7EC40)
    inline void setVictimDisable(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x85110, _gb);
    }
    // void setVictimUponSpawn(@CCharacter victim)
    //   thunk 0x85620  (impl 0x83480)
    inline void setVictimUponSpawn(void* self, void* victim) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, victim);
        vm::call(0x85620, _gb);
    }
    // void skipToAnimation(String animationName, bool useSkelFileExit = false)
    //   thunk 0x84AD0  (impl 0x774A0)
    inline void skipToAnimation(void* self, const char* animationName, bool useSkelFileExit = false) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, animationName);
        _gb.set_b(2, useSkelFileExit);
        vm::call(0x84AD0, _gb);
    }
    // bool startControllingActor(@CActor actorToControl)
    //   thunk 0x84A00  (via call)
    inline bool startControllingActor(void* self, void* actorToControl) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, actorToControl);
        vm::call(0x84A00, _gb);
        return _gb.get_b(0);
    }
    // void startMorph(int index, float duration, bool loop, bool holdFinalWeight)
    //   thunk 0x84B70  (via call)
    inline void startMorph(void* self, int index, float duration, bool loop, bool holdFinalWeight) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, index);
        _gb.set_f(2, duration);
        _gb.set_b(3, loop);
        _gb.set_b(4, holdFinalWeight);
        vm::call(0x84B70, _gb);
    }
    // int startPreparedTalking(String dbEntryTag, int handle)
    //   thunk 0x84770  (via call)
    inline int startPreparedTalking(void* self, const char* dbEntryTag, int handle) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, dbEntryTag);
        _gb.set_i(3, handle);
        vm::call(0x84770, _gb);
        return _gb.get_i(0);
    }
    // bool startSyncingAnimation(@CActor slave, String animationName)
    //   thunk 0x85680  (via call)
    inline bool startSyncingAnimation(void* self, void* slave, const char* animationName) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, slave);
        _gb.set_p(3, animationName);
        vm::call(0x85680, _gb);
        return _gb.get_b(0);
    }
    // float startTalking(String dbEntryTag)
    //   thunk 0x847C0  (via call)
    inline float startTalking(void* self, const char* dbEntryTag) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, dbEntryTag);
        vm::call(0x847C0, _gb);
        return _gb.get_f(0);
    }
    // float startTalkingNoSubtitle(String dbEntryTag)
    //   thunk 0x84800  (via call)
    inline float startTalkingNoSubtitle(void* self, const char* dbEntryTag) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, dbEntryTag);
        vm::call(0x84800, _gb);
        return _gb.get_f(0);
    }
    // void stopControllingActor()
    //   thunk 0x84A40  (impl 0x76FD0)
    inline void stopControllingActor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x84A40, _gb);
    }
    // void stopMorph(int index, bool clearMorph)
    //   thunk 0x84BC0  (impl 0x77690)
    inline void stopMorph(void* self, int index, bool clearMorph) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, index);
        _gb.set_b(2, clearMorph);
        vm::call(0x84BC0, _gb);
    }
    // void stopSyncingAnimation()
    //   thunk 0x856D0  (impl 0x83A00)
    inline void stopSyncingAnimation(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x856D0, _gb);
    }
    // void stopTalking(bool now)
    //   thunk 0x84870  (impl 0x75EF0)
    inline void stopTalking(void* self, bool now) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, now);
        vm::call(0x84870, _gb);
    }
    // void toggleAiCallouts(bool flag)
    //   thunk 0x84840  (impl 0x75EE0)
    inline void toggleAiCallouts(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x84840, _gb);
    }
    // void toggleObstacleAvoidance(bool flag)
    //   thunk 0x85300  (impl 0x7F930)
    inline void toggleObstacleAvoidance(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x85300, _gb);
    }
} // namespace CCharacter

// ------------------------------------------- CCinematicSkeletonModel
namespace CCinematicSkeletonModel {
    // void setAnimation(String animationName)
    //   thunk 0x8DF00  (impl 0x8D840)
    inline void setAnimation(void* self, const char* animationName) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, animationName);
        vm::call(0x8DF00, _gb);
    }
    // void skipToAnimation(String animationName)
    //   thunk 0x8DF30  (impl 0x8D8C0)
    inline void skipToAnimation(void* self, const char* animationName) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, animationName);
        vm::call(0x8DF30, _gb);
    }
    // void stopSync()
    //   thunk 0x8DFB0  (impl 0x8DA40)
    inline void stopSync(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x8DFB0, _gb);
    }
    // bool syncWith(@CCinematicSkeletonModel withWhom, string animationName)
    //   thunk 0x8DF60  (via call)
    inline bool syncWith(void* self, void* withWhom, const char* animationName) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, withWhom);
        _gb.set_p(3, animationName);
        vm::call(0x8DF60, _gb);
        return _gb.get_b(0);
    }
} // namespace CCinematicSkeletonModel

// ---------------------------------------------------------- CConbelt
namespace CConbelt {
    // void setSpeed(float speed)
    //   thunk 0x3CFAF0  (via none)
    inline void setSpeed(void* self, float speed) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, speed);
        vm::call(0x3CFAF0, _gb);
    }
} // namespace CConbelt

// ---------------------------------------------------------- CCurtain
namespace CCurtain {
    // void letGo()
    //   thunk 0x4969D0  (via none)
    inline void letGo(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x4969D0, _gb);
    }
    // void setMaxWind(vector windVelocity)
    //   thunk 0x496A00  (via none)
    inline void setMaxWind(void* self, Vector3 windVelocity) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, windVelocity);
        vm::call(0x496A00, _gb);
    }
    // void setMinWind(vector windVelocity)
    //   thunk 0x496A60  (via none)
    inline void setMinWind(void* self, Vector3 windVelocity) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, windVelocity);
        vm::call(0x496A60, _gb);
    }
    // void setWindTime(float value)
    //   thunk 0x496AC0  (via none)
    inline void setWindTime(void* self, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, value);
        vm::call(0x496AC0, _gb);
    }
} // namespace CCurtain

// ---------------------------------------------- CDialogDatabaseEntry
namespace CDialogDatabaseEntry {
    // float estimateDurationBasedOnText()
    //   thunk 0x3D1E70  (via none)
    inline float estimateDurationBasedOnText(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x3D1E70, _gb);
        return _gb.get_f(0);
    }
    // float getDuration()
    //   thunk 0x3D1EE0  (via call)
    inline float getDuration(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x3D1EE0, _gb);
        return _gb.get_f(0);
    }
} // namespace CDialogDatabaseEntry

// ------------------------------------------------------------- CDoor
namespace CDoor {
    // float getHitPoints()
    //   thunk 0x910D0  (via call)
    inline float getHitPoints(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x910D0, _gb);
        return _gb.get_f(0);
    }
    // void getHurt(vector pos, int damageAmount)
    //   thunk 0x91110  (via call)
    inline void getHurt(void* self, Vector3 pos, int damageAmount) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, pos);
        _gb.set_i(4, damageAmount);
        vm::call(0x91110, _gb);
    }
    // void setDoorBlocked(bool isBlocked)
    //   thunk 0x91170  (impl 0x91090)
    inline void setDoorBlocked(void* self, bool isBlocked) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, isBlocked);
        vm::call(0x91170, _gb);
    }
    // void setDoorState(EDoorState doorState)
    //   thunk 0x910A0  (impl 0x90F80)
    inline void setDoorState(void* self, int doorState) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, doorState);
        vm::call(0x910A0, _gb);
    }
} // namespace CDoor

// --------------------------------------------------------- CElevator
namespace CElevator {
    // void setDoorState(EElevatorDoorState doorState)
    //   thunk 0x93B50  (impl 0x93B30)
    inline void setDoorState(void* self, int doorState) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, doorState);
        vm::call(0x93B50, _gb);
    }
} // namespace CElevator

// ---------------------------------------------------------- CEmitter
namespace CEmitter {
    // void kill()
    //   thunk 0x49A100  (via indirect)
    inline void kill(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x49A100, _gb);
    }
    // void release()
    //   thunk 0x49A130  (impl 0x49B100)
    inline void release(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x49A130, _gb);
    }
    // void start()
    //   thunk 0x49A150  (via indirect)
    inline void start(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x49A150, _gb);
    }
} // namespace CEmitter

// -------------------------------------------------- CFixedJointActor
namespace CFixedJointActor {
    // void breakJoint()
    //   thunk 0x492BC0  (via call)
    inline void breakJoint(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x492BC0, _gb);
    }
} // namespace CFixedJointActor

// ---------------------------------------------------------- CFloater
namespace CFloater {
    // void setRenderOnlyInMirrors(bool flag)
    //   thunk 0x9DD20  (impl 0x9AB30)
    inline void setRenderOnlyInMirrors(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x9DD20, _gb);
    }
} // namespace CFloater

// ------------------------------------------------------ CFlyerMedium
namespace CFlyerMedium {
    // void fillInFlyerMediumCCSOD(@SFlyerMediumCombatComponentInfo info)
    //   thunk 0xA5800  (impl 0xA4A30)
    inline void fillInFlyerMediumCCSOD(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0xA5800, _gb);
    }
} // namespace CFlyerMedium

// ------------------------------------------------------- CFlyerSmall
namespace CFlyerSmall {
    // void fillInCombatComponentScriptOverrideData(@SFlyerSmallCombatComponentInfo info)
    //   thunk 0xA9530  (impl 0xA93A0)
    inline void fillInCombatComponentScriptOverrideData(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0xA9530, _gb);
    }
    // void suicide()
    //   thunk 0xA9500  (impl 0xA7AC0)
    inline void suicide(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xA9500, _gb);
    }
} // namespace CFlyerSmall

// --------------------------------------------------------- CGameView
namespace CGameView {
    // void impactCamera(vector dirMag, float hitDuration, float recoveryDuration)
    //   thunk 0x206070  (via call)
    inline void impactCamera(void* self, Vector3 dirMag, float hitDuration, float recoveryDuration) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, dirMag);
        _gb.set_f(4, hitDuration);
        _gb.set_f(5, recoveryDuration);
        vm::call(0x206070, _gb);
    }
    // void resetCamera()
    //   thunk 0x205FB0  (impl 0x1FE7A0)
    inline void resetCamera(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x205FB0, _gb);
    }
    // void setCameraModeFixed(@CCameraPathActor path, @CActor followTarget = NULL, float transitionTime = 0.0)
    //   thunk 0x206160  (impl 0x1FF790)
    inline void setCameraModeFixed(void* self, void* path, void* followTarget, float transitionTime = 0.0) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, path);
        _gb.set_p(2, followTarget);
        _gb.set_f(3, transitionTime);
        vm::call(0x206160, _gb);
    }
    // void setCameraModeNormal(float transitionDistance = 0.0, float transitionTime = 0.0)
    //   thunk 0x2060E0  (impl 0x1FF5F0)
    inline void setCameraModeNormal(void* self, float transitionDistance = 0.0, float transitionTime = 0.0) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, transitionDistance);
        _gb.set_f(2, transitionTime);
        vm::call(0x2060E0, _gb);
    }
    // void setCameraModeOrbit(@CActor followTarget, float radius, float rps, float targetHeightPct, float offsetHeight, float transitionTime = 0.0f, float orbitDuration = -1.0f)
    //   thunk 0x2061A0  (via call)
    inline void setCameraModeOrbit(void* self, void* followTarget, float radius, float rps, float targetHeightPct, float offsetHeight, float transitionTime = 0.0f, float orbitDuration = -1.0f) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, followTarget);
        _gb.set_f(2, radius);
        _gb.set_f(3, rps);
        _gb.set_f(4, targetHeightPct);
        _gb.set_f(5, offsetHeight);
        _gb.set_f(6, transitionTime);
        _gb.set_f(7, orbitDuration);
        vm::call(0x2061A0, _gb);
    }
    // void setCameraPathActor(@CCameraPathActor path, float transitionTime, float startTime, int allowQuickLook = 0)
    //   thunk 0x206110  (via call)
    inline void setCameraPathActor(void* self, void* path, float transitionTime, float startTime, int allowQuickLook = 0) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, path);
        _gb.set_f(2, transitionTime);
        _gb.set_f(3, startTime);
        _gb.set_i(4, allowQuickLook);
        vm::call(0x206110, _gb);
    }
    // void setCinematControlOverride(bool flag)
    //   thunk 0x206210  (via none)
    inline void setCinematControlOverride(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x206210, _gb);
    }
    // void shakeCamera(float strength, float duration, float rampUpTime = 0.0, float rampDownTime = 0.0, float speed = 1.0)
    //   thunk 0x205FE0  (via call)
    inline void shakeCamera(void* self, float strength, float duration, float rampUpTime = 0.0, float rampDownTime = 0.0, float speed = 1.0) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, strength);
        _gb.set_f(2, duration);
        _gb.set_f(3, rampUpTime);
        _gb.set_f(4, rampDownTime);
        _gb.set_f(5, speed);
        vm::call(0x205FE0, _gb);
    }
    // void shakeCameraEx(@SShakeCameraInfo info)
    //   thunk 0x206040  (impl 0x1FF5A0)
    inline void shakeCameraEx(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0x206040, _gb);
    }
} // namespace CGameView

// ------------------------------------------------------------ CGhost
namespace CGhost {
    // void assignSpline(@CAiSplinePath spline)
    //   thunk 0xBF6F0  (via none)
    inline void assignSpline(void* spline) {
        vm::Block _gb;
        _gb.set_p(0, spline);
        vm::call(0xBF6F0, _gb);
    }
    // void fillInCombatComponentScriptOverrideData(@SGhostCombatComponentInfo info)
    //   thunk 0xBF690  (impl 0xBD640)
    inline void fillInCombatComponentScriptOverrideData(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0xBF690, _gb);
    }
    // @CAiSplinePath getCurrentSpline()
    //   thunk 0xBF710  (via call)
    inline void* getCurrentSpline(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xBF710, _gb);
        return _gb.get_p(0);
    }
    // bool getPKEGameResult()
    //   thunk 0xBF890  (via call)
    inline bool getPKEGameResult(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xBF890, _gb);
        return _gb.get_b(0);
    }
    // void setAcc(float newVal)
    //   thunk 0xBF750  (impl 0xBF460)
    inline void setAcc(void* self, float newVal) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, newVal);
        vm::call(0xBF750, _gb);
    }
    // void setCommandAmbush(@CActor ambushPoint)
    //   thunk 0xBF8D0  (via none)
    inline void setCommandAmbush(void* ambushPoint) {
        vm::Block _gb;
        _gb.set_p(0, ambushPoint);
        vm::call(0xBF8D0, _gb);
    }
    // void setCommandGhostFlee(@SGhostFleeInfo info)
    //   thunk 0xBF8F0  (via none)
    inline void setCommandGhostFlee(void* info) {
        vm::Block _gb;
        _gb.set_p(0, info);
        vm::call(0xBF8F0, _gb);
    }
    // void setCommandPossess(@CCharacter characterToPossess)
    //   thunk 0xBF940  (impl 0xBF5E0)
    inline void setCommandPossess(void* self, void* characterToPossess) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, characterToPossess);
        vm::call(0xBF940, _gb);
    }
    // void setFaceDirection(vector iDir)
    //   thunk 0xBF7B0  (via call)
    inline void setFaceDirection(void* self, Vector3 iDir) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, iDir);
        vm::call(0xBF7B0, _gb);
    }
    // void setScanAggressiveRange(float newVal)
    //   thunk 0xBF850  (via none)
    inline void setScanAggressiveRange(float newVal) {
        vm::Block _gb;
        _gb.set_f(0, newVal);
        vm::call(0xBF850, _gb);
    }
    // void setScanAwareRange(float newVal)
    //   thunk 0xBF830  (via none)
    inline void setScanAwareRange(float newVal) {
        vm::Block _gb;
        _gb.set_f(0, newVal);
        vm::call(0xBF830, _gb);
    }
    // void setScanAwareTime(float newVal)
    //   thunk 0xBF870  (via none)
    inline void setScanAwareTime(float newVal) {
        vm::Block _gb;
        _gb.set_f(0, newVal);
        vm::call(0xBF870, _gb);
    }
    // void setScanSightRange(float newVal)
    //   thunk 0xBF810  (via none)
    inline void setScanSightRange(float newVal) {
        vm::Block _gb;
        _gb.set_f(0, newVal);
        vm::call(0xBF810, _gb);
    }
    // void setSpeed(float newVal)
    //   thunk 0xBF780  (impl 0xBF490)
    inline void setSpeed(void* self, float newVal) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, newVal);
        vm::call(0xBF780, _gb);
    }
    // bool wasTrapSuper()
    //   thunk 0xBF970  (via call)
    inline bool wasTrapSuper(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xBF970, _gb);
        return _gb.get_b(0);
    }
} // namespace CGhost

// ----------------------------------------------------- CGhostEffects
namespace CGhostEffects {
    // void ActorForAttract(@CActor a, float spd)
    //   thunk 0xFABB0  (impl 0xF7570)
    inline void ActorForAttract(void* self, void* a, float spd) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        _gb.set_f(2, spd);
        vm::call(0xFABB0, _gb);
    }
    // void ActorForEject(@CActor a, vector iDir, float spd)
    //   thunk 0xFAB10  (via call)
    inline void ActorForEject(void* self, void* a, Vector3 iDir, float spd) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        _gb.set_v(2, iDir);
        _gb.set_f(5, spd);
        vm::call(0xFAB10, _gb);
    }
    // void ActorForFollowPath(@CActor a)
    //   thunk 0xFAAE0  (impl 0xF7370)
    inline void ActorForFollowPath(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        vm::call(0xFAAE0, _gb);
    }
    // void ActorForLevitate(@CActor a)
    //   thunk 0xFAAB0  (impl 0xF6F10)
    inline void ActorForLevitate(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        vm::call(0xFAAB0, _gb);
    }
    // void ActorForRepulse(@CActor a, float spd)
    //   thunk 0xFAB80  (impl 0xF7450)
    inline void ActorForRepulse(void* self, void* a, float spd) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        _gb.set_f(2, spd);
        vm::call(0xFAB80, _gb);
    }
    // void ActorForSlam(@CActor a, float apogee)
    //   thunk 0xFABE0  (impl 0xF7690)
    inline void ActorForSlam(void* self, void* a, float apogee) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        _gb.set_f(2, apogee);
        vm::call(0xFABE0, _gb);
    }
    // void BoneSimActorForAttract(@CBoneSimActor a, int boneIndex, float spd)
    //   thunk 0xFAD20  (impl 0xF8230)
    inline void BoneSimActorForAttract(void* self, void* a, int boneIndex, float spd) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        _gb.set_i(2, boneIndex);
        _gb.set_f(3, spd);
        vm::call(0xFAD20, _gb);
    }
    // void BoneSimActorForEject(@CBoneSimActor a, int boneIndex, vector iDir, float spd)
    //   thunk 0xFAC70  (via call)
    inline void BoneSimActorForEject(void* self, void* a, int boneIndex, Vector3 iDir, float spd) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        _gb.set_i(2, boneIndex);
        _gb.set_v(3, iDir);
        _gb.set_f(6, spd);
        vm::call(0xFAC70, _gb);
    }
    // void BoneSimActorForFollowPath(@CBoneSimActor a, int boneIndex)
    //   thunk 0xFAC40  (impl 0xF7FF0)
    inline void BoneSimActorForFollowPath(void* self, void* a, int boneIndex) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        _gb.set_i(2, boneIndex);
        vm::call(0xFAC40, _gb);
    }
    // void BoneSimActorForLevitate(@CBoneSimActor a, int boneIndex)
    //   thunk 0xFAC10  (impl 0xF7D80)
    inline void BoneSimActorForLevitate(void* self, void* a, int boneIndex) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        _gb.set_i(2, boneIndex);
        vm::call(0xFAC10, _gb);
    }
    // void BoneSimActorForRepulse(@CBoneSimActor a, int boneIndex, float spd)
    //   thunk 0xFACE0  (impl 0xF8110)
    inline void BoneSimActorForRepulse(void* self, void* a, int boneIndex, float spd) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        _gb.set_i(2, boneIndex);
        _gb.set_f(3, spd);
        vm::call(0xFACE0, _gb);
    }
    // void BoneSimActorForSlam(@CBoneSimActor a, int boneIndex, float apogee)
    //   thunk 0xFAD60  (impl 0xF8350)
    inline void BoneSimActorForSlam(void* self, void* a, int boneIndex, float apogee) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        _gb.set_i(2, boneIndex);
        _gb.set_f(3, apogee);
        vm::call(0xFAD60, _gb);
    }
    // void addActorToVortex(@CActor actor)
    //   thunk 0xFB4D0  (impl 0xFA2B0)
    inline void addActorToVortex(void* self, void* actor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, actor);
        vm::call(0xFB4D0, _gb);
    }
    // bool isAttract(@CActor a)
    //   thunk 0xFB0A0  (via call)
    inline bool isAttract(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, a);
        vm::call(0xFB0A0, _gb);
        return _gb.get_b(0);
    }
    // bool isFollowPath(@CActor a)
    //   thunk 0xFB160  (via call)
    inline bool isFollowPath(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, a);
        vm::call(0xFB160, _gb);
        return _gb.get_b(0);
    }
    // bool isIdle(@CActor a)
    //   thunk 0xFB120  (via call)
    inline bool isIdle(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, a);
        vm::call(0xFB120, _gb);
        return _gb.get_b(0);
    }
    // bool isLevitate(@CActor a)
    //   thunk 0xFB060  (via call)
    inline bool isLevitate(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, a);
        vm::call(0xFB060, _gb);
        return _gb.get_b(0);
    }
    // bool isListFull()
    //   thunk 0xFAE00  (via call)
    inline bool isListFull(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xFAE00, _gb);
        return _gb.get_b(0);
    }
    // bool isRepulse(@CActor a)
    //   thunk 0xFB0E0  (via call)
    inline bool isRepulse(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, a);
        vm::call(0xFB0E0, _gb);
        return _gb.get_b(0);
    }
    // void meander(float distance)
    //   thunk 0xFB560  (impl 0xFA740)
    inline void meander(void* self, float distance) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, distance);
        vm::call(0xFB560, _gb);
    }
    // void pathAllGo()
    //   thunk 0xFB290  (impl 0xF9BF0)
    inline void pathAllGo(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xFB290, _gb);
    }
    // void pathAllStop()
    //   thunk 0xFB260  (impl 0xF9BB0)
    inline void pathAllStop(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xFB260, _gb);
    }
    // void pathGo(@CActor a)
    //   thunk 0xFB230  (impl 0xF9B70)
    inline void pathGo(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        vm::call(0xFB230, _gb);
    }
    // void pathStop(@CActor a)
    //   thunk 0xFB200  (impl 0xF9B30)
    inline void pathStop(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        vm::call(0xFB200, _gb);
    }
    // void randomizeVortex(bool inPlace = false)
    //   thunk 0xFB320  (impl 0xFA110)
    inline void randomizeVortex(void* self, bool inPlace = false) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, inPlace);
        vm::call(0xFB320, _gb);
    }
    // void reRandomize()
    //   thunk 0xFB530  (impl 0xFA510)
    inline void reRandomize(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xFB530, _gb);
    }
    // void removeActorFromVortex(@CActor actor)
    //   thunk 0xFB500  (impl 0xFA490)
    inline void removeActorFromVortex(void* self, void* actor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, actor);
        vm::call(0xFB500, _gb);
    }
    // void setAllSpeed(float fps)
    //   thunk 0xFB1A0  (impl 0xF9A80)
    inline void setAllSpeed(void* self, float fps) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, fps);
        vm::call(0xFB1A0, _gb);
    }
    // void setAllToAttract()
    //   thunk 0xFAF10  (impl 0xF9730)
    inline void setAllToAttract(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xFAF10, _gb);
    }
    // void setAllToIdle()
    //   thunk 0xFAE80  (impl 0xF9670)
    inline void setAllToIdle(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xFAE80, _gb);
    }
    // void setAllToLevitate()
    //   thunk 0xFAEB0  (impl 0xF96B0)
    inline void setAllToLevitate(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xFAEB0, _gb);
    }
    // void setAllToRepulse()
    //   thunk 0xFAEE0  (impl 0xF96F0)
    inline void setAllToRepulse(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xFAEE0, _gb);
    }
    // void setSpeed(@CActor a, float fps)
    //   thunk 0xFB1D0  (impl 0xF9AD0)
    inline void setSpeed(void* self, void* a, float fps) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        _gb.set_f(2, fps);
        vm::call(0xFB1D0, _gb);
    }
    // void setToAttract(@CActor a)
    //   thunk 0xFB030  (impl 0xF98B0)
    inline void setToAttract(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        vm::call(0xFB030, _gb);
    }
    // void setToFollowPath(@CActor a)
    //   thunk 0xFAFD0  (impl 0xF9830)
    inline void setToFollowPath(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        vm::call(0xFAFD0, _gb);
    }
    // void setToIdle(@CActor a)
    //   thunk 0xFAF40  (impl 0xF9770)
    inline void setToIdle(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        vm::call(0xFAF40, _gb);
    }
    // void setToLevitate(@CActor a)
    //   thunk 0xFAFA0  (impl 0xF97F0)
    inline void setToLevitate(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        vm::call(0xFAFA0, _gb);
    }
    // void setToRepulse(@CActor a)
    //   thunk 0xFB000  (impl 0xF9870)
    inline void setToRepulse(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        vm::call(0xFB000, _gb);
    }
    // void setToSlam(@CActor a)
    //   thunk 0xFAF70  (impl 0xF97B0)
    inline void setToSlam(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        vm::call(0xFAF70, _gb);
    }
    // void setVortexAltitudeRate(float feetPerSecond)
    //   thunk 0xFB380  (impl 0xFA240)
    inline void setVortexAltitudeRate(void* self, float feetPerSecond) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, feetPerSecond);
        vm::call(0xFB380, _gb);
    }
    // void setVortexFollowActor(@CActor actor)
    //   thunk 0xFB4A0  (impl 0xFA2A0)
    inline void setVortexFollowActor(void* self, void* actor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, actor);
        vm::call(0xFB4A0, _gb);
    }
    // void setVortexHiAltitude(float hiAlt)
    //   thunk 0xFB3E0  (impl 0xFA260)
    inline void setVortexHiAltitude(void* self, float hiAlt) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, hiAlt);
        vm::call(0xFB3E0, _gb);
    }
    // void setVortexHiRadius(float hiRadius)
    //   thunk 0xFB440  (impl 0xFA280)
    inline void setVortexHiRadius(void* self, float hiRadius) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, hiRadius);
        vm::call(0xFB440, _gb);
    }
    // void setVortexLoAltitude(float loAlt)
    //   thunk 0xFB3B0  (impl 0xFA250)
    inline void setVortexLoAltitude(void* self, float loAlt) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, loAlt);
        vm::call(0xFB3B0, _gb);
    }
    // void setVortexLoRadius(float loRadius)
    //   thunk 0xFB410  (impl 0xFA270)
    inline void setVortexLoRadius(void* self, float loRadius) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, loRadius);
        vm::call(0xFB410, _gb);
    }
    // void setVortexRotationRate(float degreesPerSecond)
    //   thunk 0xFB350  (impl 0xFA220)
    inline void setVortexRotationRate(void* self, float degreesPerSecond) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, degreesPerSecond);
        vm::call(0xFB350, _gb);
    }
    // void setVortexSpacing(float spacing)
    //   thunk 0xFB470  (impl 0xFA290)
    inline void setVortexSpacing(void* self, float spacing) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, spacing);
        vm::call(0xFB470, _gb);
    }
    // int spaceRemaining()
    //   thunk 0xFAE40  (via call)
    inline int spaceRemaining(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xFAE40, _gb);
        return _gb.get_i(0);
    }
    // void startFollowPath()
    //   thunk 0xFADD0  (impl 0xF8F00)
    inline void startFollowPath(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xFADD0, _gb);
    }
    // void startLevitation()
    //   thunk 0xFADA0  (impl 0xF8380)
    inline void startLevitation(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xFADA0, _gb);
    }
    // void startVortex()
    //   thunk 0xFB2C0  (impl 0xF9C30)
    inline void startVortex(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xFB2C0, _gb);
    }
    // void stopVortex()
    //   thunk 0xFB2F0  (impl 0xFA060)
    inline void stopVortex(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xFB2F0, _gb);
    }
} // namespace CGhostEffects

// ------------------------------------------------------ CGhostbuster
namespace CGhostbuster {
    // bool advanceTutorial()
    //   thunk 0xEE080  (via call)
    inline bool advanceTutorial(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xEE080, _gb);
        return _gb.get_b(0);
    }
    // void blockHeroMovement(bool flag)
    //   thunk 0xED660  (impl 0xD82E0)
    inline void blockHeroMovement(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0xED660, _gb);
    }
    // void cacheRappel()
    //   thunk 0xED910  (impl 0xE1D70)
    inline void cacheRappel(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xED910, _gb);
    }
    // bool canFireProtonTorpedo()
    //   thunk 0xEDE60  (via call)
    inline bool canFireProtonTorpedo(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xEDE60, _gb);
        return _gb.get_b(0);
    }
    // void commitSuicide()
    //   thunk 0xED3B0  (impl 0xCE560)
    inline void commitSuicide(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xED3B0, _gb);
    }
    // int debounceEvasionButton(ELogicalControl button)
    //   thunk 0xED8D0  (via call)
    inline int debounceEvasionButton(void* self, int button) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, button);
        vm::call(0xED8D0, _gb);
        return _gb.get_i(0);
    }
    // void enableGiantBossMode(bool flag)
    //   thunk 0xED5D0  (impl 0xD7760)
    inline void enableGiantBossMode(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0xED5D0, _gb);
    }
    // void enableInventoryItem(EInventoryItem which, bool flag)
    //   thunk 0xEDAC0  (impl 0xE4530)
    inline void enableInventoryItem(void* self, int which, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, which);
        _gb.set_b(2, flag);
        vm::call(0xEDAC0, _gb);
    }
    // void enableProtonTorpedo(bool flag)
    //   thunk 0xEDE30  (impl 0xE6250)
    inline void enableProtonTorpedo(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0xEDE30, _gb);
    }
    // void enableTrapDummyMode()
    //   thunk 0xEDB80  (impl 0xE4A10)
    inline void enableTrapDummyMode(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xEDB80, _gb);
    }
    // void fakeFireIceStream(bool flag)
    //   thunk 0xEDED0  (impl 0xE90E0)
    inline void fakeFireIceStream(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0xEDED0, _gb);
    }
    // void fakeFireProtonGun(bool flag)
    //   thunk 0xEDEA0  (impl 0xE9060)
    inline void fakeFireProtonGun(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0xEDEA0, _gb);
    }
    // void fakeFireShotgun()
    //   thunk 0xEDF00  (impl 0xE9160)
    inline void fakeFireShotgun(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xEDF00, _gb);
    }
    // void fakePossession(bool flag)
    //   thunk 0xEE0F0  (impl 0xEC1E0)
    inline void fakePossession(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0xEE0F0, _gb);
    }
    // void flinch()
    //   thunk 0xED540  (impl 0xD18B0)
    inline void flinch(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xED540, _gb);
    }
    // void forceDeploySuperTrap(@CWayPoint waypoint)
    //   thunk 0xEDC10  (impl 0xE4CC0)
    inline void forceDeploySuperTrap(void* self, void* waypoint) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, waypoint);
        vm::call(0xEDC10, _gb);
    }
    // void forceDeployTrap(Vector wPos)
    //   thunk 0xEDBB0  (via call)
    inline void forceDeployTrap(void* self, Vector3 wPos) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wPos);
        vm::call(0xEDBB0, _gb);
    }
    // void forceSnapProtonBeam()
    //   thunk 0xEDD00  (impl 0xE6100)
    inline void forceSnapProtonBeam(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xEDD00, _gb);
    }
    // void gatherAllDeployedInventoryItems()
    //   thunk 0xEDB50  (impl 0xE4770)
    inline void gatherAllDeployedInventoryItems(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xEDB50, _gb);
    }
    // @CGhostTrap getDeployedTrapPtr()
    //   thunk 0xEDC70  (via call)
    inline void* getDeployedTrapPtr(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xEDC70, _gb);
        return _gb.get_p(0);
    }
    // float getEvasionSprintPct()
    //   thunk 0xED860  (via call)
    inline float getEvasionSprintPct(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xED860, _gb);
        return _gb.get_f(0);
    }
    // EFlashlightMode getFlashlightMode()
    //   thunk 0xEDA00  (via call)
    inline int getFlashlightMode(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xEDA00, _gb);
        return _gb.get_i(0);
    }
    // EJumpDirection getJumpDirection()
    //   thunk 0xED7F0  (via call)
    inline int getJumpDirection(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xED7F0, _gb);
        return _gb.get_i(0);
    }
    // void hideHack()
    //   thunk 0xEE2D0  (impl 0xED0D0)
    inline void hideHack(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xEE2D0, _gb);
    }
    // bool isHealthy()
    //   thunk 0xED3E0  (via call)
    inline bool isHealthy(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xED3E0, _gb);
        return _gb.get_b(0);
    }
    // bool isPackAboutToOverheat()
    //   thunk 0xEDF70  (via call)
    inline bool isPackAboutToOverheat(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xEDF70, _gb);
        return _gb.get_b(0);
    }
    // bool isPackOverheated()
    //   thunk 0xEDF30  (via call)
    inline bool isPackOverheated(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xEDF30, _gb);
        return _gb.get_b(0);
    }
    // bool isProtonBeamActive()
    //   thunk 0xEDD30  (via call)
    inline bool isProtonBeamActive(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xEDD30, _gb);
        return _gb.get_b(0);
    }
    // bool isProtonBeamInBurstMode()
    //   thunk 0xEDD70  (via call)
    inline bool isProtonBeamInBurstMode(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xEDD70, _gb);
        return _gb.get_b(0);
    }
    // bool isProtonBeamInContainMode()
    //   thunk 0xEDDB0  (via call)
    inline bool isProtonBeamInContainMode(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xEDDB0, _gb);
        return _gb.get_b(0);
    }
    // bool isProtonBeamInFreezeMode()
    //   thunk 0xEDDF0  (via call)
    inline bool isProtonBeamInFreezeMode(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xEDDF0, _gb);
        return _gb.get_b(0);
    }
    // bool isTrapDeployed()
    //   thunk 0xED7B0  (via call)
    inline bool isTrapDeployed(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xED7B0, _gb);
        return _gb.get_b(0);
    }
    // void killTutorial()
    //   thunk 0xEE0C0  (impl 0xEBD50)
    inline void killTutorial(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xEE0C0, _gb);
    }
    // void knockBack(vector wSourcePos, float impulse)
    //   thunk 0xEE300  (via call)
    inline void knockBack(void* self, Vector3 wSourcePos, float impulse) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wSourcePos);
        _gb.set_f(4, impulse);
        vm::call(0xEE300, _gb);
    }
    // void mountProtonPack(bool flag)
    //   thunk 0xEDB20  (impl 0xE4710)
    inline void mountProtonPack(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0xEDB20, _gb);
    }
    // void pretendToDrive(@CCarEffects car, bool driversSeatFlag = true, bool putTrapOnRoofFlag = false)
    //   thunk 0xEE010  (impl 0xEAC40)
    inline void pretendToDrive(void* self, void* car, bool driversSeatFlag = true, bool putTrapOnRoofFlag = false) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, car);
        _gb.set_b(2, driversSeatFlag);
        _gb.set_b(3, putTrapOnRoofFlag);
        vm::call(0xEE010, _gb);
    }
    // bool readyInventoryItem(EInventoryItem itemToSwitchTo)
    //   thunk 0xEDA40  (via call)
    inline bool readyInventoryItem(void* self, int itemToSwitchTo) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, itemToSwitchTo);
        vm::call(0xEDA40, _gb);
        return _gb.get_b(0);
    }
    // void removeSlimeDecals()
    //   thunk 0xED510  (impl 0xD1890)
    inline void removeSlimeDecals(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xED510, _gb);
    }
    // bool requestDeployTrap()
    //   thunk 0xEE150  (via call)
    inline bool requestDeployTrap(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0xEE150, _gb);
        return _gb.get_b(0);
    }
    // void requestTorpedo()
    //   thunk 0xEE120  (impl 0xEC540)
    inline void requestTorpedo(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xEE120, _gb);
    }
    // void reviveSquadmates()
    //   thunk 0xED420  (impl 0xD0E00)
    inline void reviveSquadmates(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xED420, _gb);
    }
    // void setAINodeActivationSource(@CActor a, float radius)
    //   thunk 0xEDFB0  (impl 0xEA940)
    inline void setAINodeActivationSource(void* self, void* a, float radius) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        _gb.set_f(2, radius);
        vm::call(0xEDFB0, _gb);
    }
    // void setActorToFollow(@CActor a)
    //   thunk 0xEDFE0  (impl 0xEA960)
    inline void setActorToFollow(void* self, void* a) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, a);
        vm::call(0xEDFE0, _gb);
    }
    // void setBlockCaptureStreamButton(bool flag)
    //   thunk 0xED6C0  (impl 0xDD060)
    inline void setBlockCaptureStreamButton(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0xED6C0, _gb);
    }
    // void setBlockMeterButton(bool flag)
    //   thunk 0xED690  (impl 0xDD050)
    inline void setBlockMeterButton(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0xED690, _gb);
    }
    // void setBlockSlamButton(bool flag)
    //   thunk 0xED6F0  (impl 0xDD070)
    inline void setBlockSlamButton(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0xED6F0, _gb);
    }
    // void setBlockWeaponEquipButton(bool flag)
    //   thunk 0xED720  (impl 0xDD080)
    inline void setBlockWeaponEquipButton(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0xED720, _gb);
    }
    // void setCommandContain(@CActor containActor)
    //   thunk 0xEE190  (impl 0xEC5E0)
    inline void setCommandContain(void* self, void* containActor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, containActor);
        vm::call(0xEE190, _gb);
    }
    // void setCommandCrossBeam()
    //   thunk 0xEE1F0  (impl 0xEC640)
    inline void setCommandCrossBeam(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xEE1F0, _gb);
    }
    // void setCommandTetherObject(vector wDest, vector wStart, vector wEnd)
    //   thunk 0xEE220  (via call)
    inline void setCommandTetherObject(void* self, Vector3 wDest, Vector3 wStart, Vector3 wEnd) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wDest);
        _gb.set_v(4, wStart);
        _gb.set_v(7, wEnd);
        vm::call(0xEE220, _gb);
    }
    // void setContainmentViewerAvailable()
    //   thunk 0xEDCB0  (via none)
    inline void setContainmentViewerAvailable(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xEDCB0, _gb);
    }
    // void setElevator(@CElevator elevator)
    //   thunk 0xEE1C0  (impl 0xEC620)
    inline void setElevator(void* self, void* elevator) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, elevator);
        vm::call(0xEE1C0, _gb);
    }
    // void setEvasionModeEnable(bool enable)
    //   thunk 0xED830  (impl 0xE1890)
    inline void setEvasionModeEnable(void* self, bool enable) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, enable);
        vm::call(0xED830, _gb);
    }
    // void setFacialExpression(EGhostbusterFacialExpression newExpression)
    //   thunk 0xED350  (impl 0xCD370)
    inline void setFacialExpression(void* self, int newExpression) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, newExpression);
        vm::call(0xED350, _gb);
    }
    // void setFlashlightMode(EFlashlightMode newMode)
    //   thunk 0xED9D0  (impl 0xE3C10)
    inline void setFlashlightMode(void* self, int newMode) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, newMode);
        vm::call(0xED9D0, _gb);
    }
    // void setGoggleLocation(EGoggles location)
    //   thunk 0xED570  (impl 0xD50E0)
    inline void setGoggleLocation(void* self, int location) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, location);
        vm::call(0xED570, _gb);
    }
    // void setHauntedModeEnable(bool enable)
    //   thunk 0xED9A0  (impl 0xE2260)
    inline void setHauntedModeEnable(void* self, bool enable) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, enable);
        vm::call(0xED9A0, _gb);
    }
    // void setNothingEquipped(bool flag)
    //   thunk 0xEDAF0  (impl 0xE45A0)
    inline void setNothingEquipped(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0xEDAF0, _gb);
    }
    // void setPKEMeterAmbientNoiseLevel(float pct)
    //   thunk 0xEDC40  (impl 0xE4CD0)
    inline void setPKEMeterAmbientNoiseLevel(void* self, float pct) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, pct);
        vm::call(0xEDC40, _gb);
    }
    // void setRappelModeEnable(bool enable)
    //   thunk 0xED940  (impl 0xE1F70)
    inline void setRappelModeEnable(void* self, bool enable) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, enable);
        vm::call(0xED940, _gb);
    }
    // void slamGoggleLocation(EGoggles location)
    //   thunk 0xED5A0  (impl 0xD51D0)
    inline void slamGoggleLocation(void* self, int location) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, location);
        vm::call(0xED5A0, _gb);
    }
    // bool slamInventoryItem(EInventoryItem itemToSwitchTo)
    //   thunk 0xEDA80  (via call)
    inline bool slamInventoryItem(void* self, int itemToSwitchTo) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, itemToSwitchTo);
        vm::call(0xEDA80, _gb);
        return _gb.get_b(0);
    }
    // void slamWeaponFlags(int secretFlagCode)
    //   thunk 0xEDCD0  (impl 0xE59F0)
    inline void slamWeaponFlags(void* self, int secretFlagCode) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, secretFlagCode);
        vm::call(0xEDCD0, _gb);
    }
    // void slimeMe(bool fromFront, float decalDuration)
    //   thunk 0xED4B0  (impl 0xD0F50)
    inline void slimeMe(void* self, bool fromFront, float decalDuration) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, fromFront);
        _gb.set_f(2, decalDuration);
        vm::call(0xED4B0, _gb);
    }
    // void slimeMeKnockdown(@SDamageInfo info, float decalDuration)
    //   thunk 0xED4E0  (impl 0xD1490)
    inline void slimeMeKnockdown(void* self, void* info, float decalDuration) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        _gb.set_f(2, decalDuration);
        vm::call(0xED4E0, _gb);
    }
    // void startEvasionSequence(@SEvasionInfo info)
    //   thunk 0xED8A0  (impl 0xE1950)
    inline void startEvasionSequence(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0xED8A0, _gb);
    }
    // void startFakePackOverheat()
    //   thunk 0xED750  (impl 0xDD090)
    inline void startFakePackOverheat(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xED750, _gb);
    }
    // void startRappelSwing()
    //   thunk 0xED970  (impl 0xE21E0)
    inline void startRappelSwing(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xED970, _gb);
    }
    // void startTutorial(bool pauseGame, bool letGunRemainActive = false)
    //   thunk 0xEE050  (impl 0xEBCC0)
    inline void startTutorial(void* self, bool pauseGame, bool letGunRemainActive = false) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, pauseGame);
        _gb.set_b(2, letGunRemainActive);
        vm::call(0xEE050, _gb);
    }
    // void toggleHuntMode(bool enable)
    //   thunk 0xED480  (impl 0xD0F30)
    inline void toggleHuntMode(void* self, bool enable) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, enable);
        vm::call(0xED480, _gb);
    }
    // void toggleReviveMode(bool enable)
    //   thunk 0xED450  (impl 0xD0EE0)
    inline void toggleReviveMode(void* self, bool enable) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, enable);
        vm::call(0xED450, _gb);
    }
    // void transferHeroshipTo(@CGhostbuster whom)
    //   thunk 0xED600  (impl 0xD81A0)
    inline void transferHeroshipTo(void* self, void* whom) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, whom);
        vm::call(0xED600, _gb);
    }
    // void transferRayshipTo(@CGhostbuster whom)
    //   thunk 0xED630  (impl 0xD8290)
    inline void transferRayshipTo(void* self, void* whom) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, whom);
        vm::call(0xED630, _gb);
    }
    // void vibrateController(float strength, float vibeTime)
    //   thunk 0xED780  (impl 0xDD0B0)
    inline void vibrateController(void* self, float strength, float vibeTime) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, strength);
        _gb.set_f(2, vibeTime);
        vm::call(0xED780, _gb);
    }
    // void warpToActorSeamless(@CActor destActor)
    //   thunk 0xED380  (impl 0xCDA20)
    inline void warpToActorSeamless(void* self, void* destActor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, destActor);
        vm::call(0xED380, _gb);
    }
} // namespace CGhostbuster

// ------------------------------------------------------------ CGlass
namespace CGlass {
    // bool isBroken()
    //   thunk 0x49B930  (via none)
    inline bool isBroken(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x49B930, _gb);
        return _gb.get_b(0);
    }
    // void shatter(Vector wShatterPos)
    //   thunk 0x49B8D0  (via call)
    inline void shatter(void* self, Vector3 wShatterPos) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wShatterPos);
        vm::call(0x49B8D0, _gb);
    }
} // namespace CGlass

// -------------------------------------------------------- CGolemBase
namespace CGolemBase {
    // void warpGolem(@CActor destActor)
    //   thunk 0x104860  (impl 0x1005C0)
    inline void warpGolem(void* self, void* destActor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, destActor);
        vm::call(0x104860, _gb);
    }
} // namespace CGolemBase

// -------------------------------------------------------- CGrabbable
namespace CGrabbable {
    // void addMembraneFx(float duration)
    //   thunk 0x107BA0  (impl 0x107B90)
    inline void addMembraneFx(void* self, float duration) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, duration);
        vm::call(0x107BA0, _gb);
    }
} // namespace CGrabbable

// ----------------------------------------------------- CGravityShift
namespace CGravityShift {
    // void setSpeed(float period = 8.0f)
    //   thunk 0x108A90  (impl 0x108990)
    inline void setSpeed(void* self, float period = 8.0f) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, period);
        vm::call(0x108A90, _gb);
    }
    // void startShift()
    //   thunk 0x108A30  (impl 0x108240)
    inline void startShift(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x108A30, _gb);
    }
    // void stopShift()
    //   thunk 0x108A60  (impl 0x108980)
    inline void stopShift(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x108A60, _gb);
    }
} // namespace CGravityShift

// ------------------------------------------------------- CHingeActor
namespace CHingeActor {
    // void alignHingeAxis(vector iHingeAxis)
    //   thunk 0x4906D0  (via call)
    inline void alignHingeAxis(void* self, Vector3 iHingeAxis) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, iHingeAxis);
        vm::call(0x4906D0, _gb);
    }
    // bool atMaximumRotation()
    //   thunk 0x490730  (via call)
    inline bool atMaximumRotation(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x490730, _gb);
        return _gb.get_b(0);
    }
    // bool atMinimumRotation()
    //   thunk 0x4907C0  (via call)
    inline bool atMinimumRotation(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x4907C0, _gb);
        return _gb.get_b(0);
    }
    // void breakJoint()
    //   thunk 0x490850  (impl 0x490BB0)
    inline void breakJoint(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x490850, _gb);
    }
    // float getAngle()
    //   thunk 0x4908D0  (via call)
    inline float getAngle(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x4908D0, _gb);
        return _gb.get_f(0);
    }
    // float getAngleRate()
    //   thunk 0x490870  (via call)
    inline float getAngleRate(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x490870, _gb);
        return _gb.get_f(0);
    }
    // void setJointFriction(float value)
    //   thunk 0x490930  (via call)
    inline void setJointFriction(void* self, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, value);
        vm::call(0x490930, _gb);
    }
    // void setMaximumRotation(float angleInDegrees)
    //   thunk 0x490990  (impl 0x395D70)
    inline void setMaximumRotation(void* self, float angleInDegrees) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, angleInDegrees);
        vm::call(0x490990, _gb);
    }
    // void setMinimumRotation(float angleInDegrees)
    //   thunk 0x4909D0  (impl 0x395D70)
    inline void setMinimumRotation(void* self, float angleInDegrees) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, angleInDegrees);
        vm::call(0x4909D0, _gb);
    }
} // namespace CHingeActor

// ------------------------------------------------------------ CHuman
namespace CHuman {
    // void blockGetUp(bool flag)
    //   thunk 0x10F1E0  (impl 0x10BE70)
    inline void blockGetUp(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x10F1E0, _gb);
    }
    // void gesture(EGestureChannel gestureChannel, string animationName)
    //   thunk 0x10F210  (impl 0x10C170)
    inline void gesture(void* self, int gestureChannel, const char* animationName) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, gestureChannel);
        _gb.set_p(2, animationName);
        vm::call(0x10F210, _gb);
    }
    // void gestureStop(EGestureChannel gestureChannel)
    //   thunk 0x10F240  (impl 0x10C230)
    inline void gestureStop(void* self, int gestureChannel) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, gestureChannel);
        vm::call(0x10F240, _gb);
    }
    // bool isEquipped(EInventoryItem which)
    //   thunk 0x10F270  (via call)
    inline bool isEquipped(void* self, int which) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, which);
        vm::call(0x10F270, _gb);
        return _gb.get_b(0);
    }
    // void queueTalkingAnimation(string animationName)
    //   thunk 0x10F310  (impl 0x10F190)
    inline void queueTalkingAnimation(void* self, const char* animationName) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, animationName);
        vm::call(0x10F310, _gb);
    }
    // void resetPhysicsAfterSitting()
    //   thunk 0x10F2E0  (impl 0x10D040)
    inline void resetPhysicsAfterSitting(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x10F2E0, _gb);
    }
    // void setCommandFlee(@CActor fleePoint)
    //   thunk 0x10F2B0  (impl 0x10CF50)
    inline void setCommandFlee(void* self, void* fleePoint) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, fleePoint);
        vm::call(0x10F2B0, _gb);
    }
} // namespace CHuman

// --------------------------------------------------------- CLevitate
namespace CLevitate {
    // void reRandomize()
    //   thunk 0x112390  (impl 0x111FC0)
    inline void reRandomize(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x112390, _gb);
    }
    // void setSpeed(float fps = 2.0f)
    //   thunk 0x112360  (impl 0x111FA0)
    inline void setSpeed(void* self, float fps = 2.0f) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, fps);
        vm::call(0x112360, _gb);
    }
    // void startLevitation(bool noDelays = false)
    //   thunk 0x112300  (impl 0x1113C0)
    inline void startLevitation(void* self, bool noDelays = false) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, noDelays);
        vm::call(0x112300, _gb);
    }
    // void startProcessing()
    //   thunk 0x1122D0  (via indirect)
    inline void startProcessing(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x1122D0, _gb);
    }
    // void stopLevitation()
    //   thunk 0x112330  (impl 0x111F20)
    inline void stopLevitation(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x112330, _gb);
    }
} // namespace CLevitate

// -------------------------------------------------------- CLibrarian
namespace CLibrarian {
    // void removeLayer()
    //   thunk 0x1176E0  (impl 0x115930)
    inline void removeLayer(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x1176E0, _gb);
    }
} // namespace CLibrarian

// --------------------------------------------------------- CLiteBulb
namespace CLiteBulb {
    // void activate(bool flag)
    //   thunk 0x4A0DD0  (impl 0x4A0E40)
    inline void activate(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x4A0DD0, _gb);
    }
    // void activateFlicker(bool flag)
    //   thunk 0x4A0DA0  (via none)
    inline void activateFlicker(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x4A0DA0, _gb);
    }
} // namespace CLiteBulb

// --------------------------------------------------- CMarshmallowGoo
namespace CMarshmallowGoo {
    // bool isInFlight()
    //   thunk 0x11AE90  (via call)
    inline bool isInFlight(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x11AE90, _gb);
        return _gb.get_b(0);
    }
    // void launchAtTarget(vector target, float flightTime)
    //   thunk 0x11AE30  (via call)
    inline void launchAtTarget(void* self, Vector3 target, float flightTime) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, target);
        _gb.set_f(4, flightTime);
        vm::call(0x11AE30, _gb);
    }
} // namespace CMarshmallowGoo

// --------------------------------------------------- CMaterialSystem
namespace CMaterialSystem {
    // void getMaterialAnimation(string name, E2DTransformControllerSlot transformSlot, @SWaveformControl wave)
    //   thunk 0x2D8160  (via none)
    inline void getMaterialAnimation(void* self, const char* name, int transformSlot, void* wave) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, name);
        _gb.set_i(2, transformSlot);
        _gb.set_p(3, wave);
        vm::call(0x2D8160, _gb);
    }
    // void registerVariable2D(string name, @SMaterialVariable var)
    //   thunk 0x2D8180  (impl 0x2F6BE0)
    inline void registerVariable2D(void* self, const char* name, void* var) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, name);
        _gb.set_p(2, var);
        vm::call(0x2D8180, _gb);
    }
    // void registerVariable3D(string name, @SMaterialVariable var)
    //   thunk 0x2D81E0  (impl 0x2F6BE0)
    inline void registerVariable3D(void* self, const char* name, void* var) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, name);
        _gb.set_p(2, var);
        vm::call(0x2D81E0, _gb);
    }
    // void registerVariable4D(string name, @SMaterialVariable var)
    //   thunk 0x2D8240  (impl 0x2F6BE0)
    inline void registerVariable4D(void* self, const char* name, void* var) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, name);
        _gb.set_p(2, var);
        vm::call(0x2D8240, _gb);
    }
    // void registerVariableScalar(string name, @SMaterialVariable var)
    //   thunk 0x2D82A0  (impl 0x2F6BE0)
    inline void registerVariableScalar(void* self, const char* name, void* var) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, name);
        _gb.set_p(2, var);
        vm::call(0x2D82A0, _gb);
    }
    // void setMaterialAnimation(string name, E2DTransformControllerSlot transformSlot, @SWaveformControl wave)
    //   thunk 0x2D8300  (via none)
    inline void setMaterialAnimation(void* self, const char* name, int transformSlot, void* wave) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, name);
        _gb.set_i(2, transformSlot);
        _gb.set_p(3, wave);
        vm::call(0x2D8300, _gb);
    }
    // void setVariable2D(@SMaterialVariable var, float x, float y)
    //   thunk 0x2D8320  (impl 0x2E1760)
    inline void setVariable2D(void* self, void* var, float x, float y) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, var);
        _gb.set_f(2, x);
        _gb.set_f(3, y);
        vm::call(0x2D8320, _gb);
    }
    // void setVariable3D(@SMaterialVariable var, float x, float y, float z)
    //   thunk 0x2D8350  (via call)
    inline void setVariable3D(void* self, void* var, float x, float y, float z) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, var);
        _gb.set_f(2, x);
        _gb.set_f(3, y);
        _gb.set_f(4, z);
        vm::call(0x2D8350, _gb);
    }
    // void setVariable4D(@SMaterialVariable var, float x, float y, float z, float w)
    //   thunk 0x2D83A0  (via call)
    inline void setVariable4D(void* self, void* var, float x, float y, float z, float w) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, var);
        _gb.set_f(2, x);
        _gb.set_f(3, y);
        _gb.set_f(4, z);
        _gb.set_f(5, w);
        vm::call(0x2D83A0, _gb);
    }
    // void setVariableScalar(@SMaterialVariable var, float value)
    //   thunk 0x2D83F0  (impl 0x2E1980)
    inline void setVariableScalar(void* self, void* var, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, var);
        _gb.set_f(2, value);
        vm::call(0x2D83F0, _gb);
    }
} // namespace CMaterialSystem

// ----------------------------------------------------- CMediumPeople
namespace CMediumPeople {
    // void addFollower(@CActor child)
    //   thunk 0x1242E0  (impl 0x11EBC0)
    inline void addFollower(void* self, void* child) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, child);
        vm::call(0x1242E0, _gb);
    }
    // void findVantagePoint(float pct)
    //   thunk 0x1241F0  (impl 0x11DE60)
    inline void findVantagePoint(void* self, float pct) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, pct);
        vm::call(0x1241F0, _gb);
    }
    // void findVantagePointBarrier(float pct)
    //   thunk 0x124220  (impl 0x11E2B0)
    inline void findVantagePointBarrier(void* self, float pct) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, pct);
        vm::call(0x124220, _gb);
    }
    // void flee(float pctFlee, float pctRun)
    //   thunk 0x124280  (impl 0x11E8C0)
    inline void flee(void* self, float pctFlee, float pctRun) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, pctFlee);
        _gb.set_f(2, pctRun);
        vm::call(0x124280, _gb);
    }
    // void setFollowerHeight(float h)
    //   thunk 0x124310  (impl 0x11ECE0)
    inline void setFollowerHeight(void* self, float h) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, h);
        vm::call(0x124310, _gb);
    }
    // void startGawking(float pct)
    //   thunk 0x1242B0  (impl 0x11EB20)
    inline void startGawking(void* self, float pct) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, pct);
        vm::call(0x1242B0, _gb);
    }
    // void stopGawking()
    //   thunk 0x124250  (impl 0x11E800)
    inline void stopGawking(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x124250, _gb);
    }
    // void walkBackAndForth(float pct)
    //   thunk 0x1241C0  (impl 0x11DDA0)
    inline void walkBackAndForth(void* self, float pct) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, pct);
        vm::call(0x1241C0, _gb);
    }
} // namespace CMediumPeople

// -------------------------------------------------- CMinionGenerator
namespace CMinionGenerator {
    // bool isBurnt()
    //   thunk 0x125ED0  (via call)
    inline bool isBurnt(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x125ED0, _gb);
        return _gb.get_b(0);
    }
} // namespace CMinionGenerator

// -------------------------------------------------------------- CNPC
namespace CNPC {
    // bool isRescued()
    //   thunk 0x12F350  (via call)
    inline bool isRescued(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x12F350, _gb);
        return _gb.get_b(0);
    }
    // void rescued()
    //   thunk 0x12F330  (via none)
    inline void rescued(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x12F330, _gb);
    }
    // void setCowerMode(bool flag)
    //   thunk 0x12F3F0  (impl 0x12F2D0)
    inline void setCowerMode(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x12F3F0, _gb);
    }
    // void setFleeMode(bool flag)
    //   thunk 0x12F3C0  (impl 0x12EF20)
    inline void setFleeMode(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x12F3C0, _gb);
    }
    // void setPanicMode(bool flag)
    //   thunk 0x12F390  (impl 0x12EF00)
    inline void setPanicMode(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x12F390, _gb);
    }
} // namespace CNPC

// ---------------------------------------------------------- CNavMesh
namespace CNavMesh {
    // void enablePart(String partName, bool enable)
    //   thunk 0x43EE90  (impl 0x4438B0)
    inline void enablePart(void* self, const char* partName, bool enable) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, partName);
        _gb.set_b(2, enable);
        vm::call(0x43EE90, _gb);
    }
    // bool isPartEnabled(String partName)
    //   thunk 0x43EEC0  (via call)
    inline bool isPartEnabled(void* self, const char* partName) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, partName);
        vm::call(0x43EEC0, _gb);
        return _gb.get_b(0);
    }
    // void setPartTerrainType(String partName, @STerrainTypeInfo info)
    //   thunk 0x43EF50  (impl 0x449E40)
    inline void setPartTerrainType(void* self, const char* partName, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, partName);
        _gb.set_p(2, info);
        vm::call(0x43EF50, _gb);
    }
} // namespace CNavMesh

// -------------------------------------------------------- CNightmare
namespace CNightmare {
    // void setInMirror(bool flag)
    //   thunk 0x12B9C0  (impl 0x12B970)
    inline void setInMirror(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x12B9C0, _gb);
    }
} // namespace CNightmare

// ------------------------------------------------------- COrreryBoss
namespace COrreryBoss {
    // void releaseTheBeast()
    //   thunk 0x132960  (impl 0x1310F0)
    inline void releaseTheBeast(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x132960, _gb);
    }
} // namespace COrreryBoss

// -------------------------------------------------------- CPathPoint
namespace CPathPoint {
    // void raiseContextCallout(@CCharacter actor)
    //   thunk 0x133F00  (via none)
    inline void raiseContextCallout(void* actor) {
        vm::Block _gb;
        _gb.set_p(0, actor);
        vm::call(0x133F00, _gb);
    }
} // namespace CPathPoint

// ------------------------------------------------------- CPedestrian
namespace CPedestrian {
    // void setFleeDist(float newFleeDist)
    //   thunk 0x134510  (impl 0x1344F0)
    inline void setFleeDist(void* self, float newFleeDist) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, newFleeDist);
        vm::call(0x134510, _gb);
    }
} // namespace CPedestrian

// --------------------------------------------------------- CPendulum
namespace CPendulum {
    // void start()
    //   thunk 0x135560  (impl 0x135520)
    inline void start(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x135560, _gb);
    }
    // void stop()
    //   thunk 0x135590  (impl 0x135540)
    inline void stop(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x135590, _gb);
    }
} // namespace CPendulum

// ---------------------------------------------------- CPhysicsObject
namespace CPhysicsObject {
    // void addVelocity(vector velocity)
    //   thunk 0x13A3F0  (via call)
    inline void addVelocity(void* self, Vector3 velocity) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, velocity);
        vm::call(0x13A3F0, _gb);
    }
    // void addVelocityAtPoint(vector velocity, vector wImpactPos)
    //   thunk 0x13A450  (via call)
    inline void addVelocityAtPoint(void* self, Vector3 velocity, Vector3 wImpactPos) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, velocity);
        _gb.set_v(4, wImpactPos);
        vm::call(0x13A450, _gb);
    }
    // void addWobble(float duration)
    //   thunk 0x13A570  (via indirect)
    inline void addWobble(void* self, float duration) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, duration);
        vm::call(0x13A570, _gb);
    }
    // void launchAtTarget(vector target, float flightTime)
    //   thunk 0x13A4D0  (via indirect)
    inline void launchAtTarget(void* self, Vector3 target, float flightTime) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, target);
        _gb.set_f(4, flightTime);
        vm::call(0x13A4D0, _gb);
    }
    // void setAIObstacle(bool isObstacle)
    //   thunk 0x13A5A0  (via indirect)
    inline void setAIObstacle(void* self, bool isObstacle) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, isObstacle);
        vm::call(0x13A5A0, _gb);
    }
    // void setSnareEnable(bool enable)
    //   thunk 0x13A540  (via indirect)
    inline void setSnareEnable(void* self, bool enable) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, enable);
        vm::call(0x13A540, _gb);
    }
} // namespace CPhysicsObject

// ------------------------------------------------ CPhysicsObjectBase
namespace CPhysicsObjectBase {
    // float getHitPoints()
    //   thunk 0x360520  (via none)
    inline float getHitPoints(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x360520, _gb);
        return _gb.get_f(0);
    }
    // float getHitPointsPct()
    //   thunk 0x3604E0  (via none)
    inline float getHitPointsPct(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x3604E0, _gb);
        return _gb.get_f(0);
    }
    // void lockInPlace(bool lock)
    //   thunk 0x360550  (via call)
    inline void lockInPlace(void* self, bool lock) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, lock);
        vm::call(0x360550, _gb);
    }
    // void removeAllJoints()
    //   thunk 0x3605B0  (via indirect)
    inline void removeAllJoints(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x3605B0, _gb);
    }
    // void removeJoint(@Joint joint)
    //   thunk 0x3605E0  (via indirect)
    inline void removeJoint(void* self, void* joint) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, joint);
        vm::call(0x3605E0, _gb);
    }
    // void wakeUp()
    //   thunk 0x360610  (impl 0x3674D0)
    inline void wakeUp(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x360610, _gb);
    }
} // namespace CPhysicsObjectBase

// ----------------------------------------------------------- CPhysmo
namespace CPhysmo {
    // void cancelMove(bool linear = true, bool angular = true)
    //   thunk 0x13FC70  (impl 0x13EA80)
    inline void cancelMove(void* self, bool linear = true, bool angular = true) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, linear);
        _gb.set_b(2, angular);
        vm::call(0x13FC70, _gb);
    }
    // void enforceRotationRate(bool yah)
    //   thunk 0x13FE80  (impl 0x13EF30)
    inline void enforceRotationRate(void* self, bool yah) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        vm::call(0x13FE80, _gb);
    }
    // void enforceSpeed(bool yah)
    //   thunk 0x13FE50  (impl 0x13EF20)
    inline void enforceSpeed(void* self, bool yah) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        vm::call(0x13FE50, _gb);
    }
    // vector getAMotorPosition()
    //   thunk 0x13FD40  (via call)
    inline Vector3 getAMotorPosition(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x13FD40, _gb);
        return _gb.get_v(0);
    }
    // vector getCompletionOrientationError()
    //   thunk 0x13FFF0  (via call)
    inline Vector3 getCompletionOrientationError(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x13FFF0, _gb);
        return _gb.get_v(0);
    }
    // vector getCompletionPositionError()
    //   thunk 0x13FFA0  (via call)
    inline Vector3 getCompletionPositionError(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x13FFA0, _gb);
        return _gb.get_v(0);
    }
    // vector getLMotorPosition()
    //   thunk 0x13FCF0  (via call)
    inline Vector3 getLMotorPosition(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x13FCF0, _gb);
        return _gb.get_v(0);
    }
    // vector getTetherAcceleration()
    //   thunk 0x1401F0  (via call)
    inline Vector3 getTetherAcceleration(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x1401F0, _gb);
        return _gb.get_v(0);
    }
    // void massCenterBasedNavigation()
    //   thunk 0x13FE20  (impl 0x13EF00)
    inline void massCenterBasedNavigation(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x13FE20, _gb);
    }
    // void modelOriginBasedNavigation()
    //   thunk 0x13FDF0  (impl 0x13EEE0)
    inline void modelOriginBasedNavigation(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x13FDF0, _gb);
    }
    // void moveFromPosition(vector wPos, float spd)
    //   thunk 0x13FB40  (via call)
    inline void moveFromPosition(void* self, Vector3 wPos, float spd) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wPos);
        _gb.set_f(4, spd);
        vm::call(0x13FB40, _gb);
    }
    // void moveToActor(@CActor actor, float spd, bool stop = true, bool gitErDone = false)
    //   thunk 0x13FCA0  (via call)
    inline void moveToActor(void* self, void* actor, float spd, bool stop = true, bool gitErDone = false) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, actor);
        _gb.set_f(2, spd);
        _gb.set_b(3, stop);
        _gb.set_b(4, gitErDone);
        vm::call(0x13FCA0, _gb);
    }
    // void moveToOrientation(vector wAng, float spd = 360.0f, bool stop = true)
    //   thunk 0x13FBA0  (via call)
    inline void moveToOrientation(void* self, Vector3 wAng, float spd = 360.0f, bool stop = true) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wAng);
        _gb.set_f(4, spd);
        _gb.set_b(5, stop);
        vm::call(0x13FBA0, _gb);
    }
    // void moveToPosition(vector wPos, float spd, bool stop = true, bool gitErDone = false)
    //   thunk 0x13FAD0  (via call)
    inline void moveToPosition(void* self, Vector3 wPos, float spd, bool stop = true, bool gitErDone = false) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wPos);
        _gb.set_f(4, spd);
        _gb.set_b(5, stop);
        _gb.set_b(6, gitErDone);
        vm::call(0x13FAD0, _gb);
    }
    // void notifyCompletionWhenNearActor(@CActor actor, float nearEnuf)
    //   thunk 0x13FF70  (impl 0x13EFC0)
    inline void notifyCompletionWhenNearActor(void* self, void* actor, float nearEnuf) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, actor);
        _gb.set_f(2, nearEnuf);
        vm::call(0x13FF70, _gb);
    }
    // void notifyCompletionWhenNearOrientation(vector wAng, float nearEnuf)
    //   thunk 0x13FF10  (via call)
    inline void notifyCompletionWhenNearOrientation(void* self, Vector3 wAng, float nearEnuf) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wAng);
        _gb.set_f(4, nearEnuf);
        vm::call(0x13FF10, _gb);
    }
    // void notifyCompletionWhenNearPosition(vector wPos, float nearEnuf)
    //   thunk 0x13FEB0  (via call)
    inline void notifyCompletionWhenNearPosition(void* self, Vector3 wPos, float nearEnuf) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wPos);
        _gb.set_f(4, nearEnuf);
        vm::call(0x13FEB0, _gb);
    }
    // void resetMotorParameters(bool linear = true, bool angular = true)
    //   thunk 0x13F5A0  (impl 0x13D930)
    inline void resetMotorParameters(void* self, bool linear = true, bool angular = true) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, linear);
        _gb.set_b(2, angular);
        vm::call(0x13F5A0, _gb);
    }
    // void setAMotorHiStop(vector hiStop)
    //   thunk 0x13F8B0  (via call)
    inline void setAMotorHiStop(void* self, Vector3 hiStop) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, hiStop);
        vm::call(0x13F8B0, _gb);
    }
    // void setAMotorHiStops(float p, float h, float b)
    //   thunk 0x13FA10  (impl 0x13E490)
    inline void setAMotorHiStops(void* self, float p, float h, float b) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, p);
        _gb.set_f(2, h);
        _gb.set_f(3, b);
        vm::call(0x13FA10, _gb);
    }
    // void setAMotorLoStop(vector loStop)
    //   thunk 0x13F850  (via call)
    inline void setAMotorLoStop(void* self, Vector3 loStop) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, loStop);
        vm::call(0x13F850, _gb);
    }
    // void setAMotorLoStops(float p, float h, float b)
    //   thunk 0x13F9D0  (impl 0x13E3A0)
    inline void setAMotorLoStops(void* self, float p, float h, float b) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, p);
        _gb.set_f(2, h);
        _gb.set_f(3, b);
        vm::call(0x13F9D0, _gb);
    }
    // void setAMotorRate(vector rate)
    //   thunk 0x13F970  (via call)
    inline void setAMotorRate(void* self, Vector3 rate) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, rate);
        vm::call(0x13F970, _gb);
    }
    // void setAMotorRates(float p, float h, float b)
    //   thunk 0x13FA90  (impl 0x13E5E0)
    inline void setAMotorRates(void* self, float p, float h, float b) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, p);
        _gb.set_f(2, h);
        _gb.set_f(3, b);
        vm::call(0x13FA90, _gb);
    }
    // void setAMotorStopsToCurrentPosition()
    //   thunk 0x13FDC0  (impl 0x13EE40)
    inline void setAMotorStopsToCurrentPosition(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x13FDC0, _gb);
    }
    // void setAMotorTorque(vector torque)
    //   thunk 0x13F910  (via call)
    inline void setAMotorTorque(void* self, Vector3 torque) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, torque);
        vm::call(0x13F910, _gb);
    }
    // void setAMotorTorques(float p, float h, float b)
    //   thunk 0x13FA50  (impl 0x13E550)
    inline void setAMotorTorques(void* self, float p, float h, float b) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, p);
        _gb.set_f(2, h);
        _gb.set_f(3, b);
        vm::call(0x13FA50, _gb);
    }
    // void setBreakWhenSnareForceIsExceeded(bool yah, float snareforce = 100.0f)
    //   thunk 0x1401C0  (impl 0x13F4A0)
    inline void setBreakWhenSnareForceIsExceeded(void* self, bool yah, float snareforce = 100.0f) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        _gb.set_f(2, snareforce);
        vm::call(0x1401C0, _gb);
    }
    // void setCameraPath(@CCameraPathActor path, float timeTag=0.0)
    //   thunk 0x140040  (impl 0x13F060)
    inline void setCameraPath(void* self, void* path, float timeTag = 0.0) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, path);
        _gb.set_f(2, timeTag);
        vm::call(0x140040, _gb);
    }
    // void setCameraPathForce(float force)
    //   thunk 0x140070  (impl 0x13F2D0)
    inline void setCameraPathForce(void* self, float force) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, force);
        vm::call(0x140070, _gb);
    }
    // void setCameraPathSpeed(float speed)
    //   thunk 0x1400A0  (impl 0x13F2F0)
    inline void setCameraPathSpeed(void* self, float speed) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, speed);
        vm::call(0x1400A0, _gb);
    }
    // void setGhostbusterCanRide(bool yah)
    //   thunk 0x140190  (impl 0x13F490)
    inline void setGhostbusterCanRide(void* self, bool yah) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yah);
        vm::call(0x140190, _gb);
    }
    // void setLMotorForce(vector force)
    //   thunk 0x13F690  (via call)
    inline void setLMotorForce(void* self, Vector3 force) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, force);
        vm::call(0x13F690, _gb);
    }
    // void setLMotorForces(float x, float y, float z)
    //   thunk 0x13F7D0  (impl 0x13E000)
    inline void setLMotorForces(void* self, float x, float y, float z) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, x);
        _gb.set_f(2, y);
        _gb.set_f(3, z);
        vm::call(0x13F7D0, _gb);
    }
    // void setLMotorHiStop(vector hiStop)
    //   thunk 0x13F630  (via call)
    inline void setLMotorHiStop(void* self, Vector3 hiStop) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, hiStop);
        vm::call(0x13F630, _gb);
    }
    // void setLMotorHiStops(float x, float y, float z)
    //   thunk 0x13F790  (impl 0x13DF50)
    inline void setLMotorHiStops(void* self, float x, float y, float z) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, x);
        _gb.set_f(2, y);
        _gb.set_f(3, z);
        vm::call(0x13F790, _gb);
    }
    // void setLMotorLoStop(vector loStop)
    //   thunk 0x13F5D0  (via call)
    inline void setLMotorLoStop(void* self, Vector3 loStop) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, loStop);
        vm::call(0x13F5D0, _gb);
    }
    // void setLMotorLoStops(float x, float y, float z)
    //   thunk 0x13F750  (impl 0x13DE80)
    inline void setLMotorLoStops(void* self, float x, float y, float z) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, x);
        _gb.set_f(2, y);
        _gb.set_f(3, z);
        vm::call(0x13F750, _gb);
    }
    // void setLMotorRate(vector rate)
    //   thunk 0x13F6F0  (via call)
    inline void setLMotorRate(void* self, Vector3 rate) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, rate);
        vm::call(0x13F6F0, _gb);
    }
    // void setLMotorRates(float x, float y, float z)
    //   thunk 0x13F810  (impl 0x13E090)
    inline void setLMotorRates(void* self, float x, float y, float z) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, x);
        _gb.set_f(2, y);
        _gb.set_f(3, z);
        vm::call(0x13F810, _gb);
    }
    // void setLMotorStopsToCurrentPosition()
    //   thunk 0x13FD90  (impl 0x13EDA0)
    inline void setLMotorStopsToCurrentPosition(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x13FD90, _gb);
    }
    // void setTetherable(bool yes = true)
    //   thunk 0x140240  (impl 0x13F580)
    inline void setTetherable(void* self, bool yes = true) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, yes);
        vm::call(0x140240, _gb);
    }
    // void tiltTo(vector iAxis)
    //   thunk 0x13FC10  (via call)
    inline void tiltTo(void* self, Vector3 iAxis) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, iAxis);
        vm::call(0x13FC10, _gb);
    }
} // namespace CPhysmo

// --------------------------------------------------------- CPlatform
namespace CPlatform {
    // int enableParts(string namePattern, bool flag)
    //   thunk 0x3E19C0  (via call)
    inline int enableParts(void* self, const char* namePattern, bool flag) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, namePattern);
        _gb.set_b(3, flag);
        vm::call(0x3E19C0, _gb);
        return _gb.get_i(0);
    }
    // float getMovementRate()
    //   thunk 0x3E1A10  (via none)
    inline float getMovementRate(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x3E1A10, _gb);
        return _gb.get_f(0);
    }
    // float getParameter()
    //   thunk 0x3E1A40  (via none)
    inline float getParameter(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x3E1A40, _gb);
        return _gb.get_f(0);
    }
    // float getPathTotalTime()
    //   thunk 0x3E1A70  (via none)
    inline float getPathTotalTime(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x3E1A70, _gb);
        return _gb.get_f(0);
    }
    // void moveToEnd()
    //   thunk 0x3E1AF0  (impl 0x3E3950)
    inline void moveToEnd(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x3E1AF0, _gb);
    }
    // void moveToKey(int keyIndex, float movementRate)
    //   thunk 0x3E1B10  (impl 0x3E4AE0)
    inline void moveToKey(void* self, int keyIndex, float movementRate) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, keyIndex);
        _gb.set_f(2, movementRate);
        vm::call(0x3E1B10, _gb);
    }
    // void moveToNextKey(float movementRate)
    //   thunk 0x3E1B90  (impl 0x3E3A10)
    inline void moveToNextKey(void* self, float movementRate) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, movementRate);
        vm::call(0x3E1B90, _gb);
    }
    // void moveToPrevKey(float movementRate)
    //   thunk 0x3E1BC0  (impl 0x3E3AC0)
    inline void moveToPrevKey(void* self, float movementRate) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, movementRate);
        vm::call(0x3E1BC0, _gb);
    }
    // void moveToStart()
    //   thunk 0x3E1BF0  (impl 0x3E4AE0)
    inline void moveToStart(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x3E1BF0, _gb);
    }
    // void setMovementRate(float movementRate)
    //   thunk 0x3E1C20  (via none)
    inline void setMovementRate(void* self, float movementRate) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, movementRate);
        vm::call(0x3E1C20, _gb);
    }
    // void setParameter(float param)
    //   thunk 0x3E1C50  (impl 0x3E3BD0)
    inline void setParameter(void* self, float param) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, param);
        vm::call(0x3E1C50, _gb);
    }
    // void startMoving(float goalParam, float movementRate)
    //   thunk 0x3E1C90  (impl 0x3E4AE0)
    inline void startMoving(void* self, float goalParam, float movementRate) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, goalParam);
        _gb.set_f(2, movementRate);
        vm::call(0x3E1C90, _gb);
    }
    // void stopMoving()
    //   thunk 0x3E1CC0  (impl 0x3E4C50)
    inline void stopMoving(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x3E1CC0, _gb);
    }
} // namespace CPlatform

// ------------------------------------------------------ CPortalLight
namespace CPortalLight {
    // void enable(bool flag)
    //   thunk 0x2E3600  (via none)
    inline void enable(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x2E3600, _gb);
    }
    // vector getColor()
    //   thunk 0x2E3630  (via none)
    inline Vector3 getColor(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x2E3630, _gb);
        return _gb.get_v(0);
    }
    // float getIntensity()
    //   thunk 0x2E3690  (via none)
    inline float getIntensity(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2E3690, _gb);
        return _gb.get_f(0);
    }
    // bool isEnabled()
    //   thunk 0x2E36B0  (via none)
    inline bool isEnabled(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2E36B0, _gb);
        return _gb.get_b(0);
    }
    // void setColor(float r, float g, float b)
    //   thunk 0x2E36E0  (impl 0x2E9FC0)
    inline void setColor(void* self, float r, float g, float b) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, r);
        _gb.set_f(2, g);
        _gb.set_f(3, b);
        vm::call(0x2E36E0, _gb);
    }
    // void setIntensity(float intensity)
    //   thunk 0x2E3710  (via none)
    inline void setIntensity(void* self, float intensity) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, intensity);
        vm::call(0x2E3710, _gb);
    }
} // namespace CPortalLight

// ---------------------------------------------------- CPostProcessFx
namespace CPostProcessFx {
    // void enableDOF(bool flag)
    //   thunk 0x41A330  (via none)
    inline void enableDOF(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x41A330, _gb);
    }
    // void enableHDR(bool flag)
    //   thunk 0x41A360  (via none)
    inline void enableHDR(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x41A360, _gb);
    }
    // void reset()
    //   thunk 0x41A390  (impl 0x41D300)
    inline void reset(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x41A390, _gb);
    }
    // void setBloomIntensity(float intensity)
    //   thunk 0x41A3B0  (via none)
    inline void setBloomIntensity(void* self, float intensity) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, intensity);
        vm::call(0x41A3B0, _gb);
    }
    // void setBlueMax(float value)
    //   thunk 0x41A3F0  (via none)
    inline void setBlueMax(void* self, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, value);
        vm::call(0x41A3F0, _gb);
    }
    // void setBlueMin(float value)
    //   thunk 0x41A430  (via none)
    inline void setBlueMin(void* self, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, value);
        vm::call(0x41A430, _gb);
    }
    // void setBlueScale(float value)
    //   thunk 0x41A470  (via none)
    inline void setBlueScale(void* self, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, value);
        vm::call(0x41A470, _gb);
    }
    // void setBrightThreshold(float threshold)
    //   thunk 0x41A4B0  (via none)
    inline void setBrightThreshold(void* self, float threshold) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, threshold);
        vm::call(0x41A4B0, _gb);
    }
    // void setBrightness(float brightness)
    //   thunk 0x41A500  (via none)
    inline void setBrightness(void* self, float brightness) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, brightness);
        vm::call(0x41A500, _gb);
    }
    // void setContrast(float contrast)
    //   thunk 0x41A560  (impl 0x41A5A4)
    inline void setContrast(void* self, float contrast) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, contrast);
        vm::call(0x41A560, _gb);
    }
    // void setDOFPlanes(@SDOFPlanes info)
    //   thunk 0x41A5C0  (via none)
    inline void setDOFPlanes(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0x41A5C0, _gb);
    }
    // void setDOFSettings_Gen2(int startDistance, int step, int opacity)
    //   thunk 0x41A640  (via none)
    inline void setDOFSettings_Gen2(void* self, int startDistance, int step, int opacity) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, startDistance);
        _gb.set_i(2, step);
        _gb.set_i(3, opacity);
        vm::call(0x41A640, _gb);
    }
    // void setEmissive(float v)
    //   thunk 0x41A6D0  (via none)
    inline void setEmissive(void* self, float v) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, v);
        vm::call(0x41A6D0, _gb);
    }
    // void setExposure(float exp)
    //   thunk 0x41A710  (via none)
    inline void setExposure(void* self, float exp) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, exp);
        vm::call(0x41A710, _gb);
    }
    // void setGrainEnable(bool value)
    //   thunk 0x41A750  (via none)
    inline void setGrainEnable(void* self, bool value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, value);
        vm::call(0x41A750, _gb);
    }
    // void setGrainSize(float value)
    //   thunk 0x41A790  (via none)
    inline void setGrainSize(void* self, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, value);
        vm::call(0x41A790, _gb);
    }
    // void setGrainValue(float value)
    //   thunk 0x41A7D0  (via none)
    inline void setGrainValue(void* self, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, value);
        vm::call(0x41A7D0, _gb);
    }
    // void setGreenMax(float value)
    //   thunk 0x41A810  (via none)
    inline void setGreenMax(void* self, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, value);
        vm::call(0x41A810, _gb);
    }
    // void setGreenMin(float value)
    //   thunk 0x41A850  (via none)
    inline void setGreenMin(void* self, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, value);
        vm::call(0x41A850, _gb);
    }
    // void setGreenScale(float value)
    //   thunk 0x41A890  (via none)
    inline void setGreenScale(void* self, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, value);
        vm::call(0x41A890, _gb);
    }
    // void setRedMax(float value)
    //   thunk 0x41A8D0  (via none)
    inline void setRedMax(void* self, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, value);
        vm::call(0x41A8D0, _gb);
    }
    // void setRedMin(float value)
    //   thunk 0x41A910  (via none)
    inline void setRedMin(void* self, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, value);
        vm::call(0x41A910, _gb);
    }
    // void setRedScale(float value)
    //   thunk 0x41A950  (via none)
    inline void setRedScale(void* self, float value) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, value);
        vm::call(0x41A950, _gb);
    }
    // void setSaturation(float v)
    //   thunk 0x41A990  (via none)
    inline void setSaturation(void* self, float v) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, v);
        vm::call(0x41A990, _gb);
    }
} // namespace CPostProcessFx

// -------------------------------------------------------- CRailRider
namespace CRailRider {
    // void setRail(@CRailSpline spline)
    //   thunk 0x14C9C0  (impl 0x14C9A0)
    inline void setRail(void* self, void* spline) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, spline);
        vm::call(0x14C9C0, _gb);
    }
} // namespace CRailRider

// ---------------------------------------------------------- CRailcar
namespace CRailcar {
    // void DisableSteering()
    //   thunk 0x14B810  (impl 0x14B750)
    inline void DisableSteering(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x14B810, _gb);
    }
    // void SetCameraPathActor(@CCameraPathActor path)
    //   thunk 0x14B7E0  (impl 0x14B740)
    inline void SetCameraPathActor(void* self, void* path) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, path);
        vm::call(0x14B7E0, _gb);
    }
    // void SteerFree()
    //   thunk 0x14B930  (impl 0x14B7A0)
    inline void SteerFree(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x14B930, _gb);
    }
    // void SteerToCameraPathPosition()
    //   thunk 0x14B870  (impl 0x14B770)
    inline void SteerToCameraPathPosition(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x14B870, _gb);
    }
    // void SteerToCameraPathPositionAndOrientation()
    //   thunk 0x14B8A0  (impl 0x14B780)
    inline void SteerToCameraPathPositionAndOrientation(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x14B8A0, _gb);
    }
    // void SteerToPosition(vector pos)
    //   thunk 0x14B8D0  (via call)
    inline void SteerToPosition(void* self, Vector3 pos) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, pos);
        vm::call(0x14B8D0, _gb);
    }
    // void Synchronize()
    //   thunk 0x14B840  (impl 0x14B760)
    inline void Synchronize(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x14B840, _gb);
    }
    // int getDamageStage()
    //   thunk 0x14B990  (via call)
    inline int getDamageStage(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x14B990, _gb);
        return _gb.get_i(0);
    }
    // void setDamageStage(int stage)
    //   thunk 0x14B960  (impl 0x14B7C0)
    inline void setDamageStage(void* self, int stage) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, stage);
        vm::call(0x14B960, _gb);
    }
} // namespace CRailcar

// ------------------------------------------------------------- CRoom
namespace CRoom {
    // int enablePart(string partName, bool flag)
    //   thunk 0x2E3750  (via call)
    inline int enablePart(void* self, const char* partName, bool flag) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, partName);
        _gb.set_b(3, flag);
        vm::call(0x2E3750, _gb);
        return _gb.get_i(0);
    }
    // string getName()
    //   thunk 0x2E3790  (impl 0x2E3804)
    inline const char* getName(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2E3790, _gb);
        return static_cast<const char*>(_gb.get_p(0));
    }
} // namespace CRoom

// --------------------------------------------------------- CScuttler
namespace CScuttler {
    // void fillInScuttlerCCSOD(@SScuttlerCombatComponentInfo info)
    //   thunk 0x155AC0  (impl 0x155980)
    inline void fillInScuttlerCCSOD(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0x155AC0, _gb);
    }
    // void setSplinePathActor(@CSplinePath sPath, float startT)
    //   thunk 0x155AF0  (impl 0x155A70)
    inline void setSplinePathActor(void* self, void* sPath, float startT) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, sPath);
        _gb.set_f(2, startT);
        vm::call(0x155AF0, _gb);
    }
} // namespace CScuttler

// ------------------------------------------------------------- CSign
namespace CSign {
    // string getMessage()
    //   thunk 0x4A3140  (impl 0x4A31B4)
    inline const char* getMessage(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x4A3140, _gb);
        return static_cast<const char*>(_gb.get_p(0));
    }
    // void setMessage(string text)
    //   thunk 0x4A31C0  (via call)
    inline void setMessage(void* self, const char* text) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, text);
        vm::call(0x4A31C0, _gb);
    }
} // namespace CSign

// ----------------------------------------------------------- CSkyBox
namespace CSkyBox {
    // void setLightning(float attack, float sustain, float decay)
    //   thunk 0x47C630  (via none)
    inline void setLightning(void* self, float attack, float sustain, float decay) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, attack);
        _gb.set_f(2, sustain);
        _gb.set_f(3, decay);
        vm::call(0x47C630, _gb);
    }
    // void showLayer(int layer, bool show)
    //   thunk 0x47C680  (via none)
    inline void showLayer(void* self, int layer, bool show) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, layer);
        _gb.set_b(2, show);
        vm::call(0x47C680, _gb);
    }
} // namespace CSkyBox

// ------------------------------------------------------ CSliderActor
namespace CSliderActor {
    // bool atMaximumPosition()
    //   thunk 0x493250  (via call)
    inline bool atMaximumPosition(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x493250, _gb);
        return _gb.get_b(0);
    }
    // bool atMinimumPosition()
    //   thunk 0x4932E0  (via call)
    inline bool atMinimumPosition(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x4932E0, _gb);
        return _gb.get_b(0);
    }
    // void breakJoint()
    //   thunk 0x493370  (impl 0x493570)
    inline void breakJoint(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x493370, _gb);
    }
    // float getPosition()
    //   thunk 0x493390  (via call)
    inline float getPosition(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x493390, _gb);
        return _gb.get_f(0);
    }
    // void setMaximumPosition(float pos)
    //   thunk 0x4933F0  (impl 0x3962E0)
    inline void setMaximumPosition(void* self, float pos) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, pos);
        vm::call(0x4933F0, _gb);
    }
    // void setMinimumPosition(float pos)
    //   thunk 0x493430  (impl 0x3962E0)
    inline void setMinimumPosition(void* self, float pos) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, pos);
        vm::call(0x493430, _gb);
    }
} // namespace CSliderActor

// ----------------------------------------------------------- CSlimer
namespace CSlimer {
    // void disableDefenseMode()
    //   thunk 0x15C0D0  (impl 0x15BE70)
    inline void disableDefenseMode(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x15C0D0, _gb);
    }
    // void setCommandSlimeAttack(@SSlimeAttackInfo info)
    //   thunk 0x15C100  (impl 0x15BE90)
    inline void setCommandSlimeAttack(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        vm::call(0x15C100, _gb);
    }
    // void setCommandSpecialHide(@CActor hidePoint, float dist)
    //   thunk 0x15C130  (impl 0x15BF00)
    inline void setCommandSpecialHide(void* self, void* hidePoint, float dist) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, hidePoint);
        _gb.set_f(2, dist);
        vm::call(0x15C130, _gb);
    }
    // void setInHallway(bool flag)
    //   thunk 0x15C0A0  (impl 0x15BE50)
    inline void setInHallway(void* self, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        vm::call(0x15C0A0, _gb);
    }
} // namespace CSlimer

// ------------------------------------------------------------ CSpawn
namespace CSpawn {
    // @CCharacter spawnCharacter(@SSpawnInfo info)
    //   thunk 0x15E130  (via call)
    inline void* spawnCharacter(void* self, void* info) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, info);
        vm::call(0x15E130, _gb);
        return _gb.get_p(0);
    }
} // namespace CSpawn

// ----------------------------------------------------- CSpawnTrigger
namespace CSpawnTrigger {
    // void activate()
    //   thunk 0x15FC20  (impl 0x15FA60)
    inline void activate(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x15FC20, _gb);
    }
    // void setMaxNumberOfEnemies(int newVal)
    //   thunk 0x15FCB0  (impl 0x15FBD0)
    inline void setMaxNumberOfEnemies(void* self, int newVal) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, newVal);
        vm::call(0x15FCB0, _gb);
    }
    // void setNumberOfEnemies(int newVal)
    //   thunk 0x15FC80  (impl 0x15FBC0)
    inline void setNumberOfEnemies(void* self, int newVal) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, newVal);
        vm::call(0x15FC80, _gb);
    }
    // void unspawnAllEnemies()
    //   thunk 0x15FC50  (impl 0x15FAA0)
    inline void unspawnAllEnemies(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x15FC50, _gb);
    }
} // namespace CSpawnTrigger

// ------------------------------------------------------ CSpiderWitch
namespace CSpiderWitch {
    // void fall()
    //   thunk 0x163410  (impl 0x163200)
    inline void fall(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x163410, _gb);
    }
    // void feed(@CWayPoint wp)
    //   thunk 0x1633E0  (impl 0x1631D0)
    inline void feed(void* self, void* wp) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, wp);
        vm::call(0x1633E0, _gb);
    }
    // void setTeleportDelay(float delay)
    //   thunk 0x1633C0  (via none)
    inline void setTeleportDelay(float delay) {
        vm::Block _gb;
        _gb.set_f(0, delay);
        vm::call(0x1633C0, _gb);
    }
} // namespace CSpiderWitch

// ---------------------------------------------------------- CSpinner
namespace CSpinner {
    // void enableSpin(bool enable)
    //   thunk 0x4A41E0  (impl 0x4A57B0)
    inline void enableSpin(void* self, bool enable) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, enable);
        vm::call(0x4A41E0, _gb);
    }
    // vector getAxisOfRotation()
    //   thunk 0x4A4210  (via none)
    inline Vector3 getAxisOfRotation(void* self) {
        vm::Block _gb;
        _gb.set_p(3, self);
        vm::call(0x4A4210, _gb);
        return _gb.get_v(0);
    }
    // float getRPM()
    //   thunk 0x4A4250  (via none)
    inline float getRPM(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x4A4250, _gb);
        return _gb.get_f(0);
    }
    // void setAxisOfRotation(vector axis)
    //   thunk 0x4A4280  (via none)
    inline void setAxisOfRotation(void* self, Vector3 axis) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, axis);
        vm::call(0x4A4280, _gb);
    }
    // void setRPM(float rpm)
    //   thunk 0x4A42C0  (via none)
    inline void setRPM(void* self, float rpm) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, rpm);
        vm::call(0x4A42C0, _gb);
    }
} // namespace CSpinner

// ------------------------------------------------------- CSplinePath
namespace CSplinePath {
    // float findClosestTimeToPoint(Vector wSearchPos)
    //   thunk 0x3482A0  (via call)
    inline float findClosestTimeToPoint(void* self, Vector3 wSearchPos) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_v(2, wSearchPos);
        vm::call(0x3482A0, _gb);
        return _gb.get_f(0);
    }
    // int findKeyByName(string keyName)
    //   thunk 0x3485D0  (via call)
    inline int findKeyByName(void* self, const char* keyName) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, keyName);
        vm::call(0x3485D0, _gb);
        return _gb.get_i(0);
    }
    // int findKeyByNameGraceful(string keyName)
    //   thunk 0x348560  (impl 0x3485BC)
    inline int findKeyByNameGraceful(void* self, const char* keyName) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, keyName);
        vm::call(0x348560, _gb);
        return _gb.get_i(0);
    }
    // string getKeyName(int key)
    //   thunk 0x348610  (impl 0x3486AC)
    inline const char* getKeyName(void* self, int key) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, key);
        vm::call(0x348610, _gb);
        return static_cast<const char*>(_gb.get_p(0));
    }
    // Vector getKeyOrient(int keyIndex)
    //   thunk 0x348310  (via call)
    inline Vector3 getKeyOrient(void* self, int keyIndex) {
        vm::Block _gb;
        _gb.set_p(3, self);
        _gb.set_i(4, keyIndex);
        vm::call(0x348310, _gb);
        return _gb.get_v(0);
    }
    // Vector getKeyPos(int keyIndex)
    //   thunk 0x3483A0  (impl 0x3484AA)
    inline Vector3 getKeyPos(void* self, int keyIndex) {
        vm::Block _gb;
        _gb.set_p(3, self);
        _gb.set_i(4, keyIndex);
        vm::call(0x3483A0, _gb);
        return _gb.get_v(0);
    }
    // float getTotalTime()
    //   thunk 0x3486C0  (via none)
    inline float getTotalTime(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x3486C0, _gb);
        return _gb.get_f(0);
    }
    // void warpKey(int whichKey, Vector newPos, Vector newOrient)
    //   thunk 0x3484D0  (via call)
    inline void warpKey(void* self, int whichKey, Vector3 newPos, Vector3 newOrient) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, whichKey);
        _gb.set_v(2, newPos);
        _gb.set_v(5, newOrient);
        vm::call(0x3484D0, _gb);
    }
    // void warpKeyToActor(int whichKey, @CActor destActor)
    //   thunk 0x348740  (impl 0x34EA20)
    inline void warpKeyToActor(void* self, int whichKey, void* destActor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, whichKey);
        _gb.set_p(2, destActor);
        vm::call(0x348740, _gb);
    }
} // namespace CSplinePath

// ----------------------------------------------------- CStayPuftBoss
namespace CStayPuftBoss {
    // void beginDoubleHandAttack()
    //   thunk 0x171C30  (impl 0x170A80)
    inline void beginDoubleHandAttack(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x171C30, _gb);
    }
    // void beginGrabAttack()
    //   thunk 0x171BC0  (impl 0x170A40)
    inline void beginGrabAttack(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x171BC0, _gb);
    }
    // void deflateHead()
    //   thunk 0x171E20  (impl 0x170BB0)
    inline void deflateHead(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x171E20, _gb);
    }
    // void endDoubleHandAttack()
    //   thunk 0x171C60  (impl 0x170AE0)
    inline void endDoubleHandAttack(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x171C60, _gb);
    }
    // void extinguishHead()
    //   thunk 0x171D60  (impl 0x170B30)
    inline void extinguishHead(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x171D60, _gb);
    }
    // int getCurrentStage()
    //   thunk 0x171B80  (via call)
    inline int getCurrentStage(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x171B80, _gb);
        return _gb.get_i(0);
    }
    // bool getDoubleHandAttackResult()
    //   thunk 0x171C90  (via call)
    inline bool getDoubleHandAttackResult(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x171C90, _gb);
        return _gb.get_b(0);
    }
    // bool getGrabAttackResult()
    //   thunk 0x171BF0  (via call)
    inline bool getGrabAttackResult(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x171BF0, _gb);
        return _gb.get_b(0);
    }
    // @CActor getThrowActor()
    //   thunk 0x171B40  (via call)
    inline void* getThrowActor(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x171B40, _gb);
        return _gb.get_p(0);
    }
    // void igniteHead()
    //   thunk 0x171D30  (impl 0x170B20)
    inline void igniteHead(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x171D30, _gb);
    }
    // void igniteLeftHand()
    //   thunk 0x171D00  (impl 0x170B10)
    inline void igniteLeftHand(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x171D00, _gb);
    }
    // void igniteRightHand()
    //   thunk 0x171CD0  (impl 0x170B00)
    inline void igniteRightHand(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x171CD0, _gb);
    }
    // void inflateHead()
    //   thunk 0x171DF0  (impl 0x170B90)
    inline void inflateHead(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x171DF0, _gb);
    }
    // void pickupCar(@CCarEffects car)
    //   thunk 0x172030  (impl 0x171470)
    inline void pickupCar(void* self, void* car) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, car);
        vm::call(0x172030, _gb);
    }
    // void requestFacialExpression(EFacialExpression newExpression)
    //   thunk 0x171EB0  (impl 0x170C60)
    inline void requestFacialExpression(void* self, int newExpression) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, newExpression);
        vm::call(0x171EB0, _gb);
    }
    // void requestRoar()
    //   thunk 0x171E80  (impl 0x170C10)
    inline void requestRoar(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x171E80, _gb);
    }
    // void setDamageState(int state)
    //   thunk 0x171EE0  (impl 0x170D30)
    inline void setDamageState(void* self, int state) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, state);
        vm::call(0x171EE0, _gb);
    }
    // void setHeadScale(float scale)
    //   thunk 0x171E50  (impl 0x170BD0)
    inline void setHeadScale(void* self, float scale) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, scale);
        vm::call(0x171E50, _gb);
    }
    // void startDamageFade(float time)
    //   thunk 0x171DC0  (impl 0x170B60)
    inline void startDamageFade(void* self, float time) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, time);
        vm::call(0x171DC0, _gb);
    }
    // void throwCar(@CActor targetActor, @CCarEffects car, float err)
    //   thunk 0x171F70  (impl 0x170FC0)
    inline void throwCar(void* self, void* targetActor, void* car, float err) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, targetActor);
        _gb.set_p(2, car);
        _gb.set_f(3, err);
        vm::call(0x171F70, _gb);
    }
    // void throwCarFast(@CActor targetActor, @CCarEffects car, float err)
    //   thunk 0x171FB0  (impl 0x1711D0)
    inline void throwCarFast(void* self, void* targetActor, void* car, float err) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, targetActor);
        _gb.set_p(2, car);
        _gb.set_f(3, err);
        vm::call(0x171FB0, _gb);
    }
    // void throwCarStreet(@CActor targetActor, @CCarEffects car, float err)
    //   thunk 0x171FF0  (impl 0x1713E0)
    inline void throwCarStreet(void* self, void* targetActor, void* car, float err) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, targetActor);
        _gb.set_p(2, car);
        _gb.set_f(3, err);
        vm::call(0x171FF0, _gb);
    }
    // void throwDebris(@CActor targetActor)
    //   thunk 0x171F10  (impl 0x170DA0)
    inline void throwDebris(void* self, void* targetActor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, targetActor);
        vm::call(0x171F10, _gb);
    }
    // void throwDebrisStreet(@CActor targetActor)
    //   thunk 0x171F40  (impl 0x170F80)
    inline void throwDebrisStreet(void* self, void* targetActor) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, targetActor);
        vm::call(0x171F40, _gb);
    }
    // void throwPhysicsObject(@CActor tActor, @CPhysicsObject po)
    //   thunk 0x172060  (impl 0x1714C0)
    inline void throwPhysicsObject(void* self, void* tActor, void* po) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, tActor);
        _gb.set_p(2, po);
        vm::call(0x172060, _gb);
    }
} // namespace CStayPuftBoss

// --------------------------------------------------- CStayPuftMinion
namespace CStayPuftMinion {
    // bool canBeAFlamer()
    //   thunk 0x1674D0  (via call)
    inline bool canBeAFlamer(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x1674D0, _gb);
        return _gb.get_b(0);
    }
    // float getFlamingMiniMaxTime()
    //   thunk 0x167450  (via call)
    inline float getFlamingMiniMaxTime(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x167450, _gb);
        return _gb.get_f(0);
    }
    // float getFlamingMiniMinTime()
    //   thunk 0x167490  (via call)
    inline float getFlamingMiniMinTime(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x167490, _gb);
        return _gb.get_f(0);
    }
    // void launchAtTarget(Vector wPos, float tol)
    //   thunk 0x1673C0  (via call)
    inline void launchAtTarget(void* self, Vector3 wPos, float tol) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_v(1, wPos);
        _gb.set_f(4, tol);
        vm::call(0x1673C0, _gb);
    }
    // void setElevator(@CElevator elevator)
    //   thunk 0x167390  (impl 0x167110)
    inline void setElevator(void* self, void* elevator) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, elevator);
        vm::call(0x167390, _gb);
    }
    // void setFlamer()
    //   thunk 0x167420  (impl 0x167340)
    inline void setFlamer(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x167420, _gb);
    }
} // namespace CStayPuftMinion

// ----------------------------------------------------------- CTarget
namespace CTarget {
    // void reset()
    //   thunk 0x175540  (via none)
    inline void reset(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x175540, _gb);
    }
    // void setARGB(int a, int r, int g, int b)
    //   thunk 0x175560  (via call)
    inline void setARGB(void* self, int a, int r, int g, int b) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_i(1, a);
        _gb.set_i(2, r);
        _gb.set_i(3, g);
        _gb.set_i(4, b);
        vm::call(0x175560, _gb);
    }
} // namespace CTarget

// ---------------------------------------------------------- CTrigger
namespace CTrigger {
    // bool containsActor(@CActor actorToTest = NULL)
    //   thunk 0x4A5BF0  (via call)
    inline bool containsActor(void* self, void* actorToTest) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, actorToTest);
        vm::call(0x4A5BF0, _gb);
        return _gb.get_b(0);
    }
    // void deleteAllActorsInside(String classes)
    //   thunk 0x4A5C90  (impl 0x4A6720)
    inline void deleteAllActorsInside(void* self, const char* classes) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, classes);
        vm::call(0x4A5C90, _gb);
    }
    // void enableActorsInsideMe(String classes)
    //   thunk 0x4A5CF0  (impl 0x4A67F0)
    inline void enableActorsInsideMe(void* self, const char* classes) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, classes);
        vm::call(0x4A5CF0, _gb);
    }
    // void enableActorsInsideMeNameFilter(String classes, String actorNameFilter)
    //   thunk 0x4A5CC0  (impl 0x4A6880)
    inline void enableActorsInsideMeNameFilter(void* self, const char* classes, const char* actorNameFilter) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, classes);
        _gb.set_p(2, actorNameFilter);
        vm::call(0x4A5CC0, _gb);
    }
    // void setMaxHitPoints(float newMaxHitPoints)
    //   thunk 0x4A5C50  (via none)
    inline void setMaxHitPoints(void* self, float newMaxHitPoints) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, newMaxHitPoints);
        vm::call(0x4A5C50, _gb);
    }
} // namespace CTrigger

// ---------------------------------------------------------- CTrinket
namespace CTrinket {
    // bool hasBeenPickedUp()
    //   thunk 0x179550  (via call)
    inline bool hasBeenPickedUp(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x179550, _gb);
        return _gb.get_b(0);
    }
} // namespace CTrinket

// ----------------------------------------------------------- CTurret
namespace CTurret {
    // void activate(bool flag, @CActor t = NULL)
    //   thunk 0x17B930  (impl 0x17B8A0)
    inline void activate(void* self, bool flag, void* t) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_b(1, flag);
        _gb.set_p(2, t);
        vm::call(0x17B930, _gb);
    }
} // namespace CTurret

// ---------------------------------------------- CUniversalJointActor
namespace CUniversalJointActor {
    // void breakJoint()
    //   thunk 0x491FF0  (via call)
    inline void breakJoint(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x491FF0, _gb);
    }
} // namespace CUniversalJointActor

// ----------------------------------------------------------- CVolFog
namespace CVolFog {
    // void dissipateFog()
    //   thunk 0x47FC20  (via none)
    inline void dissipateFog(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x47FC20, _gb);
    }
} // namespace CVolFog

// ---------------------------------------------------------- CVolLite
namespace CVolLite {
    // void fadeIn(float duration)
    //   thunk 0x3CA6D0  (via none)
    inline void fadeIn(void* self, float duration) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, duration);
        vm::call(0x3CA6D0, _gb);
    }
    // void fadeOut(float duration)
    //   thunk 0x3CA720  (via none)
    inline void fadeOut(void* self, float duration) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, duration);
        vm::call(0x3CA720, _gb);
    }
    // void stirDust(float rampUpTime, float sustainTime, float rampDownTime)
    //   thunk 0x3CA770  (via none)
    inline void stirDust(void* self, float rampUpTime, float sustainTime, float rampDownTime) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, rampUpTime);
        _gb.set_f(2, sustainTime);
        _gb.set_f(3, rampDownTime);
        vm::call(0x3CA770, _gb);
    }
} // namespace CVolLite

// -------------------------------------------------------- CWindActor
namespace CWindActor {
    // void setHeading(float angle)
    //   thunk 0x4A8230  (impl 0x4A8860)
    inline void setHeading(void* self, float angle) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, angle);
        vm::call(0x4A8230, _gb);
    }
    // void setMagnitudeMax(float mag)
    //   thunk 0x4A8270  (impl 0x4A8860)
    inline void setMagnitudeMax(void* self, float mag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, mag);
        vm::call(0x4A8270, _gb);
    }
    // void setMagnitudeMin(float mag)
    //   thunk 0x4A82B0  (impl 0x4A8860)
    inline void setMagnitudeMin(void* self, float mag) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, mag);
        vm::call(0x4A82B0, _gb);
    }
    // void setPitch(float angle)
    //   thunk 0x4A82F0  (impl 0x4A8860)
    inline void setPitch(void* self, float angle) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_f(1, angle);
        vm::call(0x4A82F0, _gb);
    }
} // namespace CWindActor

// ------------------------------------------------------------ Global
namespace Global {
    // void GTFO(String msg)
    //   thunk 0x48F550  (via none)
    inline void GTFO(const char* msg) {
        vm::Block _gb;
        _gb.set_p(0, msg);
        vm::call(0x48F550, _gb);
    }
    // void addLight(vector pos, float radius, vector rgb, float intensity, float duration, float rampUp = 0.0, float rampDown = 0.0)
    //   thunk 0x1F8210  (via call)
    inline void addLight(Vector3 pos, float radius, Vector3 rgb, float intensity, float duration, float rampUp = 0.0, float rampDown = 0.0) {
        vm::Block _gb;
        _gb.set_v(0, pos);
        _gb.set_f(3, radius);
        _gb.set_v(4, rgb);
        _gb.set_f(7, intensity);
        _gb.set_f(8, duration);
        _gb.set_f(9, rampUp);
        _gb.set_f(10, rampDown);
        vm::call(0x1F8210, _gb);
    }
    // void addSfxControllerSpeakerInfo(@SSfxInfo info, EControllerSpeaker s)
    //   thunk 0x411E50  (via none)
    inline void addSfxControllerSpeakerInfo(void* info, int s) {
        vm::Block _gb;
        _gb.set_p(0, info);
        _gb.set_i(1, s);
        vm::call(0x411E50, _gb);
    }
    // void allowEnemyAttack(bool flag)
    //   thunk 0x2D8420  (via indirect)
    inline void allowEnemyAttack(bool flag) {
        vm::Block _gb;
        _gb.set_b(0, flag);
        vm::call(0x2D8420, _gb);
    }
    // void allowHeroControls(bool flag)
    //   thunk 0x2D8440  (via indirect)
    inline void allowHeroControls(bool flag) {
        vm::Block _gb;
        _gb.set_b(0, flag);
        vm::call(0x2D8440, _gb);
    }
    // void allowHeroDamage(bool flag)
    //   thunk 0x2D8460  (via indirect)
    inline void allowHeroDamage(bool flag) {
        vm::Block _gb;
        _gb.set_b(0, flag);
        vm::call(0x2D8460, _gb);
    }
    // void animation()
    //   thunk 0x1F7FB0  (impl 0x1EC2D0)
    inline void animation() {
        vm::Block _gb;
        vm::call(0x1F7FB0, _gb);
    }
    // void awardAchievement(int achievementId)
    //   thunk 0x1F80B0  (impl 0x1EC4D0)
    inline void awardAchievement(int achievementId) {
        vm::Block _gb;
        _gb.set_i(0, achievementId);
        vm::call(0x1F80B0, _gb);
    }
    // void beginCinemat(string name)
    //   thunk 0x2D8480  (via call)
    inline void beginCinemat(const char* name) {
        vm::Block _gb;
        _gb.set_p(0, name);
        vm::call(0x2D8480, _gb);
    }
    // void buttonPrompt(EButtonAction buttonAction, float duration)
    //   thunk 0x254790  (impl 0x2494D0)
    inline void buttonPrompt(int buttonAction, float duration) {
        vm::Block _gb;
        _gb.set_i(0, buttonAction);
        _gb.set_f(1, duration);
        vm::call(0x254790, _gb);
    }
    // void cacheEffect(string effectName)
    //   thunk 0x35A380  (via call)
    inline void cacheEffect(const char* effectName) {
        vm::Block _gb;
        _gb.set_p(0, effectName);
        vm::call(0x35A380, _gb);
    }
    // bool cacheSfx(string name)
    //   thunk 0x411E70  (via call)
    inline bool cacheSfx(const char* name) {
        vm::Block _gb;
        _gb.set_p(1, name);
        vm::call(0x411E70, _gb);
        return _gb.get_b(0);
    }
    // int cacheSfxEventGroup(string name)
    //   thunk 0x411E60  (via none)
    inline int cacheSfxEventGroup(const char* name) {
        vm::Block _gb;
        _gb.set_p(1, name);
        vm::call(0x411E60, _gb);
        return _gb.get_i(0);
    }
    // void cacheSkeletalAnimation(string animationName)
    //   thunk 0x2D84D0  (via call)
    inline void cacheSkeletalAnimation(const char* animationName) {
        vm::Block _gb;
        _gb.set_p(0, animationName);
        vm::call(0x2D84D0, _gb);
    }
    // void cacheSkeletalAnimationByName(string animationName)
    //   thunk 0x2D84C0  (impl 0x2D9AF0)
    inline void cacheSkeletalAnimationByName(const char* animationName) {
        vm::Block _gb;
        _gb.set_p(0, animationName);
        vm::call(0x2D84C0, _gb);
    }
    // void cacheStreamingCinemat(string cinematName)
    //   thunk 0x476520  (via call)
    inline void cacheStreamingCinemat(const char* cinematName) {
        vm::Block _gb;
        _gb.set_p(0, cinematName);
        vm::call(0x476520, _gb);
    }
    // void cacheStreamingCinematAndAudio(string cinematName, string audioFileName)
    //   thunk 0x476510  (impl 0x477500)
    inline void cacheStreamingCinematAndAudio(const char* cinematName, const char* audioFileName) {
        vm::Block _gb;
        _gb.set_p(0, cinematName);
        _gb.set_p(1, audioFileName);
        vm::call(0x476510, _gb);
    }
    // vector calcLaunchVelocityPitch(vector launchPos, vector targetPos, float minElev, float maxElev)
    //   thunk 0x48F330  (impl 0x48F403)
    inline Vector3 calcLaunchVelocityPitch(Vector3 launchPos, Vector3 targetPos, float minElev, float maxElev) {
        vm::Block _gb;
        _gb.set_v(3, launchPos);
        _gb.set_v(6, targetPos);
        _gb.set_f(9, minElev);
        _gb.set_f(10, maxElev);
        vm::call(0x48F330, _gb);
        return _gb.get_v(0);
    }
    // vector calcLaunchVelocityTime(vector launchPos, vector targetPos, float flightTime)
    //   thunk 0x48F420  (impl 0x48F4C6)
    inline Vector3 calcLaunchVelocityTime(Vector3 launchPos, Vector3 targetPos, float flightTime) {
        vm::Block _gb;
        _gb.set_v(3, launchPos);
        _gb.set_v(6, targetPos);
        _gb.set_f(9, flightTime);
        vm::call(0x48F420, _gb);
        return _gb.get_v(0);
    }
    // void cancelWalkToOnAllCharacters()
    //   thunk 0x1F81B0  (impl 0x1EC8D0)
    inline void cancelWalkToOnAllCharacters() {
        vm::Block _gb;
        vm::call(0x1F81B0, _gb);
    }
    // float ceil(float in)
    //   thunk 0x48F560  (via call)
    inline float ceil(float in) {
        vm::Block _gb;
        _gb.set_f(1, in);
        vm::call(0x48F560, _gb);
        return _gb.get_f(0);
    }
    // void chainToLevel(String levelName, String lvlCheckpoint = "")
    //   thunk 0x1F8190  (impl 0x1EC8B0)
    inline void chainToLevel(const char* levelName, const char* lvlCheckpoint) {
        vm::Block _gb;
        _gb.set_p(0, levelName);
        _gb.set_p(1, lvlCheckpoint);
        vm::call(0x1F8190, _gb);
    }
    // void channels()
    //   thunk 0x1F7FD0  (impl 0x1EC350)
    inline void channels() {
        vm::Block _gb;
        vm::call(0x1F7FD0, _gb);
    }
    // void cinemat()
    //   thunk 0x1F7FC0  (impl 0x1EC310)
    inline void cinemat() {
        vm::Block _gb;
        vm::call(0x1F7FC0, _gb);
    }
    // void clearDebug()
    //   thunk 0x1F7FE0  (impl 0x1EC390)
    inline void clearDebug() {
        vm::Block _gb;
        vm::call(0x1F7FE0, _gb);
    }
    // float cos(float in)
    //   thunk 0x48F590  (via call)
    inline float cos(float in) {
        vm::Block _gb;
        _gb.set_f(1, in);
        vm::call(0x48F590, _gb);
        return _gb.get_f(0);
    }
    // @CActor createActorOfType(string className, vector wPos)
    //   thunk 0x2BD6E0  (via call)
    inline void* createActorOfType(const char* className, Vector3 wPos) {
        vm::Block _gb;
        _gb.set_p(1, className);
        _gb.set_v(2, wPos);
        vm::call(0x2BD6E0, _gb);
        return _gb.get_p(0);
    }
    // void createExplosion(vector pos, float radius, float damageStrength, float speed = 999.0)
    //   thunk 0x1EB660  (via call)
    inline void createExplosion(Vector3 pos, float radius, float damageStrength, float speed = 999.0) {
        vm::Block _gb;
        _gb.set_v(0, pos);
        _gb.set_f(3, radius);
        _gb.set_f(4, damageStrength);
        _gb.set_f(5, speed);
        vm::call(0x1EB660, _gb);
    }
    // void createInvisibleExplosion(vector pos, float radius, float damageToNPC)
    //   thunk 0x1EB6B0  (via call)
    inline void createInvisibleExplosion(Vector3 pos, float radius, float damageToNPC) {
        vm::Block _gb;
        _gb.set_v(0, pos);
        _gb.set_f(3, radius);
        _gb.set_f(4, damageToNPC);
        vm::call(0x1EB6B0, _gb);
    }
    // vector crossProduct(vector a, vector b)
    //   thunk 0x48F4E0  (via none)
    inline Vector3 crossProduct(Vector3 a, Vector3 b) {
        vm::Block _gb;
        _gb.set_v(3, a);
        _gb.set_v(6, b);
        vm::call(0x48F4E0, _gb);
        return _gb.get_v(0);
    }
    // void cueStreamingCinemat(string cinematName, float initialCursorPos = 0.0)
    //   thunk 0x4765C0  (impl 0x4779B0)
    inline void cueStreamingCinemat(const char* cinematName, float initialCursorPos = 0.0) {
        vm::Block _gb;
        _gb.set_p(0, cinematName);
        _gb.set_f(1, initialCursorPos);
        vm::call(0x4765C0, _gb);
    }
    // void customVolLiteHotel(@CVolLite which)
    //   thunk 0x1F84C0  (impl 0x1ECF20)
    inline void customVolLiteHotel(void* which) {
        vm::Block _gb;
        _gb.set_p(0, which);
        vm::call(0x1F84C0, _gb);
    }
    // void customVolLiteSewerSkylight(@CVolLite which)
    //   thunk 0x1F84D0  (impl 0x1ECFA0)
    inline void customVolLiteSewerSkylight(void* which) {
        vm::Block _gb;
        _gb.set_p(0, which);
        vm::call(0x1F84D0, _gb);
    }
    // float dbNarrative(@CDialogDatabaseEntry dbEntry)
    //   thunk 0x1F83E0  (via call)
    inline float dbNarrative(void* dbEntry) {
        vm::Block _gb;
        _gb.set_p(1, dbEntry);
        vm::call(0x1F83E0, _gb);
        return _gb.get_f(0);
    }
    // void dbNarrativeStop()
    //   thunk 0x1F8410  (impl 0x1ECED0)
    inline void dbNarrativeStop() {
        vm::Block _gb;
        vm::call(0x1F8410, _gb);
    }
    // void debugPrint(String msg)
    //   thunk 0x48F5C0  (via none)
    inline void debugPrint(const char* msg) {
        vm::Block _gb;
        _gb.set_p(0, msg);
        vm::call(0x48F5C0, _gb);
    }
    // void defineCheckpoint(string functionName, string checkpointName)
    //   thunk 0x1F81D0  (impl 0x1ECA20)
    inline void defineCheckpoint(const char* functionName, const char* checkpointName) {
        vm::Block _gb;
        _gb.set_p(0, functionName);
        _gb.set_p(1, checkpointName);
        vm::call(0x1F81D0, _gb);
    }
    // Vector deltaToOrient(Vector delta)
    //   thunk 0x48F5D0  (via call)
    inline Vector3 deltaToOrient(Vector3 delta) {
        vm::Block _gb;
        _gb.set_v(3, delta);
        vm::call(0x48F5D0, _gb);
        return _gb.get_v(0);
    }
    // void display(string text, float duration = -1.0, int slot = 0)
    //   thunk 0x1F7FF0  (impl 0x1EC3B0)
    inline void display(const char* text, float duration = -1.0, int slot = 0) {
        vm::Block _gb;
        _gb.set_p(0, text);
        _gb.set_f(1, duration);
        _gb.set_i(2, slot);
        vm::call(0x1F7FF0, _gb);
    }
    // void displayDemoSplashScreen()
    //   thunk 0x1F8330  (impl 0x1ECD30)
    inline void displayDemoSplashScreen() {
        vm::Block _gb;
        vm::call(0x1F8330, _gb);
    }
    // void displayGhostViewer()
    //   thunk 0x1F8360  (impl 0x1ECD90)
    inline void displayGhostViewer() {
        vm::Block _gb;
        vm::call(0x1F8360, _gb);
    }
    // void displayMessage(EHudMessage messageId, String text, float duration = -1.0f)
    //   thunk 0x254770  (impl 0x2494A0)
    inline void displayMessage(int messageId, const char* text, float duration = -1.0f) {
        vm::Block _gb;
        _gb.set_i(0, messageId);
        _gb.set_p(1, text);
        _gb.set_f(2, duration);
        vm::call(0x254770, _gb);
    }
    // void displaySplashScreen(string textureName, float duration, bool stretch = false, bool clear = true)
    //   thunk 0x1F8340  (impl 0x1ECD50)
    inline void displaySplashScreen(const char* textureName, float duration, bool stretch = false, bool clear = true) {
        vm::Block _gb;
        _gb.set_p(0, textureName);
        _gb.set_f(1, duration);
        _gb.set_b(2, stretch);
        _gb.set_b(3, clear);
        vm::call(0x1F8340, _gb);
    }
    // float distance(Vector p1, Vector p2)
    //   thunk 0x48F630  (via call)
    inline float distance(Vector3 p1, Vector3 p2) {
        vm::Block _gb;
        _gb.set_v(1, p1);
        _gb.set_v(4, p2);
        vm::call(0x48F630, _gb);
        return _gb.get_f(0);
    }
    // float dotProduct(vector a, vector b)
    //   thunk 0x48F8F0  (via none)
    inline float dotProduct(Vector3 a, Vector3 b) {
        vm::Block _gb;
        _gb.set_v(1, a);
        _gb.set_v(4, b);
        vm::call(0x48F8F0, _gb);
        return _gb.get_f(0);
    }
    // void drinkFromFountain(int fountainIdx)
    //   thunk 0x1F8140  (impl 0x1EC7A0)
    inline void drinkFromFountain(int fountainIdx) {
        vm::Block _gb;
        _gb.set_i(0, fountainIdx);
        vm::call(0x1F8140, _gb);
    }
    // void enableActorsInLevelSection(string levelSection, bool flag)
    //   thunk 0x2D85C0  (impl 0x2DA4A0)
    inline void enableActorsInLevelSection(const char* levelSection, bool flag) {
        vm::Block _gb;
        _gb.set_p(0, levelSection);
        _gb.set_b(1, flag);
        vm::call(0x2D85C0, _gb);
    }
    // void enableAllLights(bool flag)
    //   thunk 0x2E3810  (via none)
    inline void enableAllLights(bool flag) {
        vm::Block _gb;
        _gb.set_b(0, flag);
        vm::call(0x2E3810, _gb);
    }
    // void enableHUD(int flag)
    //   thunk 0x1F8150  (impl 0x1EC7F0)
    inline void enableHUD(int flag) {
        vm::Block _gb;
        _gb.set_i(0, flag);
        vm::call(0x1F8150, _gb);
    }
    // void endCinemat(string name)
    //   thunk 0x2D85E0  (via call)
    inline void endCinemat(const char* name) {
        vm::Block _gb;
        _gb.set_p(0, name);
        vm::call(0x2D85E0, _gb);
    }
    // void endGame()
    //   thunk 0x1F80C0  (impl 0x1EC500)
    inline void endGame() {
        vm::Block _gb;
        vm::call(0x1F80C0, _gb);
    }
    // float fAbs(float value)
    //   thunk 0x48F680  (via none)
    inline float fAbs(float value) {
        vm::Block _gb;
        _gb.set_f(1, value);
        vm::call(0x48F680, _gb);
        return _gb.get_f(0);
    }
    // float fRand(float min, float max)
    //   thunk 0x48F6A0  (via none)
    inline float fRand(float min, float max) {
        vm::Block _gb;
        _gb.set_f(1, min);
        _gb.set_f(2, max);
        vm::call(0x48F6A0, _gb);
        return _gb.get_f(0);
    }
    // void fade(float opacity, float r, float g, float b, float duration)
    //   thunk 0x1F82F0  (via call)
    inline void fade(float opacity, float r, float g, float b, float duration) {
        vm::Block _gb;
        _gb.set_f(0, opacity);
        _gb.set_f(1, r);
        _gb.set_f(2, g);
        _gb.set_f(3, b);
        _gb.set_f(4, duration);
        vm::call(0x1F82F0, _gb);
    }
    // @CActor findActorByName(string actorName)
    //   thunk 0x2D8620  (via call)
    inline void* findActorByName(const char* actorName) {
        vm::Block _gb;
        _gb.set_p(1, actorName);
        vm::call(0x2D8620, _gb);
        return _gb.get_p(0);
    }
    // @CActorGroup findActorGroupByName(string groupName)
    //   thunk 0x2D8650  (via call)
    inline void* findActorGroupByName(const char* groupName) {
        vm::Block _gb;
        _gb.set_p(1, groupName);
        vm::call(0x2D8650, _gb);
        return _gb.get_p(0);
    }
    // int findActorsByWildcard(@CActorGroup groupName, string wildcard)
    //   thunk 0x2D86A0  (via call)
    inline int findActorsByWildcard(void* groupName, const char* wildcard) {
        vm::Block _gb;
        _gb.set_p(1, groupName);
        _gb.set_p(2, wildcard);
        vm::call(0x2D86A0, _gb);
        return _gb.get_i(0);
    }
    // @CBreaker findBreakerByName(string name)
    //   thunk 0x2E3860  (via call)
    inline void* findBreakerByName(const char* name) {
        vm::Block _gb;
        _gb.set_p(1, name);
        vm::call(0x2E3860, _gb);
        return _gb.get_p(0);
    }
    // @CDialogDatabaseEntry findDBEntry(String tag)
    //   thunk 0x48F920  (via call)
    inline void* findDBEntry(const char* tag) {
        vm::Block _gb;
        _gb.set_p(1, tag);
        vm::call(0x48F920, _gb);
        return _gb.get_p(0);
    }
    // @CPortalLight findLightByName(string name)
    //   thunk 0x2E38B0  (via call)
    inline void* findLightByName(const char* name) {
        vm::Block _gb;
        _gb.set_p(1, name);
        vm::call(0x2E38B0, _gb);
        return _gb.get_p(0);
    }
    // @CRoom findRoomByName(string name)
    //   thunk 0x2E3900  (via call)
    inline void* findRoomByName(const char* name) {
        vm::Block _gb;
        _gb.set_p(1, name);
        vm::call(0x2E3900, _gb);
        return _gb.get_p(0);
    }
    // @CRoom findRoomForPoint(vector wPos)
    //   thunk 0x2E3950  (via call)
    inline void* findRoomForPoint(Vector3 wPos) {
        vm::Block _gb;
        _gb.set_v(1, wPos);
        vm::call(0x2E3950, _gb);
        return _gb.get_p(0);
    }
    // float floor(float in)
    //   thunk 0x48F6F0  (via call)
    inline float floor(float in) {
        vm::Block _gb;
        _gb.set_f(1, in);
        vm::call(0x48F6F0, _gb);
        return _gb.get_f(0);
    }
    // string formatFloat(float floatVal, int decimalDigits)
    //   thunk 0x48F9A0  (via call)
    inline const char* formatFloat(float floatVal, int decimalDigits) {
        vm::Block _gb;
        _gb.set_f(1, floatVal);
        _gb.set_i(2, decimalDigits);
        vm::call(0x48F9A0, _gb);
        return static_cast<const char*>(_gb.get_p(0));
    }
    // string getBuildDescription()
    //   thunk 0x1F8430  (via call)
    inline const char* getBuildDescription() {
        vm::Block _gb;
        vm::call(0x1F8430, _gb);
        return static_cast<const char*>(_gb.get_p(0));
    }
    // float getLevelDamage()
    //   thunk 0x1F80F0  (via call)
    inline float getLevelDamage() {
        vm::Block _gb;
        vm::call(0x1F80F0, _gb);
        return _gb.get_f(0);
    }
    // int getNextAttackId()
    //   thunk 0x1FD3F0  (via call)
    inline int getNextAttackId() {
        vm::Block _gb;
        vm::call(0x1FD3F0, _gb);
        return _gb.get_i(0);
    }
    // EGameControl getPlayerControllerType()
    //   thunk 0x2D8570  (via indirect)
    inline int getPlayerControllerType() {
        vm::Block _gb;
        vm::call(0x2D8570, _gb);
        return _gb.get_i(0);
    }
    // float getStat(int statIdx)
    //   thunk 0x1F8110  (via call)
    inline float getStat(int statIdx) {
        vm::Block _gb;
        _gb.set_i(1, statIdx);
        vm::call(0x1F8110, _gb);
        return _gb.get_f(0);
    }
    // int iRand(int min, int max)
    //   thunk 0x48F720  (via call)
    inline int iRand(int min, int max) {
        vm::Block _gb;
        _gb.set_i(1, min);
        _gb.set_i(2, max);
        vm::call(0x48F720, _gb);
        return _gb.get_i(0);
    }
    // void idle()
    //   thunk 0x48F740  (via none)
    inline void idle() {
        vm::Block _gb;
        vm::call(0x48F740, _gb);
    }
    // void insertTweakVarBool(string label, @bool variablePtr)
    //   thunk 0x2D86D0  (impl 0x31D780)
    inline void insertTweakVarBool(const char* label, void* variablePtr) {
        vm::Block _gb;
        _gb.set_p(0, label);
        _gb.set_p(1, variablePtr);
        vm::call(0x2D86D0, _gb);
    }
    // void insertTweakVarFloat(string label, @float variablePtr, float minVal, float maxVal, float increment)
    //   thunk 0x2D86F0  (via call)
    inline void insertTweakVarFloat(const char* label, void* variablePtr, float minVal, float maxVal, float increment) {
        vm::Block _gb;
        _gb.set_p(0, label);
        _gb.set_p(1, variablePtr);
        _gb.set_f(2, minVal);
        _gb.set_f(3, maxVal);
        _gb.set_f(4, increment);
        vm::call(0x2D86F0, _gb);
    }
    // void insertTweakVarInt(string label, @int variablePtr, int minVal, int maxVal, int increment)
    //   thunk 0x2D8740  (via call)
    inline void insertTweakVarInt(const char* label, void* variablePtr, int minVal, int maxVal, int increment) {
        vm::Block _gb;
        _gb.set_p(0, label);
        _gb.set_p(1, variablePtr);
        _gb.set_i(2, minVal);
        _gb.set_i(3, maxVal);
        _gb.set_i(4, increment);
        vm::call(0x2D8740, _gb);
    }
    // bool isDemoBuild()
    //   thunk 0x2D8780  (via none)
    inline bool isDemoBuild() {
        vm::Block _gb;
        vm::call(0x2D8780, _gb);
        return _gb.get_b(0);
    }
    // bool isLibrarianScanned()
    //   thunk 0x1F83A0  (via call)
    inline bool isLibrarianScanned() {
        vm::Block _gb;
        vm::call(0x1F83A0, _gb);
        return _gb.get_b(0);
    }
    // bool isRunningOnGen3Platform()
    //   thunk 0x2D8790  (via none)
    inline bool isRunningOnGen3Platform() {
        vm::Block _gb;
        vm::call(0x2D8790, _gb);
        return _gb.get_b(0);
    }
    // bool isSfxActive(int handle)
    //   thunk 0x411EC0  (via call)
    inline bool isSfxActive(int handle) {
        vm::Block _gb;
        _gb.set_i(1, handle);
        vm::call(0x411EC0, _gb);
        return _gb.get_b(0);
    }
    // bool isSixAxisEnabled()
    //   thunk 0x1F84A0  (via call)
    inline bool isSixAxisEnabled() {
        vm::Block _gb;
        vm::call(0x1F84A0, _gb);
        return _gb.get_b(0);
    }
    // bool isStreamingCinematPlaying(string cinematName)
    //   thunk 0x4765D0  (via call)
    inline bool isStreamingCinematPlaying(const char* cinematName) {
        vm::Block _gb;
        _gb.set_p(1, cinematName);
        vm::call(0x4765D0, _gb);
        return _gb.get_b(0);
    }
    // bool isThreadActive(int threadHandle)
    //   thunk 0x48FA00  (via call)
    inline bool isThreadActive(int threadHandle) {
        vm::Block _gb;
        _gb.set_i(1, threadHandle);
        vm::call(0x48FA00, _gb);
        return _gb.get_b(0);
    }
    // void killEffect(int handle)
    //   thunk 0x35A3E0  (impl 0x3DFDB0)
    inline void killEffect(int handle) {
        vm::Block _gb;
        _gb.set_i(0, handle);
        vm::call(0x35A3E0, _gb);
    }
    // void killSfx(int handle)
    //   thunk 0x411F10  (via call)
    inline void killSfx(int handle) {
        vm::Block _gb;
        _gb.set_i(0, handle);
        vm::call(0x411F10, _gb);
    }
    // bool killThreadByHandle(int threadHandle)
    //   thunk 0x48FA90  (via call)
    inline bool killThreadByHandle(int threadHandle) {
        vm::Block _gb;
        _gb.set_i(1, threadHandle);
        vm::call(0x48FA90, _gb);
        return _gb.get_b(0);
    }
    // void letterbox(bool flag)
    //   thunk 0x2D87A0  (via indirect)
    inline void letterbox(bool flag) {
        vm::Block _gb;
        _gb.set_b(0, flag);
        vm::call(0x2D87A0, _gb);
    }
    // float limit(float min, float value, float max)
    //   thunk 0x48F770  (via none)
    inline float limit(float min, float value, float max) {
        vm::Block _gb;
        _gb.set_f(1, min);
        _gb.set_f(2, value);
        _gb.set_f(3, max);
        vm::call(0x48F770, _gb);
        return _gb.get_f(0);
    }
    // void loadCheckpoint(string checkpointName)
    //   thunk 0x1F81F0  (impl 0x1ECA40)
    inline void loadCheckpoint(const char* checkpointName) {
        vm::Block _gb;
        _gb.set_p(0, checkpointName);
        vm::call(0x1F81F0, _gb);
    }
    // void loadMixSnapshot(string name)
    //   thunk 0x411F50  (impl 0x414170)
    inline void loadMixSnapshot(const char* name) {
        vm::Block _gb;
        _gb.set_p(0, name);
        vm::call(0x411F50, _gb);
    }
    // void notifyNewEquipmentAvailable()
    //   thunk 0x1F83D0  (impl 0x1ECE90)
    inline void notifyNewEquipmentAvailable() {
        vm::Block _gb;
        vm::call(0x1F83D0, _gb);
    }
    // Vector orientToDelta(Vector orient)
    //   thunk 0x48F790  (via call)
    inline Vector3 orientToDelta(Vector3 orient) {
        vm::Block _gb;
        _gb.set_v(3, orient);
        vm::call(0x48F790, _gb);
        return _gb.get_v(0);
    }
    // void pauseEditor(string msg)
    //   thunk 0x2D87D0  (via none)
    inline void pauseEditor(const char* msg) {
        vm::Block _gb;
        _gb.set_p(0, msg);
        vm::call(0x2D87D0, _gb);
    }
    // void pauseSfx(int handle, bool flag)
    //   thunk 0x411F60  (via call)
    inline void pauseSfx(int handle, bool flag) {
        vm::Block _gb;
        _gb.set_i(0, handle);
        _gb.set_b(1, flag);
        vm::call(0x411F60, _gb);
    }
    // void playStreamingCinemat(string cinematName)
    //   thunk 0x476600  (impl 0x478930)
    inline void playStreamingCinemat(const char* cinematName) {
        vm::Block _gb;
        _gb.set_p(0, cinematName);
        vm::call(0x476600, _gb);
    }
    // bool playerHasArtifact(int artifact)
    //   thunk 0x1F80D0  (via call)
    inline bool playerHasArtifact(int artifact) {
        vm::Block _gb;
        _gb.set_i(1, artifact);
        vm::call(0x1F80D0, _gb);
        return _gb.get_b(0);
    }
    // void queueVideo(string name)
    //   thunk 0x2D87E0  (via indirect)
    inline void queueVideo(const char* name) {
        vm::Block _gb;
        _gb.set_p(0, name);
        vm::call(0x2D87E0, _gb);
    }
    // void quitLevel()
    //   thunk 0x1F8170  (impl 0x1EC830)
    inline void quitLevel() {
        vm::Block _gb;
        vm::call(0x1F8170, _gb);
    }
    // void rampTimeFactor(float timeFactor, float rampTime)
    //   thunk 0x3B0390  (impl 0x3B11C0)
    inline void rampTimeFactor(float timeFactor, float rampTime) {
        vm::Block _gb;
        _gb.set_f(0, timeFactor);
        _gb.set_f(1, rampTime);
        vm::call(0x3B0390, _gb);
    }
    // bool randBool(float odds)
    //   thunk 0x48F7F0  (via none)
    inline bool randBool(float odds) {
        vm::Block _gb;
        _gb.set_f(1, odds);
        vm::call(0x48F7F0, _gb);
        return _gb.get_b(0);
    }
    // void removeSfxControllerSpeakerInfo(@SSfxInfo info, EControllerSpeaker s)
    //   thunk 0x411FA0  (via none)
    inline void removeSfxControllerSpeakerInfo(void* info, int s) {
        vm::Block _gb;
        _gb.set_p(0, info);
        _gb.set_i(1, s);
        vm::call(0x411FA0, _gb);
    }
    // void removeTweakVar(string label)
    //   thunk 0x2D8800  (impl 0x31E190)
    inline void removeTweakVar(const char* label) {
        vm::Block _gb;
        _gb.set_p(0, label);
        vm::call(0x2D8800, _gb);
    }
    // void resetBreakers()
    //   thunk 0x2E39D0  (via call)
    inline void resetBreakers() {
        vm::Block _gb;
        vm::call(0x2E39D0, _gb);
    }
    // void resetGravity()
    //   thunk 0x1F82E0  (impl 0x1ECC60)
    inline void resetGravity() {
        vm::Block _gb;
        vm::call(0x1F82E0, _gb);
    }
    // void resetScorchMarks()
    //   thunk 0x27DA60  (impl 0x27DA40)
    inline void resetScorchMarks() {
        vm::Block _gb;
        vm::call(0x27DA60, _gb);
    }
    // void restartLevel()
    //   thunk 0x1F8180  (impl 0x1EC850)
    inline void restartLevel() {
        vm::Block _gb;
        vm::call(0x1F8180, _gb);
    }
    // void restartStreamingCinemat(string cinematName)
    //   thunk 0x476610  (via call)
    inline void restartStreamingCinemat(const char* cinematName) {
        vm::Block _gb;
        _gb.set_p(0, cinematName);
        vm::call(0x476610, _gb);
    }
    // void rollCredits()
    //   thunk 0x2D85A0  (via indirect)
    inline void rollCredits() {
        vm::Block _gb;
        vm::call(0x2D85A0, _gb);
    }
    // void saveCheckpoint(string checkpointName)
    //   thunk 0x1F81C0  (impl 0x1EC970)
    inline void saveCheckpoint(const char* checkpointName) {
        vm::Block _gb;
        _gb.set_p(0, checkpointName);
        vm::call(0x1F81C0, _gb);
    }
    // void setAllowDamageTally(bool flag)
    //   thunk 0x1F8160  (impl 0x1EC810)
    inline void setAllowDamageTally(bool flag) {
        vm::Block _gb;
        _gb.set_b(0, flag);
        vm::call(0x1F8160, _gb);
    }
    // bool setAmbience(string name)
    //   thunk 0x411FB0  (via call)
    inline bool setAmbience(const char* name) {
        vm::Block _gb;
        _gb.set_p(1, name);
        vm::call(0x411FB0, _gb);
        return _gb.get_b(0);
    }
    // void setCurrentObjective(string test)
    //   thunk 0x1F8200  (impl 0x1ECB00)
    inline void setCurrentObjective(const char* test) {
        vm::Block _gb;
        _gb.set_p(0, test);
        vm::call(0x1F8200, _gb);
    }
    // void setDamageFog(bool flag)
    //   thunk 0x1E6A10  (impl 0x1E6A00)
    inline void setDamageFog(bool flag) {
        vm::Block _gb;
        _gb.set_b(0, flag);
        vm::call(0x1E6A10, _gb);
    }
    // void setDisplayBoxed(int slot, bool boxed = true)
    //   thunk 0x1F8070  (impl 0x1EC490)
    inline void setDisplayBoxed(int slot, bool boxed = true) {
        vm::Block _gb;
        _gb.set_i(0, slot);
        _gb.set_b(1, boxed);
        vm::call(0x1F8070, _gb);
    }
    // void setDisplayColor(int slot, float red, float green, float blue)
    //   thunk 0x1F8010  (impl 0x1EC410)
    inline void setDisplayColor(int slot, float red, float green, float blue) {
        vm::Block _gb;
        _gb.set_i(0, slot);
        _gb.set_f(1, red);
        _gb.set_f(2, green);
        _gb.set_f(3, blue);
        vm::call(0x1F8010, _gb);
    }
    // void setDisplayMaxWidth(int slot, float maxWidth = -1.0)
    //   thunk 0x1F8090  (impl 0x1EC4B0)
    inline void setDisplayMaxWidth(int slot, float maxWidth = -1.0) {
        vm::Block _gb;
        _gb.set_i(0, slot);
        _gb.set_f(1, maxWidth);
        vm::call(0x1F8090, _gb);
    }
    // void setDisplayPosition(int slot, int xPosition, int yPosition, ETextAlignX alignment)
    //   thunk 0x1F8030  (impl 0x1EC440)
    inline void setDisplayPosition(int slot, int xPosition, int yPosition, int alignment) {
        vm::Block _gb;
        _gb.set_i(0, slot);
        _gb.set_i(1, xPosition);
        _gb.set_i(2, yPosition);
        _gb.set_i(3, alignment);
        vm::call(0x1F8030, _gb);
    }
    // void setDisplaySize(int slot, float size)
    //   thunk 0x1F8050  (impl 0x1EC470)
    inline void setDisplaySize(int slot, float size) {
        vm::Block _gb;
        _gb.set_i(0, slot);
        _gb.set_f(1, size);
        vm::call(0x1F8050, _gb);
    }
    // void setFog(int r, int g, int b, float beginDistance, float endDistance, float maxValue, float rampTime)
    //   thunk 0x2E3A30  (via call)
    inline void setFog(int r, int g, int b, float beginDistance, float endDistance, float maxValue, float rampTime) {
        vm::Block _gb;
        _gb.set_i(0, r);
        _gb.set_i(1, g);
        _gb.set_i(2, b);
        _gb.set_f(3, beginDistance);
        _gb.set_f(4, endDistance);
        _gb.set_f(5, maxValue);
        _gb.set_f(6, rampTime);
        vm::call(0x2E3A30, _gb);
    }
    // void setGlobalWindHeading(float angle)
    //   thunk 0x2E3A80  (impl 0x4A86C0)
    inline void setGlobalWindHeading(float angle) {
        vm::Block _gb;
        _gb.set_f(0, angle);
        vm::call(0x2E3A80, _gb);
    }
    // void setGlobalWindMagnitudeMax(float mag)
    //   thunk 0x2E3A90  (impl 0x4A86E0)
    inline void setGlobalWindMagnitudeMax(float mag) {
        vm::Block _gb;
        _gb.set_f(0, mag);
        vm::call(0x2E3A90, _gb);
    }
    // void setGlobalWindMagnitudeMin(float mag)
    //   thunk 0x2E3AA0  (impl 0x4A8700)
    inline void setGlobalWindMagnitudeMin(float mag) {
        vm::Block _gb;
        _gb.set_f(0, mag);
        vm::call(0x2E3AA0, _gb);
    }
    // void setGlobalWindPitch(float angle)
    //   thunk 0x2E3AB0  (impl 0x4A8720)
    inline void setGlobalWindPitch(float angle) {
        vm::Block _gb;
        _gb.set_f(0, angle);
        vm::call(0x2E3AB0, _gb);
    }
    // void setGravity(vector v)
    //   thunk 0x1F82A0  (via call)
    inline void setGravity(Vector3 v) {
        vm::Block _gb;
        _gb.set_v(0, v);
        vm::call(0x1F82A0, _gb);
    }
    // void setLevelDescription(string name)
    //   thunk 0x2D8820  (via none)
    inline void setLevelDescription(const char* name) {
        vm::Block _gb;
        _gb.set_p(0, name);
        vm::call(0x2D8820, _gb);
    }
    // void setMovieCaptureEnable(bool flag)
    //   thunk 0x2D8850  (via indirect)
    inline void setMovieCaptureEnable(bool flag) {
        vm::Block _gb;
        _gb.set_b(0, flag);
        vm::call(0x2D8850, _gb);
    }
    // bool setMusic(string name)
    //   thunk 0x411FE0  (via call)
    inline bool setMusic(const char* name) {
        vm::Block _gb;
        _gb.set_p(1, name);
        vm::call(0x411FE0, _gb);
        return _gb.get_b(0);
    }
    // void setProtonBeamMaxLength(float length)
    //   thunk 0x277B30  (impl 0x277A50)
    inline void setProtonBeamMaxLength(float length) {
        vm::Block _gb;
        _gb.set_f(0, length);
        vm::call(0x277B30, _gb);
    }
    // bool setReverb(string name)
    //   thunk 0x412010  (via call)
    inline bool setReverb(const char* name) {
        vm::Block _gb;
        _gb.set_p(1, name);
        vm::call(0x412010, _gb);
        return _gb.get_b(0);
    }
    // void setSfxInfo(@SSfxInfo info, string name)
    //   thunk 0x412040  (via none)
    inline void setSfxInfo(void* info, const char* name) {
        vm::Block _gb;
        _gb.set_p(0, info);
        _gb.set_p(1, name);
        vm::call(0x412040, _gb);
    }
    // void setSfxSpatializedInfo(@SSfxInfo info, @CVector pos)
    //   thunk 0x412060  (via none)
    inline void setSfxSpatializedInfo(void* self, void* info, void* pos) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, info);
        _gb.set_p(2, pos);
        vm::call(0x412060, _gb);
    }
    // void setSfxTrackedInfo(@SSfxInfo info, @CVector pos, @void data)
    //   thunk 0x412070  (via none)
    inline void setSfxTrackedInfo(void* info, void* pos, void* data) {
        vm::Block _gb;
        _gb.set_p(0, info);
        _gb.set_p(1, pos);
        _gb.set_p(2, data);
        vm::call(0x412070, _gb);
    }
    // void setSkyboxAngle(float pitch, float bank, float heading, bool slam = false)
    //   thunk 0x2E3AC0  (via call)
    inline void setSkyboxAngle(float pitch, float bank, float heading, bool slam = false) {
        vm::Block _gb;
        _gb.set_f(0, pitch);
        _gb.set_f(1, bank);
        _gb.set_f(2, heading);
        _gb.set_b(3, slam);
        vm::call(0x2E3AC0, _gb);
    }
    // void setTimeFactor(float timeFactor, float rampTime = 0.0f)
    //   thunk 0x3B03B0  (impl 0x3B11C0)
    inline void setTimeFactor(float timeFactor, float rampTime = 0.0f) {
        vm::Block _gb;
        _gb.set_f(0, timeFactor);
        _gb.set_f(1, rampTime);
        vm::call(0x3B03B0, _gb);
    }
    // bool simplePatternMatch(String pattern, String testString)
    //   thunk 0x48F830  (via call)
    inline bool simplePatternMatch(const char* pattern, const char* testString) {
        vm::Block _gb;
        _gb.set_p(1, pattern);
        _gb.set_p(2, testString);
        vm::call(0x48F830, _gb);
        return _gb.get_b(0);
    }
    // float sin(float in)
    //   thunk 0x48F860  (via call)
    inline float sin(float in) {
        vm::Block _gb;
        _gb.set_f(1, in);
        vm::call(0x48F860, _gb);
        return _gb.get_f(0);
    }
    // void snapProfile()
    //   thunk 0x2D8870  (via none)
    inline void snapProfile() {
        vm::Block _gb;
        vm::call(0x2D8870, _gb);
    }
    // float sqrt(float in)
    //   thunk 0x48F890  (via call)
    inline float sqrt(float in) {
        vm::Block _gb;
        _gb.set_f(1, in);
        vm::call(0x48F890, _gb);
        return _gb.get_f(0);
    }
    // int startEffect(string effectName, vector position, vector orientation)
    //   thunk 0x35A3F0  (via call)
    inline int startEffect(const char* effectName, Vector3 position, Vector3 orientation) {
        vm::Block _gb;
        _gb.set_p(1, effectName);
        _gb.set_v(2, position);
        _gb.set_v(5, orientation);
        vm::call(0x35A3F0, _gb);
        return _gb.get_i(0);
    }
    // int startSfx(string name)
    //   thunk 0x412140  (via call)
    inline int startSfx(const char* name) {
        vm::Block _gb;
        _gb.set_p(1, name);
        vm::call(0x412140, _gb);
        return _gb.get_i(0);
    }
    // int startSfxInfo(@SSfxInfo info)
    //   thunk 0x412090  (via call)
    inline int startSfxInfo(void* info) {
        vm::Block _gb;
        _gb.set_p(1, info);
        vm::call(0x412090, _gb);
        return _gb.get_i(0);
    }
    // int startSfxPaused(string name)
    //   thunk 0x4120B0  (via call)
    inline int startSfxPaused(const char* name) {
        vm::Block _gb;
        _gb.set_p(1, name);
        vm::call(0x4120B0, _gb);
        return _gb.get_i(0);
    }
    // int startSfxSpatialized(string name, vector pos)
    //   thunk 0x4120D0  (via call)
    inline int startSfxSpatialized(const char* name, Vector3 pos) {
        vm::Block _gb;
        _gb.set_p(1, name);
        _gb.set_v(2, pos);
        vm::call(0x4120D0, _gb);
        return _gb.get_i(0);
    }
    // int startSfxTracked(string name, @CVector pos, @void data)
    //   thunk 0x412110  (via call)
    inline int startSfxTracked(const char* name, void* pos, void* data) {
        vm::Block _gb;
        _gb.set_p(1, name);
        _gb.set_p(2, pos);
        _gb.set_p(3, data);
        vm::call(0x412110, _gb);
        return _gb.get_i(0);
    }
    // void stopStreamingCinemat(string cinematName)
    //   thunk 0x476640  (via call)
    inline void stopStreamingCinemat(const char* cinematName) {
        vm::Block _gb;
        _gb.set_p(0, cinematName);
        vm::call(0x476640, _gb);
    }
    // float streamingCinematGetDuration(string cinematName)
    //   thunk 0x4766B0  (via call)
    inline float streamingCinematGetDuration(const char* cinematName) {
        vm::Block _gb;
        _gb.set_p(1, cinematName);
        vm::call(0x4766B0, _gb);
        return _gb.get_f(0);
    }
    // float streamingCinematGetFrame(string cinematName)
    //   thunk 0x476700  (via call)
    inline float streamingCinematGetFrame(const char* cinematName) {
        vm::Block _gb;
        _gb.set_p(1, cinematName);
        vm::call(0x476700, _gb);
        return _gb.get_f(0);
    }
    // @CActor tryFindActorByName(string actorName)
    //   thunk 0x2D8880  (via call)
    inline void* tryFindActorByName(const char* actorName) {
        vm::Block _gb;
        _gb.set_p(1, actorName);
        vm::call(0x2D8880, _gb);
        return _gb.get_p(0);
    }
    // void unloadMixSnapshot()
    //   thunk 0x412160  (via none)
    inline void unloadMixSnapshot() {
        vm::Block _gb;
        vm::call(0x412160, _gb);
    }
    // void unlockSpiritGuideEctoplasm()
    //   thunk 0x1F8380  (impl 0x1ECDD0)
    inline void unlockSpiritGuideEctoplasm() {
        vm::Block _gb;
        vm::call(0x1F8380, _gb);
    }
    // void unlockSpiritGuideLibrarian()
    //   thunk 0x1F8390  (impl 0x1ECDF0)
    inline void unlockSpiritGuideLibrarian() {
        vm::Block _gb;
        vm::call(0x1F8390, _gb);
    }
    // void unlockSpiritGuideSlothGhost()
    //   thunk 0x1F83C0  (impl 0x1ECE70)
    inline void unlockSpiritGuideSlothGhost() {
        vm::Block _gb;
        vm::call(0x1F83C0, _gb);
    }
    // void unlockSpiritGuideTraceEvidence()
    //   thunk 0x1F8370  (impl 0x1ECDB0)
    inline void unlockSpiritGuideTraceEvidence() {
        vm::Block _gb;
        vm::call(0x1F8370, _gb);
    }
    // float wrapPi(float value)
    //   thunk 0x48F8D0  (via call)
    inline float wrapPi(float value) {
        vm::Block _gb;
        _gb.set_f(1, value);
        vm::call(0x48F8D0, _gb);
        return _gb.get_f(0);
    }
} // namespace Global

// ------------------------------------------------------------ HSound
namespace HSound {
    // bool cache(String name)
    //   thunk 0x35AE50  (via call)
    inline bool cache(void* self, const char* name) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, name);
        vm::call(0x35AE50, _gb);
        return _gb.get_b(0);
    }
    // void constructor()
    //   thunk 0x35AE90  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x35AE90, _gb);
    }
    // void destructor()
    //   thunk 0x35AEB0  (via call)
    inline void destructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x35AEB0, _gb);
    }
    // void release()
    //   thunk 0x35AF00  (via call)
    inline void release(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x35AF00, _gb);
    }
} // namespace HSound

// ---------------------------------------------------- HSoundInstance
namespace HSoundInstance {
    // void constructor()
    //   thunk 0x35AD30  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x35AD30, _gb);
    }
    // void destructor()
    //   thunk 0x35AD50  (via none)
    inline void destructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x35AD50, _gb);
    }
    // void start(@HSound handle, @Vector pos = NULL)
    //   thunk 0x35AD60  (via call)
    inline void start(void* self, void* handle, void* pos) {
        vm::Block _gb;
        _gb.set_p(0, self);
        _gb.set_p(1, handle);
        _gb.set_p(2, pos);
        vm::call(0x35AD60, _gb);
    }
    // void stop()
    //   thunk 0x35ADF0  (impl 0x4199E0)
    inline void stop(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x35ADF0, _gb);
    }
} // namespace HSoundInstance

// ----------------------------------------- SBipedCombatComponentInfo
namespace SBipedCombatComponentInfo {
    // void constructor()
    //   thunk 0x37530  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x37530, _gb);
    }
    // void destructor()
    //   thunk 0x37550  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0x37550, _gb);
    }
} // namespace SBipedCombatComponentInfo

// ------------------------------------ SBipedLargeCombatComponentInfo
namespace SBipedLargeCombatComponentInfo {
    // void constructor()
    //   thunk 0x3D830  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x3D830, _gb);
    }
    // void destructor()
    //   thunk 0x3D850  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0x3D850, _gb);
    }
} // namespace SBipedLargeCombatComponentInfo

// -------------------------------------------------------- SDOFPlanes
namespace SDOFPlanes {
    // void constructor()
    //   thunk 0x41A9D0  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x41A9D0, _gb);
    }
    // void destructor()
    //   thunk 0x41AA20  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0x41AA20, _gb);
    }
} // namespace SDOFPlanes

// ------------------------------------------------------- SDamageInfo
namespace SDamageInfo {
    // void constructor()
    //   thunk 0x1FD410  (via call)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x1FD410, _gb);
    }
    // void destructor()
    //   thunk 0x1FD470  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0x1FD470, _gb);
    }
} // namespace SDamageInfo

// ------------------------------------------------------ SEvasionInfo
namespace SEvasionInfo {
    // void constructor()
    //   thunk 0xED2F0  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xED2F0, _gb);
    }
    // void destructor()
    //   thunk 0xED310  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0xED310, _gb);
    }
} // namespace SEvasionInfo

// ----------------------------------- SFlyerMediumCombatComponentInfo
namespace SFlyerMediumCombatComponentInfo {
    // void constructor()
    //   thunk 0xA57D0  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xA57D0, _gb);
    }
    // void destructor()
    //   thunk 0xA57F0  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0xA57F0, _gb);
    }
} // namespace SFlyerMediumCombatComponentInfo

// ------------------------------------ SFlyerSmallCombatComponentInfo
namespace SFlyerSmallCombatComponentInfo {
    // void constructor()
    //   thunk 0xA94D0  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xA94D0, _gb);
    }
    // void destructor()
    //   thunk 0xA94F0  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0xA94F0, _gb);
    }
} // namespace SFlyerSmallCombatComponentInfo

// -------------------------------------------- SGBCombatComponentInfo
namespace SGBCombatComponentInfo {
    // void constructor()
    //   thunk 0xED320  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xED320, _gb);
    }
    // void destructor()
    //   thunk 0xED340  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0xED340, _gb);
    }
} // namespace SGBCombatComponentInfo

// ----------------------------------------- SGhostCombatComponentInfo
namespace SGhostCombatComponentInfo {
    // void constructor()
    //   thunk 0xBF660  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xBF660, _gb);
    }
    // void destructor()
    //   thunk 0xBF680  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0xBF680, _gb);
    }
} // namespace SGhostCombatComponentInfo

// ---------------------------------------------------- SGhostFleeInfo
namespace SGhostFleeInfo {
    // void constructor()
    //   thunk 0xBF620  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0xBF620, _gb);
    }
    // void destructor()
    //   thunk 0xBF650  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0xBF650, _gb);
    }
} // namespace SGhostFleeInfo

// -------------------------------------------------------- SJointData
namespace SJointData {
    // void constructor()
    //   thunk 0x3CE410  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x3CE410, _gb);
    }
    // void destructor()
    //   thunk 0x3CE450  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0x3CE450, _gb);
    }
    // bool setPosFromTag(@CActor actor, string tagName)
    //   thunk 0x3CE460  (via call)
    inline bool setPosFromTag(void* self, void* actor, const char* tagName) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_p(2, actor);
        _gb.set_p(3, tagName);
        vm::call(0x3CE460, _gb);
        return _gb.get_b(0);
    }
} // namespace SJointData

// ------------------------------------------------- SMaterialVariable
namespace SMaterialVariable {
    // void constructor()
    //   thunk 0x3AAA50  (via none)
    inline void constructor() {
        vm::Block _gb;
        vm::call(0x3AAA50, _gb);
    }
    // void destructor()
    //   thunk 0x3AAA60  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0x3AAA60, _gb);
    }
} // namespace SMaterialVariable

// --------------------------------------------------- SScriptWalkInfo
namespace SScriptWalkInfo {
    // void constructor()
    //   thunk 0x84700  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x84700, _gb);
    }
    // void destructor()
    //   thunk 0x84720  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0x84720, _gb);
    }
} // namespace SScriptWalkInfo

// -------------------------------------- SScuttlerCombatComponentInfo
namespace SScuttlerCombatComponentInfo {
    // void constructor()
    //   thunk 0x155A90  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x155A90, _gb);
    }
    // void destructor()
    //   thunk 0x155AB0  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0x155AB0, _gb);
    }
} // namespace SScuttlerCombatComponentInfo

// ---------------------------------------------------------- SSfxInfo
namespace SSfxInfo {
    // void constructor()
    //   thunk 0x411E20  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x411E20, _gb);
    }
    // void destructor()
    //   thunk 0x411E40  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0x411E40, _gb);
    }
} // namespace SSfxInfo

// -------------------------------------------------- SShakeCameraInfo
namespace SShakeCameraInfo {
    // void constructor()
    //   thunk 0x205F80  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x205F80, _gb);
    }
    // void destructor()
    //   thunk 0x205FA0  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0x205FA0, _gb);
    }
} // namespace SShakeCameraInfo

// -------------------------------------------------- SSlimeAttackInfo
namespace SSlimeAttackInfo {
    // void constructor()
    //   thunk 0x15C040  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x15C040, _gb);
    }
    // void destructor()
    //   thunk 0x15C090  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0x15C090, _gb);
    }
} // namespace SSlimeAttackInfo

// -------------------------------------------------------- SSpawnInfo
namespace SSpawnInfo {
    // void constructor()
    //   thunk 0x15E0B0  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x15E0B0, _gb);
    }
    // void destructor()
    //   thunk 0x15E0F0  (via call)
    inline void destructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x15E0F0, _gb);
    }
} // namespace SSpawnInfo

// -------------------------------------------------- STerrainTypeInfo
namespace STerrainTypeInfo {
    // void constructor()
    //   thunk 0x43EF80  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x43EF80, _gb);
    }
    // void destructor()
    //   thunk 0x43EFA0  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0x43EFA0, _gb);
    }
} // namespace STerrainTypeInfo

// -------------------------------------------------- SWaveformControl
namespace SWaveformControl {
    // void constructor()
    //   thunk 0x3504A0  (via none)
    inline void constructor(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x3504A0, _gb);
    }
    // void destructor()
    //   thunk 0x3504C0  (via none)
    inline void destructor() {
        vm::Block _gb;
        vm::call(0x3504C0, _gb);
    }
} // namespace SWaveformControl

// ------------------------------------------------------------ String
namespace String {
    // float atof()
    //   thunk 0x2D3810  (via call)
    inline float atof(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2D3810, _gb);
        return _gb.get_f(0);
    }
    // int atoi()
    //   thunk 0x2D3850  (via call)
    inline int atoi(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2D3850, _gb);
        return _gb.get_i(0);
    }
    // String getCharacter(int N)
    //   thunk 0x2D3880  (via call)
    inline const char* getCharacter(void* self, int N) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, N);
        vm::call(0x2D3880, _gb);
        return static_cast<const char*>(_gb.get_p(0));
    }
    // String left(int n)
    //   thunk 0x2D38F0  (via call)
    inline const char* left(void* self, int n) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, n);
        vm::call(0x2D38F0, _gb);
        return static_cast<const char*>(_gb.get_p(0));
    }
    // int length()
    //   thunk 0x2D3970  (via none)
    inline int length(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2D3970, _gb);
        return _gb.get_i(0);
    }
    // String right(int n)
    //   thunk 0x2D39A0  (via call)
    inline const char* right(void* self, int n) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, n);
        vm::call(0x2D39A0, _gb);
        return static_cast<const char*>(_gb.get_p(0));
    }
    // String substring(int start, int run)
    //   thunk 0x2D3A50  (via call)
    inline const char* substring(void* self, int start, int run) {
        vm::Block _gb;
        _gb.set_p(1, self);
        _gb.set_i(2, start);
        _gb.set_i(3, run);
        vm::call(0x2D3A50, _gb);
        return static_cast<const char*>(_gb.get_p(0));
    }
} // namespace String

// ------------------------------------------------------------ Vector
namespace Vector {
    // float length()
    //   thunk 0x2D3D70  (via call)
    inline float length(void* self) {
        vm::Block _gb;
        _gb.set_p(1, self);
        vm::call(0x2D3D70, _gb);
        return _gb.get_f(0);
    }
    // void normalize()
    //   thunk 0x2D3DC0  (via call)
    inline void normalize(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x2D3DC0, _gb);
    }
    // void zero()
    //   thunk 0x2D3E40  (via none)
    inline void zero(void* self) {
        vm::Block _gb;
        _gb.set_p(0, self);
        vm::call(0x2D3E40, _gb);
    }
} // namespace Vector

} // namespace GB
