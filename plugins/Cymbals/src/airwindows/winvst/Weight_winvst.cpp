// WinVST compilation wrapper for Weight
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Weight

#include "../../../../../deps/airwindows/plugins/WinVST/Weight/Weight.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Weight/WeightProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Weight() { return new Weight(nullptr); }
}

