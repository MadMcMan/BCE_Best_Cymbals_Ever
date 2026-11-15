// WinVST compilation wrapper for Highpass2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Highpass2

#include "../../../../../deps/airwindows/plugins/WinVST/Highpass2/Highpass2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Highpass2/Highpass2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Highpass2() { return new Highpass2(nullptr); }
}

