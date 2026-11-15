// WinVST compilation wrapper for ADClip8
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ADClip8

#include "../../../../../deps/airwindows/plugins/WinVST/ADClip8/ADClip8.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ADClip8/ADClip8Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ADClip8() { return new ADClip8(nullptr); }
}

