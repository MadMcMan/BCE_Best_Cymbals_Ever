// WinVST compilation wrapper for Pafnuty
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Pafnuty

#include "../../../../../deps/airwindows/plugins/WinVST/Pafnuty/Pafnuty.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Pafnuty/PafnutyProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Pafnuty() { return new Pafnuty(nullptr); }
}

