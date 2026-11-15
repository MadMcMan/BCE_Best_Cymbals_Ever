// WinVST compilation wrapper for Cabs2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Cabs2

#include "../../../../../deps/airwindows/plugins/WinVST/Cabs2/Cabs2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Cabs2/Cabs2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Cabs2() { return new Cabs2(nullptr); }
}

