// WinVST compilation wrapper for TapeFat
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TapeFat

#include "../../../../../deps/airwindows/plugins/WinVST/TapeFat/TapeFat.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TapeFat/TapeFatProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TapeFat() { return new TapeFat(nullptr); }
}

