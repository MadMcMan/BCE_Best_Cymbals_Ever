// WinVST compilation wrapper for Surge
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Surge

#include "../../../../../deps/airwindows/plugins/WinVST/Surge/Surge.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Surge/SurgeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Surge() { return new Surge(nullptr); }
}

