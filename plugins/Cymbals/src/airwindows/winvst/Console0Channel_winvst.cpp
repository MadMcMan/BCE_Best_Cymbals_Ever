// WinVST compilation wrapper for Console0Channel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console0Channel

#include "../../../../../deps/airwindows/plugins/WinVST/Console0Channel/Console0Channel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console0Channel/Console0ChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console0Channel() { return new Console0Channel(nullptr); }
}

