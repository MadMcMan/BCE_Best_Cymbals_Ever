// WinVST compilation wrapper for Hull
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Hull

#include "../../../../../deps/airwindows/plugins/WinVST/Hull/Hull.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Hull/HullProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Hull() { return new Hull(nullptr); }
}

