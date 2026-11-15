// WinVST compilation wrapper for Console8ChannelHype
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console8ChannelHype

#include "../../../../../deps/airwindows/plugins/WinVST/Console8ChannelHype/Console8ChannelHype.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console8ChannelHype/Console8ChannelHypeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console8ChannelHype() { return new Console8ChannelHype(nullptr); }
}

