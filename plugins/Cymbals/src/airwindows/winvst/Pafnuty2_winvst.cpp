// WinVST compilation wrapper for Pafnuty2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Pafnuty2

#include "../../../../../deps/airwindows/plugins/WinVST/Pafnuty2/Pafnuty2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Pafnuty2/Pafnuty2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Pafnuty2() { return new Pafnuty2(nullptr); }
}

