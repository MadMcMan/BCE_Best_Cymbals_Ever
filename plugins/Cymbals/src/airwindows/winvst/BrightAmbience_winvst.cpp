// WinVST compilation wrapper for BrightAmbience
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BrightAmbience

#include "../../../../../deps/airwindows/plugins/WinVST/BrightAmbience/BrightAmbience.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BrightAmbience/BrightAmbienceProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BrightAmbience() { return new BrightAmbience(nullptr); }
}

