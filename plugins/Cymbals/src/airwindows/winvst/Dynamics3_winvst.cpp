// WinVST compilation wrapper for Dynamics3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Dynamics3

#include "../../../../../deps/airwindows/plugins/WinVST/Dynamics3/Dynamics3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Dynamics3/Dynamics3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Dynamics3() { return new Dynamics3(nullptr); }
}

