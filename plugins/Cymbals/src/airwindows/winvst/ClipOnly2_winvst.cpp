// WinVST compilation wrapper for ClipOnly2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ClipOnly2

#include "../../../../../deps/airwindows/plugins/WinVST/ClipOnly2/ClipOnly2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ClipOnly2/ClipOnly2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ClipOnly2() { return new ClipOnly2(nullptr); }
}

