// WinVST compilation wrapper for Console8LiteChannel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console8LiteChannel

#include "../../../../../deps/airwindows/plugins/WinVST/Console8LiteChannel/Console8LiteChannel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console8LiteChannel/Console8LiteChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console8LiteChannel() { return new Console8LiteChannel(nullptr); }
}

