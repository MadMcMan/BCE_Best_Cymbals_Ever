// WinVST compilation wrapper for NaturalizeDither
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_NaturalizeDither

#include "../../../../../deps/airwindows/plugins/WinVST/NaturalizeDither/NaturalizeDither.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/NaturalizeDither/NaturalizeDitherProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_NaturalizeDither() { return new NaturalizeDither(nullptr); }
}

