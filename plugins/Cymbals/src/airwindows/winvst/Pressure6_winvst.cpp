// WinVST compilation wrapper for Pressure6
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Pressure6

#include "../../../../../deps/airwindows/plugins/WinVST/Pressure6/Pressure6.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Pressure6/Pressure6Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Pressure6() { return new Pressure6(nullptr); }
}

