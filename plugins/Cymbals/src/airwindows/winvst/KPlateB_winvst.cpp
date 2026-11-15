// WinVST compilation wrapper for KPlateB
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KPlateB

#include "../../../../../deps/airwindows/plugins/WinVST/KPlateB/KPlateB.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/KPlateB/KPlateBProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KPlateB() { return new kPlateB(nullptr); }
}

