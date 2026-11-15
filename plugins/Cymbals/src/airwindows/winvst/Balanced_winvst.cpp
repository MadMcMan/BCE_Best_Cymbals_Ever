// WinVST compilation wrapper for Balanced
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Balanced

#include "../../../../../deps/airwindows/plugins/WinVST/Balanced/Balanced.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Balanced/BalancedProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Balanced() { return new Balanced(nullptr); }
}

