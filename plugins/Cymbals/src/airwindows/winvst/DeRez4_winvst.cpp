// WinVST compilation wrapper for DeRez4
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DeRez4

#include "../../../../../deps/airwindows/plugins/WinVST/DeRez4/DeRez4.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DeRez4/DeRez4Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DeRez4() { return new DeRez4(nullptr); }
}

