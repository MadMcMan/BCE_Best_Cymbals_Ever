// WinVST compilation wrapper for DeRez3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DeRez3

#include "../../../../../deps/airwindows/plugins/WinVST/DeRez3/DeRez3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DeRez3/DeRez3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DeRez3() { return new DeRez3(nullptr); }
}

