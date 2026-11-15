// WinVST compilation wrapper for BiquadHiLo
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BiquadHiLo

#include "../../../../../deps/airwindows/plugins/WinVST/BiquadHiLo/BiquadHiLo.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BiquadHiLo/BiquadHiLoProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BiquadHiLo() { return new BiquadHiLo(nullptr); }
}

