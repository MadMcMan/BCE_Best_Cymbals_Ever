// WinVST compilation wrapper for Cans
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Cans

#include "../../../../../deps/airwindows/plugins/WinVST/Cans/Cans.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Cans/CansProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Cans() { return new Cans(nullptr); }
}

