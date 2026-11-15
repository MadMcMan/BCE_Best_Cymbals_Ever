// WinVST compilation wrapper for DitherFloat
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DitherFloat

#include "../../../../../deps/airwindows/plugins/WinVST/DitherFloat/DitherFloat.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DitherFloat/DitherFloatProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DitherFloat() { return new DitherFloat(nullptr); }
}

