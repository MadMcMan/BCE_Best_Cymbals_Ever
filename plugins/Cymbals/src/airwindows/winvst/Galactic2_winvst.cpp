// WinVST compilation wrapper for Galactic2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Galactic2

#include "../../../../../deps/airwindows/plugins/WinVST/Galactic2/Galactic2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Galactic2/Galactic2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Galactic2() { return new Galactic2(nullptr); }
}

