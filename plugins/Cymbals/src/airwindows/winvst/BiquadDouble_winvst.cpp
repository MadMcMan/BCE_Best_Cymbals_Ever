// WinVST compilation wrapper for BiquadDouble
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BiquadDouble

#include "../../../../../deps/airwindows/plugins/WinVST/BiquadDouble/BiquadDouble.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BiquadDouble/BiquadDoubleProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BiquadDouble() { return new BiquadDouble(nullptr); }
}

