// WinVST compilation wrapper for PurestFade
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestFade

#include "../../../../../deps/airwindows/plugins/WinVST/PurestFade/PurestFade.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestFade/PurestFadeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestFade() { return new PurestFade(nullptr); }
}

