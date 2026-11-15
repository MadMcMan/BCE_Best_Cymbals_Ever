// WinVST compilation wrapper for OrbitKick
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_OrbitKick

#include "../../../../../deps/airwindows/plugins/WinVST/OrbitKick/OrbitKick.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/OrbitKick/OrbitKickProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_OrbitKick() { return new OrbitKick(nullptr); }
}

