// WinVST compilation wrapper for Logical4
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Logical4

#include "../../../../../deps/airwindows/plugins/WinVST/Logical4/Logical4.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Logical4/Logical4Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Logical4() { return new Logical4(nullptr); }
}

