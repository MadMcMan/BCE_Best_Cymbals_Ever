// WinVST compilation wrapper for Dynamics2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Dynamics2

#include "../../../../../deps/airwindows/plugins/WinVST/Dynamics2/Dynamics2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Dynamics2/Dynamics2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Dynamics2() { return new Dynamics2(nullptr); }
}

