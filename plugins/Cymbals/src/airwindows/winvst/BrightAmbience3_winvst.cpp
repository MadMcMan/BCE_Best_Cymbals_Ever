// WinVST compilation wrapper for BrightAmbience3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BrightAmbience3

#include "../../../../../deps/airwindows/plugins/WinVST/BrightAmbience3/BrightAmbience3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BrightAmbience3/BrightAmbience3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BrightAmbience3() { return new BrightAmbience3(nullptr); }
}

