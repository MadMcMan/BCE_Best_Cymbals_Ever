// WinVST compilation wrapper for Distortion
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Distortion

#include "../../../../../deps/airwindows/plugins/WinVST/Distortion/Distortion.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Distortion/DistortionProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Distortion() { return new Distortion(nullptr); }
}

