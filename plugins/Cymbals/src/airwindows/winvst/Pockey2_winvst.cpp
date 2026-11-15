// WinVST compilation wrapper for Pockey2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Pockey2

#include "../../../../../deps/airwindows/plugins/WinVST/Pockey2/Pockey2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Pockey2/Pockey2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Pockey2() { return new Pockey2(nullptr); }
}

