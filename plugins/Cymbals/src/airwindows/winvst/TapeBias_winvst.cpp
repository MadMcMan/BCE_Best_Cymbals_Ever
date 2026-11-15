// WinVST compilation wrapper for TapeBias
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TapeBias

#include "../../../../../deps/airwindows/plugins/WinVST/TapeBias/TapeBias.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TapeBias/TapeBiasProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TapeBias() { return new TapeBias(nullptr); }
}

