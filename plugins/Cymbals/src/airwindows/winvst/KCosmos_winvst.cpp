// WinVST compilation wrapper for KCosmos
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KCosmos

#include "../../../../../deps/airwindows/plugins/WinVST/kCosmos/kCosmos.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/kCosmos/kCosmosProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KCosmos() { return new kCosmos(nullptr); }
}

