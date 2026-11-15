// WinVST compilation wrapper for Reverb
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Reverb

#include "../../../../../deps/airwindows/plugins/WinVST/Reverb/Reverb.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Reverb/ReverbProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Reverb() { return new Reverb(nullptr); }
}

