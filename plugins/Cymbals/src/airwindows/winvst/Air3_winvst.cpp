// WinVST compilation wrapper for Air3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Air3

#include "../../../../../deps/airwindows/plugins/WinVST/Air3/Air3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Air3/Air3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Air3() { return new Air3(nullptr); }
}

