// WinVST compilation wrapper for BezEQ2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BezEQ2

#include "../../../../../deps/airwindows/plugins/WinVST/BezEQ2/BezEQ2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BezEQ2/BezEQ2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BezEQ2() { return new BezEQ2(nullptr); }
}

