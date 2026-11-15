// WinVST compilation wrapper for DubPlate2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DubPlate2

#include "../../../../../deps/airwindows/plugins/WinVST/DubPlate2/DubPlate2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DubPlate2/DubPlate2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DubPlate2() { return new DubPlate2(nullptr); }
}

