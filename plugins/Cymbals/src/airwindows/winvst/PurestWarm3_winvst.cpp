// WinVST compilation wrapper for PurestWarm3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestWarm3

#include "../../../../../deps/airwindows/plugins/WinVST/PurestWarm3/PurestWarm3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestWarm3/PurestWarm3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestWarm3() { return new PurestWarm3(nullptr); }
}

