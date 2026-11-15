// WinVST compilation wrapper for Channel5
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Channel5

#include "../../../../../deps/airwindows/plugins/WinVST/Channel5/Channel5.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Channel5/Channel5Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Channel5() { return new Channel5(nullptr); }
}

