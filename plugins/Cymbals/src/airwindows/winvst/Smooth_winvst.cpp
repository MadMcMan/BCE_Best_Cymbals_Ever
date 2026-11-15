// WinVST compilation wrapper for Smooth
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Smooth

#include "../../../../../deps/airwindows/plugins/WinVST/Smooth/Smooth.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Smooth/SmoothProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Smooth() { return new Smooth(nullptr); }
}

