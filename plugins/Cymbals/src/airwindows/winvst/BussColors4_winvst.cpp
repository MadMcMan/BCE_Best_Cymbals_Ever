// WinVST compilation wrapper for BussColors4
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BussColors4

#include "../../../../../deps/airwindows/plugins/WinVST/BussColors4/BussColors4.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BussColors4/BussColors4Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BussColors4() { return new BussColors4(nullptr); }
}

