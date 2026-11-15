// WinVST compilation wrapper for GoldenSlew
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_GoldenSlew

#include "../../../../../deps/airwindows/plugins/WinVST/GoldenSlew/GoldenSlew.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/GoldenSlew/GoldenSlewProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_GoldenSlew() { return new GoldenSlew(nullptr); }
}

