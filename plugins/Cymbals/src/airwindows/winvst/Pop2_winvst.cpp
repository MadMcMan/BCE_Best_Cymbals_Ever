// WinVST compilation wrapper for Pop2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Pop2

#include "../../../../../deps/airwindows/plugins/WinVST/Pop2/Pop2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Pop2/Pop2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Pop2() { return new Pop2(nullptr); }
}

