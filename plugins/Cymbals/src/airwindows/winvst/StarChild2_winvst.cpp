// WinVST compilation wrapper for StarChild2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_StarChild2

#include "../../../../../deps/airwindows/plugins/WinVST/StarChild2/StarChild2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/StarChild2/StarChild2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_StarChild2() { return new StarChild2(nullptr); }
}

