// WinVST compilation wrapper for ResEQ2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ResEQ2

#include "../../../../../deps/airwindows/plugins/WinVST/ResEQ2/ResEQ2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ResEQ2/ResEQ2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ResEQ2() { return new ResEQ2(nullptr); }
}

