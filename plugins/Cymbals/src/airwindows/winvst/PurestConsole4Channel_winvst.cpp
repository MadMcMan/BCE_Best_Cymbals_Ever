// WinVST compilation wrapper for PurestConsole4Channel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestConsole4Channel

#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsole4Channel/PurestConsole4Channel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsole4Channel/PurestConsole4ChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestConsole4Channel() { return new PurestConsole4Channel(nullptr); }
}

