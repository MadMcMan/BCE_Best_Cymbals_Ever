// WinVST compilation wrapper for Spiral3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Spiral3

#include "../../../../../deps/airwindows/plugins/WinVST/Spiral3/Spiral3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Spiral3/Spiral3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Spiral3() { return new Spiral3(nullptr); }
}

