// WinVST compilation wrapper for Slew3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Slew3

#include "../../../../../deps/airwindows/plugins/WinVST/Slew3/Slew3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Slew3/Slew3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Slew3() { return new Slew3(nullptr); }
}

