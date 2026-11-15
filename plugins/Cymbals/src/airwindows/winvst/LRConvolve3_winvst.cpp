// WinVST compilation wrapper for LRConvolve3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_LRConvolve3

#include "../../../../../deps/airwindows/plugins/WinVST/LRConvolve3/LRConvolve3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/LRConvolve3/LRConvolve3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_LRConvolve3() { return new LRConvolve3(nullptr); }
}

