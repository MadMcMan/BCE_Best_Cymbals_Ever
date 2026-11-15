// WinVST compilation wrapper for BrassRider
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BrassRider

#include "../../../../../deps/airwindows/plugins/WinVST/BrassRider/BrassRider.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BrassRider/BrassRiderProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BrassRider() { return new BrassRider(nullptr); }
}

