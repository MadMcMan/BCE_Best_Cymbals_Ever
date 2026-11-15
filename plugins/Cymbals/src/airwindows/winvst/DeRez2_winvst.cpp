// WinVST compilation wrapper for DeRez2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DeRez2

#include "../../../../../deps/airwindows/plugins/WinVST/DeRez2/DeRez2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DeRez2/DeRez2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DeRez2() { return new DeRez2(nullptr); }
}

