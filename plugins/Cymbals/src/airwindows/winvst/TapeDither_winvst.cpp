// WinVST compilation wrapper for TapeDither
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TapeDither

#include "../../../../../deps/airwindows/plugins/WinVST/TapeDither/TapeDither.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TapeDither/TapeDitherProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TapeDither() { return new TapeDither(nullptr); }
}

