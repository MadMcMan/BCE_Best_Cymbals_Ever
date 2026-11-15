// WinVST compilation wrapper for DarkNoise
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DarkNoise

#include "../../../../../deps/airwindows/plugins/WinVST/DarkNoise/DarkNoise.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DarkNoise/DarkNoiseProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DarkNoise() { return new DarkNoise(nullptr); }
}

