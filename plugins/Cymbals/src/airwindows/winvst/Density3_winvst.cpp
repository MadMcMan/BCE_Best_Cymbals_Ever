// WinVST compilation wrapper for Density3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Density3

#include "../../../../../deps/airwindows/plugins/WinVST/Density3/Density3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Density3/Density3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Density3() { return new Density3(nullptr); }
}

