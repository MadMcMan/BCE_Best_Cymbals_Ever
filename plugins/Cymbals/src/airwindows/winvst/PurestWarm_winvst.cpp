// WinVST compilation wrapper for PurestWarm
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestWarm

#include "../../../../../deps/airwindows/plugins/WinVST/PurestWarm/PurestWarm.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestWarm/PurestWarmProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestWarm() { return new PurestWarm(nullptr); }
}

