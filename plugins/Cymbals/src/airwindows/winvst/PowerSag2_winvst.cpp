// WinVST compilation wrapper for PowerSag2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PowerSag2

#include "../../../../../deps/airwindows/plugins/WinVST/PowerSag2/PowerSag2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PowerSag2/PowerSag2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PowerSag2() { return new PowerSag2(nullptr); }
}

