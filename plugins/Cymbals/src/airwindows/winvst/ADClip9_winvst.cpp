// WinVST compilation wrapper for ADClip9
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ADClip9

#include "../../../../../deps/airwindows/plugins/WinVST/ADClip9/ADClip9.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ADClip9/ADClip9Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ADClip9() { return new ADClip9(nullptr); }
}

