// WinVST compilation wrapper for Console8ChannelIn
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console8ChannelIn

#include "../../../../../deps/airwindows/plugins/WinVST/Console8ChannelIn/Console8ChannelIn.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console8ChannelIn/Console8ChannelInProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console8ChannelIn() { return new Console8ChannelIn(nullptr); }
}

