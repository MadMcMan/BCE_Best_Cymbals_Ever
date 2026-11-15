// WinVST compilation wrapper for PurestConsole2Channel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestConsole2Channel

#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsole2Channel/PurestConsole2Channel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsole2Channel/PurestConsole2ChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestConsole2Channel() { return new PurestConsole2Channel(nullptr); }
}

