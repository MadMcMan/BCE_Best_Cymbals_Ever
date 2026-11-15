// WinVST compilation wrapper for Channel6
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Channel6

#include "../../../../../deps/airwindows/plugins/WinVST/Channel6/Channel6.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Channel6/Channel6Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Channel6() { return new Channel6(nullptr); }
}

