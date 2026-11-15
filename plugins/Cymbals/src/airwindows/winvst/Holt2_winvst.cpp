// WinVST compilation wrapper for Holt2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Holt2

#include "../../../../../deps/airwindows/plugins/WinVST/Holt2/Holt2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Holt2/Holt2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Holt2() { return new Holt2(nullptr); }
}

