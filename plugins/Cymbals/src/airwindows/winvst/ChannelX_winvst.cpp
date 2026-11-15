// WinVST compilation wrapper for ChannelX
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ChannelX

#include "../../../../../deps/airwindows/plugins/WinVST/ChannelX/ChannelX.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ChannelX/ChannelXProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ChannelX() { return new ChannelX(nullptr); }
}

