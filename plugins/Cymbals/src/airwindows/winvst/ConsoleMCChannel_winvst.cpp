// WinVST compilation wrapper for ConsoleMCChannel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleMCChannel

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleMCChannel/ConsoleMCChannel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleMCChannel/ConsoleMCChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleMCChannel() { return new ConsoleMCChannel(nullptr); }
}

