// WinVST compilation wrapper for Galactic3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Galactic3

#include "../../../../../deps/airwindows/plugins/WinVST/Galactic3/Galactic3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Galactic3/Galactic3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Galactic3() { return new Galactic3(nullptr); }
}

