// WinVST compilation wrapper for Air4
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Air4

#include "../../../../../deps/airwindows/plugins/WinVST/Air4/Air4.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Air4/Air4Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Air4() { return new Air4(nullptr); }
}

