// WinVST compilation wrapper for HighGlossDither
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_HighGlossDither

#include "../../../../../deps/airwindows/plugins/WinVST/HighGlossDither/HighGlossDither.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/HighGlossDither/HighGlossDitherProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_HighGlossDither() { return new HighGlossDither(nullptr); }
}

