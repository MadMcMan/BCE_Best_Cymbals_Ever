// WinVST compilation wrapper for Loud
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Loud

#include "../../../../../deps/airwindows/plugins/WinVST/Loud/Loud.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Loud/LoudProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Loud() { return new Loud(nullptr); }
}

