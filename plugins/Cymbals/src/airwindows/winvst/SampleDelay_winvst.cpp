// WinVST compilation wrapper for SampleDelay
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_SampleDelay

#include "../../../../../deps/airwindows/plugins/WinVST/SampleDelay/SampleDelay.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/SampleDelay/SampleDelayProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_SampleDelay() { return new SampleDelay(nullptr); }
}

