// WinVST compilation wrapper for BezEQ3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BezEQ3

#include "../../../../../deps/airwindows/plugins/WinVST/BezEQ3/BezEQ3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BezEQ3/BezEQ3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BezEQ3() { return new BezEQ3(nullptr); }
}

