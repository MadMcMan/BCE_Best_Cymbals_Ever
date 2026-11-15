// WinVST compilation wrapper for Vibrato
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Vibrato

#include "../../../../../deps/airwindows/plugins/WinVST/Vibrato/Vibrato.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Vibrato/VibratoProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Vibrato() { return new Vibrato(nullptr); }
}

