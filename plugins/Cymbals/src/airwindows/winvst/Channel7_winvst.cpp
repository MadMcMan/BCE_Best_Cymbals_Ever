// WinVST compilation wrapper for Channel7
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Channel7

#include "../../../../../deps/airwindows/plugins/WinVST/Channel7/Channel7.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Channel7/Channel7Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Channel7() { return new Channel7(nullptr); }
}

