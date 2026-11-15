// WinVST compilation wrapper for Discontinuity
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Discontinuity

#include "../../../../../deps/airwindows/plugins/WinVST/Discontinuity/Discontinuity.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Discontinuity/DiscontinuityProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Discontinuity() { return new Discontinuity(nullptr); }
}

