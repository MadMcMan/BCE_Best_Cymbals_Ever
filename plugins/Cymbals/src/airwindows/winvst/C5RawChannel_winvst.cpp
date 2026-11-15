// WinVST compilation wrapper for C5RawChannel
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_C5RawChannel

#include "../../../../../deps/airwindows/plugins/WinVST/C5RawChannel/C5RawChannel.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/C5RawChannel/C5RawChannelProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_C5RawChannel() { return new C5RawChannel(nullptr); }
}

