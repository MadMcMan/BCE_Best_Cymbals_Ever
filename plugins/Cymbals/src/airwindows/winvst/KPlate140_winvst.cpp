// WinVST compilation wrapper for KPlate140
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KPlate140

#include "../../../../../deps/airwindows/plugins/WinVST/kPlate140/kPlate140.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/kPlate140/kPlate140Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KPlate140() { return new kPlate140(nullptr); }
}

