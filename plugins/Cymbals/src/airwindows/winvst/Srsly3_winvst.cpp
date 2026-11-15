// WinVST compilation wrapper for Srsly3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Srsly3

#include "../../../../../deps/airwindows/plugins/WinVST/Srsly3/Srsly3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Srsly3/Srsly3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Srsly3() { return new Srsly3(nullptr); }
}

