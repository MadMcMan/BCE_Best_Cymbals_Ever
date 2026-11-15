// WinVST compilation wrapper for Distance2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Distance2

#include "../../../../../deps/airwindows/plugins/WinVST/Distance2/Distance2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Distance2/Distance2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Distance2() { return new Distance2(nullptr); }
}

