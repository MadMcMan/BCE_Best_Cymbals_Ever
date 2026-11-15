// WinVST compilation wrapper for DubPlate
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DubPlate

#include "../../../../../deps/airwindows/plugins/WinVST/DubPlate/DubPlate.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DubPlate/DubPlateProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DubPlate() { return new DubPlate(nullptr); }
}

