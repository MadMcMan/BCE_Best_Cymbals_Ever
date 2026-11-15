// WinVST compilation wrapper for ConsoleHChannel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleHChannel

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleHChannel/ConsoleHChannel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleHChannel/ConsoleHChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleHChannel() { return new ConsoleHChannel(nullptr); }
}

