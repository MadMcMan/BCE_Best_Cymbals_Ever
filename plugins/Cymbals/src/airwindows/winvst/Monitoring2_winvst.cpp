// WinVST compilation wrapper for Monitoring2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Monitoring2

#include "../../../../../deps/airwindows/plugins/WinVST/Monitoring2/Monitoring2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Monitoring2/Monitoring2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Monitoring2() { return new Monitoring2(nullptr); }
}

