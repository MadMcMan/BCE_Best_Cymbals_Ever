// WinVST compilation wrapper for Console8ChannelOut
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console8ChannelOut

#include "../../../../../deps/airwindows/plugins/WinVST/Console8ChannelOut/Console8ChannelOut.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console8ChannelOut/Console8ChannelOutProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console8ChannelOut() { return new Console8ChannelOut(nullptr); }
}

