// WinVST compilation wrapper for Hypersoft
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Hypersoft

#include "../../../../../deps/airwindows/plugins/WinVST/Hypersoft/Hypersoft.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Hypersoft/HypersoftProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Hypersoft() { return new Hypersoft(nullptr); }
}

