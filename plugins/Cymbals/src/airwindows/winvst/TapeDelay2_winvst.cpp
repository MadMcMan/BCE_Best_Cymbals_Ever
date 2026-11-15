// WinVST compilation wrapper for TapeDelay2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TapeDelay2

#include "../../../../../deps/airwindows/plugins/WinVST/TapeDelay2/TapeDelay2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TapeDelay2/TapeDelay2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TapeDelay2() { return new TapeDelay2(nullptr); }
}

