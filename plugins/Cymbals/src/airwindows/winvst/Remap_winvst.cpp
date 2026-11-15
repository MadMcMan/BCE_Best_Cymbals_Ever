// WinVST compilation wrapper for Remap
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Remap

#include "../../../../../deps/airwindows/plugins/WinVST/Remap/Remap.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Remap/RemapProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Remap() { return new Remap(nullptr); }
}

