// WinVST compilation wrapper for Luxor
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Luxor

#include "../../../../../deps/airwindows/plugins/WinVST/Luxor/Luxor.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Luxor/LuxorProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Luxor() { return new Luxor(nullptr); }
}

