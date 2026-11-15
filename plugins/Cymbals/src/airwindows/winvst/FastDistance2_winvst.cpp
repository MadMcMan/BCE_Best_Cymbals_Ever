// WinVST compilation wrapper for FastDistance2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_FastDistance2

#include "../../../../../deps/airwindows/plugins/WinVST/FastDistance2/FastDistance2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/FastDistance2/FastDistance2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_FastDistance2() { return new FastDistance2(nullptr); }
}

