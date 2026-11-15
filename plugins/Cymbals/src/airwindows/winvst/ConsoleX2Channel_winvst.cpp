// WinVST compilation wrapper for ConsoleX2Channel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleX2Channel

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleX2Channel/ConsoleX2Channel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleX2Channel/ConsoleX2ChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleX2Channel() { return new ConsoleX2Channel(nullptr); }
}

