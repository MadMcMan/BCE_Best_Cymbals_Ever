// WinVST compilation wrapper for Console9Channel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console9Channel

#include "../../../../../deps/airwindows/plugins/WinVST/Console9Channel/Console9Channel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console9Channel/Console9ChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console9Channel() { return new Console9Channel(nullptr); }
}

