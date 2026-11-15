// WinVST compilation wrapper for DubSub2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DubSub2

#include "../../../../../deps/airwindows/plugins/WinVST/DubSub2/DubSub2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DubSub2/DubSub2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DubSub2() { return new DubSub2(nullptr); }
}

