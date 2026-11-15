// WinVST compilation wrapper for Infinity
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Infinity

#include "../../../../../deps/airwindows/plugins/WinVST/Infinity/Infinity.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Infinity/InfinityProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Infinity() { return new Infinity(nullptr); }
}

