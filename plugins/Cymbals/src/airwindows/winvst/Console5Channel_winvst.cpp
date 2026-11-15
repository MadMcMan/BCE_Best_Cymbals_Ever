// WinVST compilation wrapper for Console5Channel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console5Channel

#include "../../../../../deps/airwindows/plugins/WinVST/Console5Channel/Console5Channel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console5Channel/Console5ChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console5Channel() { return new Console5Channel(nullptr); }
}

