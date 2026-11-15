// WinVST compilation wrapper for FastDistance
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_FastDistance

#include "../../../../../deps/airwindows/plugins/WinVST/FastDistance/FastDistance.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/FastDistance/FastDistanceProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_FastDistance() { return new FastDistance(nullptr); }
}

