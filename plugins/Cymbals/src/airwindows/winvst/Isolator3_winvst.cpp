// WinVST compilation wrapper for Isolator3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Isolator3

#include "../../../../../deps/airwindows/plugins/WinVST/Isolator3/Isolator3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Isolator3/Isolator3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Isolator3() { return new Isolator3(nullptr); }
}

