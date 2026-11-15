// WinVST compilation wrapper for BiquadNonLin
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BiquadNonLin

#include "../../../../../deps/airwindows/plugins/WinVST/BiquadNonLin/BiquadNonLin.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BiquadNonLin/BiquadNonLinProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BiquadNonLin() { return new BiquadNonLin(nullptr); }
}

