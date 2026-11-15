// WinVST compilation wrapper for ADClip7
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ADClip7

#include "../../../../../deps/airwindows/plugins/WinVST/ADClip7/ADClip7.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ADClip7/ADClip7Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ADClip7() { return new ADClip7(nullptr); }
}

