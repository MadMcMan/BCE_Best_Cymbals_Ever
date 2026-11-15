// WinVST compilation wrapper for TapeDust
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TapeDust

#include "../../../../../deps/airwindows/plugins/WinVST/TapeDust/TapeDust.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TapeDust/TapeDustProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TapeDust() { return new TapeDust(nullptr); }
}

