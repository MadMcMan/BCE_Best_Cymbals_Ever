// WinVST compilation wrapper for PitchNasty
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PitchNasty

#include "../../../../../deps/airwindows/plugins/WinVST/PitchNasty/PitchNasty.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PitchNasty/PitchNastyProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PitchNasty() { return new PitchNasty(nullptr); }
}

