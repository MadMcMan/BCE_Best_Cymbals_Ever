// WinVST compilation wrapper for Channel9
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Channel9

#include "../../../../../deps/airwindows/plugins/WinVST/Channel9/Channel9.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Channel9/Channel9Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Channel9() { return new Channel9(nullptr); }
}

