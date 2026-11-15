// WinVST compilation wrapper for TapeDelay
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TapeDelay

#include "../../../../../deps/airwindows/plugins/WinVST/TapeDelay/TapeDelay.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TapeDelay/TapeDelayProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TapeDelay() { return new TapeDelay(nullptr); }
}

