// WinVST compilation wrapper for Pressure4
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Pressure4

#include "../../../../../deps/airwindows/plugins/WinVST/Pressure4/Pressure4.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Pressure4/Pressure4Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Pressure4() { return new Pressure4(nullptr); }
}

