// WinVST compilation wrapper for ConsoleXChannel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleXChannel

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleXChannel/ConsoleXChannel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleXChannel/ConsoleXChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleXChannel() { return new ConsoleXChannel(nullptr); }
}

