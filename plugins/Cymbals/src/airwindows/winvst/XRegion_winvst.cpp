// WinVST compilation wrapper for XRegion
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_XRegion

#include "../../../../../deps/airwindows/plugins/WinVST/XRegion/XRegion.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/XRegion/XRegionProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_XRegion() { return new XRegion(nullptr); }
}

