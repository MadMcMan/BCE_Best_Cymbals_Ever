// WinVST compilation wrapper for Infinity2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Infinity2

#include "../../../../../deps/airwindows/plugins/WinVST/Infinity2/Infinity2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Infinity2/Infinity2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Infinity2() { return new Infinity2(nullptr); }
}

