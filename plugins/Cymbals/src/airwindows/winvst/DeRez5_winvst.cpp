// WinVST compilation wrapper for DeRez5
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DeRez5

#include "../../../../../deps/airwindows/plugins/WinVST/DeRez5/DeRez5.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DeRez5/DeRez5Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DeRez5() { return new DeRez5(nullptr); }
}

