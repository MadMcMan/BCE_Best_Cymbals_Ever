// WinVST compilation wrapper for PitchDelay
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PitchDelay

#include "../../../../../deps/airwindows/plugins/WinVST/PitchDelay/PitchDelay.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PitchDelay/PitchDelayProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PitchDelay() { return new PitchDelay(nullptr); }
}

