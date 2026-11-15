// WinVST compilation wrapper for Channel4
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Channel4

#include "../../../../../deps/airwindows/plugins/WinVST/Channel4/Channel4.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Channel4/Channel4Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Channel4() { return new Channel4(nullptr); }
}

