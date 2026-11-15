// WinVST compilation wrapper for ZNotch
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ZNotch

#include "../../../../../deps/airwindows/plugins/WinVST/ZNotch/ZNotch.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ZNotch/ZNotchProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ZNotch() { return new ZNotch(nullptr); }
}

