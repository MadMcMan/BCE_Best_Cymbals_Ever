// WinVST compilation wrapper for Slew4
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Slew4

#include "../../../../../deps/airwindows/plugins/WinVST/Slew4/Slew4.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Slew4/Slew4Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Slew4() { return new Slew4(nullptr); }
}

