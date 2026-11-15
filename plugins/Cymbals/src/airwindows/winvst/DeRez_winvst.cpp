// WinVST compilation wrapper for DeRez
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DeRez

#include "../../../../../deps/airwindows/plugins/WinVST/DeRez/DeRez.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DeRez/DeRezProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DeRez() { return new DeRez(nullptr); }
}

