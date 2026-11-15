// WinVST compilation wrapper for Noise
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Noise

#include "../../../../../deps/airwindows/plugins/WinVST/Noise/Noise.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Noise/NoiseProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Noise() { return new Noise(nullptr); }
}

