// WinVST compilation wrapper for ZHighpass2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ZHighpass2

#include "../../../../../deps/airwindows/plugins/WinVST/ZHighpass2/ZHighpass2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ZHighpass2/ZHighpass2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ZHighpass2() { return new ZHighpass2(nullptr); }
}

