// WinVST compilation wrapper for PurestAir
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestAir

#include "../../../../../deps/airwindows/plugins/WinVST/PurestAir/PurestAir.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestAir/PurestAirProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestAir() { return new PurestAir(nullptr); }
}

