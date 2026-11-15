// WinVST compilation wrapper for Spiral2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Spiral2

#include "../../../../../deps/airwindows/plugins/WinVST/Spiral2/Spiral2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Spiral2/Spiral2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Spiral2() { return new Spiral2(nullptr); }
}

