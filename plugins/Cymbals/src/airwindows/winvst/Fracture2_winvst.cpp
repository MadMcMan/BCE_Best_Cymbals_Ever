// WinVST compilation wrapper for Fracture2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Fracture2

#include "../../../../../deps/airwindows/plugins/WinVST/Fracture2/Fracture2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Fracture2/Fracture2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Fracture2() { return new Fracture2(nullptr); }
}

