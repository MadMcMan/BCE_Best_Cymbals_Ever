// WinVST compilation wrapper for AtmosphereChannel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_AtmosphereChannel

#include "../../../../../deps/airwindows/plugins/WinVST/AtmosphereChannel/AtmosphereChannel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/AtmosphereChannel/AtmosphereChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_AtmosphereChannel() { return new AtmosphereChannel(nullptr); }
}

