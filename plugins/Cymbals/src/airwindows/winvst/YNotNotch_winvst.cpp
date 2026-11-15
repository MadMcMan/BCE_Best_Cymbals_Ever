// WinVST compilation wrapper for YNotNotch
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_YNotNotch

#include "../../../../../deps/airwindows/plugins/WinVST/YNotNotch/YNotNotch.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/YNotNotch/YNotNotchProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_YNotNotch() { return new YNotNotch(nullptr); }
}

