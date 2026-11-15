// WinVST compilation wrapper for Parametric
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Parametric

#include "../../../../../deps/airwindows/plugins/WinVST/Parametric/Parametric.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Parametric/ParametricProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Parametric() { return new Parametric(nullptr); }
}

