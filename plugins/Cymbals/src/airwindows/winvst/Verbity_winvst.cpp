// WinVST compilation wrapper for Verbity
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Verbity

#include "../../../../../deps/airwindows/plugins/WinVST/Verbity/Verbity.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Verbity/VerbityProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Verbity() { return new Verbity(nullptr); }
}

