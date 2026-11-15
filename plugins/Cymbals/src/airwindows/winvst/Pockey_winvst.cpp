// WinVST compilation wrapper for Pockey
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Pockey

#include "../../../../../deps/airwindows/plugins/WinVST/Pockey/Pockey.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Pockey/PockeyProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Pockey() { return new Pockey(nullptr); }
}

