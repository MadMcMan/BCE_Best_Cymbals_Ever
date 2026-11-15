// WinVST compilation wrapper for KPlateD
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KPlateD

#include "../../../../../deps/airwindows/plugins/WinVST/kPlateD/kPlateD.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/kPlateD/kPlateDProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KPlateD() { return new kPlateD(nullptr); }
}

