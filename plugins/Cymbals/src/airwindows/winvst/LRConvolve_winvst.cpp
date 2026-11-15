// WinVST compilation wrapper for LRConvolve
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_LRConvolve

#include "../../../../../deps/airwindows/plugins/WinVST/LRConvolve/LRConvolve.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/LRConvolve/LRConvolveProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_LRConvolve() { return new LRConvolve(nullptr); }
}

