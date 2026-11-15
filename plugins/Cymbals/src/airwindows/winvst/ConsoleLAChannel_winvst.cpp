// WinVST compilation wrapper for ConsoleLAChannel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleLAChannel

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleLAChannel/ConsoleLAChannel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleLAChannel/ConsoleLAChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleLAChannel() { return new ConsoleLAChannel(nullptr); }
}

