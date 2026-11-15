// WinVST compilation wrapper for ZRegion
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ZRegion

#include "../../../../../deps/airwindows/plugins/WinVST/ZRegion/ZRegion.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ZRegion/ZRegionProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ZRegion() { return new ZRegion(nullptr); }
}

