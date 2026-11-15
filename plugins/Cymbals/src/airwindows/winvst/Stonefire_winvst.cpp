// WinVST compilation wrapper for Stonefire
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Stonefire

#include "../../../../../deps/airwindows/plugins/WinVST/Stonefire/Stonefire.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Stonefire/StonefireProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Stonefire() { return new Stonefire(nullptr); }
}

