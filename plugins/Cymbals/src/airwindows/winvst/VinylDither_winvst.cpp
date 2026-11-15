// WinVST compilation wrapper for VinylDither
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_VinylDither

#include "../../../../../deps/airwindows/plugins/WinVST/VinylDither/VinylDither.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/VinylDither/VinylDitherProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_VinylDither() { return new VinylDither(nullptr); }
}

