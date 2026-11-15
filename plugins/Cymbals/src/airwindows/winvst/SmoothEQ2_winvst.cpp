// WinVST compilation wrapper for SmoothEQ2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_SmoothEQ2

#include "../../../../../deps/airwindows/plugins/WinVST/SmoothEQ2/SmoothEQ2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/SmoothEQ2/SmoothEQ2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_SmoothEQ2() { return new SmoothEQ2(nullptr); }
}

