// WinVST compilation wrapper for BiquadOneHalf
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BiquadOneHalf

#include "../../../../../deps/airwindows/plugins/WinVST/BiquadOneHalf/BiquadOneHalf.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BiquadOneHalf/BiquadOneHalfProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BiquadOneHalf() { return new BiquadOneHalf(nullptr); }
}

