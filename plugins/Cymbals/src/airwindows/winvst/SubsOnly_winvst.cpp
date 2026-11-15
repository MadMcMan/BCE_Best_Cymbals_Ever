// WinVST compilation wrapper for SubsOnly
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_SubsOnly

#include "../../../../../deps/airwindows/plugins/WinVST/SubsOnly/SubsOnly.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/SubsOnly/SubsOnlyProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_SubsOnly() { return new SubsOnly(nullptr); }
}

