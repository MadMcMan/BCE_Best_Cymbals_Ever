// WinVST compilation wrapper for Average
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Average

#include "../../../../../deps/airwindows/plugins/WinVST/Average/Average.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Average/AverageProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Average() { return new Average(nullptr); }
}

