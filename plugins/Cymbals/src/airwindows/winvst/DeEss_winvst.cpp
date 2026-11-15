// WinVST compilation wrapper for DeEss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DeEss

#include "../../../../../deps/airwindows/plugins/WinVST/DeEss/DeEss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DeEss/DeEssProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DeEss() { return new DeEss(nullptr); }
}

