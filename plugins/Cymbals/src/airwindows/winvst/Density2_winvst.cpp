// WinVST compilation wrapper for Density2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Density2

#include "../../../../../deps/airwindows/plugins/WinVST/Density2/Density2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Density2/Density2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Density2() { return new Density2(nullptr); }
}

