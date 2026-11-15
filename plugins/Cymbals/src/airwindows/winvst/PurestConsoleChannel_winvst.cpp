// WinVST compilation wrapper for PurestConsoleChannel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestConsoleChannel

#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsoleChannel/PurestConsoleChannel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsoleChannel/PurestConsoleChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestConsoleChannel() { return new PurestConsoleChannel(nullptr); }
}

