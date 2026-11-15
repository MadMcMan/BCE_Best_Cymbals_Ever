// WinVST compilation wrapper for Channel8
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Channel8

#include "../../../../../deps/airwindows/plugins/WinVST/Channel8/Channel8.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Channel8/Channel8Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Channel8() { return new Channel8(nullptr); }
}

