// WinVST compilation wrapper for Chamber2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Chamber2

#include "../../../../../deps/airwindows/plugins/WinVST/Chamber2/Chamber2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Chamber2/Chamber2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Chamber2() { return new Chamber2(nullptr); }
}

