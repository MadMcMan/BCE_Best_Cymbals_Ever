// WinVST compilation wrapper for Cojones
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Cojones

#include "../../../../../deps/airwindows/plugins/WinVST/Cojones/Cojones.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Cojones/CojonesProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Cojones() { return new Cojones(nullptr); }
}

