// WinVST compilation wrapper for StereoFX
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_StereoFX

#include "../../../../../deps/airwindows/plugins/WinVST/StereoFX/StereoFX.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/StereoFX/StereoFXProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_StereoFX() { return new StereoFX(nullptr); }
}

