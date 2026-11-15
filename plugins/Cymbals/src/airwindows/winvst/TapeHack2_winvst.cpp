// WinVST compilation wrapper for TapeHack2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TapeHack2

#include "../../../../../deps/airwindows/plugins/WinVST/TapeHack2/TapeHack2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TapeHack2/TapeHack2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TapeHack2() { return new TapeHack2(nullptr); }
}

