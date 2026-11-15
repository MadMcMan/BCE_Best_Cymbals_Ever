// WinVST compilation wrapper for Isolator2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Isolator2

#include "../../../../../deps/airwindows/plugins/WinVST/Isolator2/Isolator2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Isolator2/Isolator2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Isolator2() { return new Isolator2(nullptr); }
}

