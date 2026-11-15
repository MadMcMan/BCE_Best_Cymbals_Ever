// WinVST compilation wrapper for Console4Channel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console4Channel

#include "../../../../../deps/airwindows/plugins/WinVST/Console4Channel/Console4Channel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console4Channel/Console4ChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console4Channel() { return new Console4Channel(nullptr); }
}

