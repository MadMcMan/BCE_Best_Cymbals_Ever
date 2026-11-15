// WinVST compilation wrapper for Cabs
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Cabs

#include "../../../../../deps/airwindows/plugins/WinVST/Cabs/Cabs.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Cabs/CabsProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Cabs() { return new Cabs(nullptr); }
}

