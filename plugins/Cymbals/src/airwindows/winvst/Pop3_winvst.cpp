// WinVST compilation wrapper for Pop3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Pop3

#include "../../../../../deps/airwindows/plugins/WinVST/Pop3/Pop3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Pop3/Pop3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Pop3() { return new Pop3(nullptr); }
}

