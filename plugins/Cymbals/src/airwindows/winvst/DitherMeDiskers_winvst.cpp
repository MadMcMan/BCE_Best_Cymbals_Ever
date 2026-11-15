// WinVST compilation wrapper for DitherMeDiskers
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DitherMeDiskers

#include "../../../../../deps/airwindows/plugins/WinVST/DitherMeDiskers/DitherMeDiskers.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DitherMeDiskers/DitherMeDiskersProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DitherMeDiskers() { return new DitherMeDiskers(nullptr); }
}

