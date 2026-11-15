// WinVST compilation wrapper for HighImpact
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_HighImpact

#include "../../../../../deps/airwindows/plugins/WinVST/HighImpact/HighImpact.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/HighImpact/HighImpactProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_HighImpact() { return new HighImpact(nullptr); }
}

