// WinVST compilation wrapper for Desk4
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Desk4

#include "../../../../../deps/airwindows/plugins/WinVST/Desk4/Desk4.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Desk4/Desk4Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Desk4() { return new Desk4(nullptr); }
}

