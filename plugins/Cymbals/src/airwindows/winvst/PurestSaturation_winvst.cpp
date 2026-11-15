// WinVST compilation wrapper for PurestSaturation
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestSaturation

#include "../../../../../deps/airwindows/plugins/WinVST/PurestSaturation/PurestSaturation.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestSaturation/PurestSaturationProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestSaturation() { return new PurestSaturation(nullptr); }
}

