// WinVST compilation wrapper for ADT
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ADT

#include "../../../../../deps/airwindows/plugins/WinVST/ADT/ADT.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ADT/ADTProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ADT() { return new ADT(nullptr); }
}

