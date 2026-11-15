// WinVST compilation wrapper for ConsoleX3Channel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleX3Channel

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleX3Channel/ConsoleX3Channel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleX3Channel/ConsoleX3ChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleX3Channel() { return new ConsoleX3Channel(nullptr); }
}

