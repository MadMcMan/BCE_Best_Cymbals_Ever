// WinVST compilation wrapper for SideDull
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_SideDull

#include "../../../../../deps/airwindows/plugins/WinVST/SideDull/SideDull.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/SideDull/SideDullProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_SideDull() { return new SideDull(nullptr); }
}

