// WinVST compilation wrapper for ConsoleMDChannel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleMDChannel

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleMDChannel/ConsoleMDChannel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleMDChannel/ConsoleMDChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleMDChannel() { return new ConsoleMDChannel(nullptr); }
}

