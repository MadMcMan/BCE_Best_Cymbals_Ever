// WinVST compilation wrapper for MV2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_MV2

#include "../../../../../deps/airwindows/plugins/WinVST/MV2/MV2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/MV2/MV2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_MV2() { return new MV2(nullptr); }
}

