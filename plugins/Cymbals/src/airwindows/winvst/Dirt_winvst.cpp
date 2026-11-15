// WinVST compilation wrapper for Dirt
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Dirt

#include "../../../../../deps/airwindows/plugins/WinVST/Dirt/Dirt.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Dirt/DirtProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Dirt() { return new Dirt(nullptr); }
}

