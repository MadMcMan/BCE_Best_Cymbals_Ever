// WinVST compilation wrapper for Coils2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Coils2

#include "../../../../../deps/airwindows/plugins/WinVST/Coils2/Coils2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Coils2/Coils2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Coils2() { return new Coils2(nullptr); }
}

