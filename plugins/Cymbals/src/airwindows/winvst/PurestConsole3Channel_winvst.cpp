// WinVST compilation wrapper for PurestConsole3Channel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestConsole3Channel

#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsole3Channel/PurestConsole3Channel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsole3Channel/PurestConsole3ChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestConsole3Channel() { return new PurestConsole3Channel(nullptr); }
}

