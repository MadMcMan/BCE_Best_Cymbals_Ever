// WinVST compilation wrapper for Biquad
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Biquad

#include "../../../../../deps/airwindows/plugins/WinVST/Biquad/Biquad.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Biquad/BiquadProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Biquad() { return new Biquad(nullptr); }
}

