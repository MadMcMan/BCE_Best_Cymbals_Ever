// WinVST compilation wrapper for PurestWarm2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestWarm2

#include "../../../../../deps/airwindows/plugins/WinVST/PurestWarm2/PurestWarm2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestWarm2/PurestWarm2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestWarm2() { return new PurestWarm2(nullptr); }
}

