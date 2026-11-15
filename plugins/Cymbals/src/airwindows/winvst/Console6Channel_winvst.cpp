// WinVST compilation wrapper for Console6Channel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console6Channel

#include "../../../../../deps/airwindows/plugins/WinVST/Console6Channel/Console6Channel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console6Channel/Console6ChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console6Channel() { return new Console6Channel(nullptr); }
}

