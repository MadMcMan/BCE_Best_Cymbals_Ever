// WinVST compilation wrapper for Console7Channel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console7Channel

#include "../../../../../deps/airwindows/plugins/WinVST/Console7Channel/Console7Channel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console7Channel/Console7ChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console7Channel() { return new Console7Channel(nullptr); }
}

