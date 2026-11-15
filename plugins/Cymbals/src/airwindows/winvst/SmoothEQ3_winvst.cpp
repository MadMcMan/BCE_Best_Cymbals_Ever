// WinVST compilation wrapper for SmoothEQ3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_SmoothEQ3

#include "../../../../../deps/airwindows/plugins/WinVST/SmoothEQ3/SmoothEQ3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/SmoothEQ3/SmoothEQ3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_SmoothEQ3() { return new SmoothEQ3(nullptr); }
}

