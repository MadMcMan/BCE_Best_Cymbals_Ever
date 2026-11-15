// WinVST compilation wrapper for Pressure5
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Pressure5

#include "../../../../../deps/airwindows/plugins/WinVST/Pressure5/Pressure5.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Pressure5/Pressure5Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Pressure5() { return new Pressure5(nullptr); }
}

