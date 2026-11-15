// WinVST compilation wrapper for Dubly3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Dubly3

#include "../../../../../deps/airwindows/plugins/WinVST/Dubly3/Dubly3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Dubly3/Dubly3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Dubly3() { return new Dubly3(nullptr); }
}

