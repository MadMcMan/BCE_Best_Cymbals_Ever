// WinVST compilation wrapper for KChamberAR
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KChamberAR

#include "../../../../../deps/airwindows/plugins/WinVST/kChamberAR/kChamberAR.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/kChamberAR/kChamberARProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KChamberAR() { return new kChamberAR(nullptr); }
}

