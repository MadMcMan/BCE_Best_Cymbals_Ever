// WinVST compilation wrapper for ClipOnly3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ClipOnly3

#include "../../../../../deps/airwindows/plugins/WinVST/ClipOnly3/ClipOnly3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ClipOnly3/ClipOnly3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ClipOnly3() { return new ClipOnly3(nullptr); }
}

