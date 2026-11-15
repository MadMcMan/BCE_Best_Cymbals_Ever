// WinVST compilation wrapper for StarChild
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_StarChild

#include "../../../../../deps/airwindows/plugins/WinVST/StarChild/StarChild.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/StarChild/StarChildProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_StarChild() { return new StarChild(nullptr); }
}

