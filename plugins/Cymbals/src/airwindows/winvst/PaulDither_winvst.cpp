// WinVST compilation wrapper for PaulDither
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PaulDither

#include "../../../../../deps/airwindows/plugins/WinVST/PaulDither/PaulDither.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PaulDither/PaulDitherProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PaulDither() { return new PaulDither(nullptr); }
}

