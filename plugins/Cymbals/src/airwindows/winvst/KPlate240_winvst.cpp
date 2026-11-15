// WinVST compilation wrapper for KPlate240
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KPlate240

#include "../../../../../deps/airwindows/plugins/WinVST/KPlate240/KPlate240.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/KPlate240/KPlate240Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KPlate240() { return new kPlate240(nullptr); }
}

