// WinVST compilation wrapper for ClipSoftly
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ClipSoftly

#include "../../../../../deps/airwindows/plugins/WinVST/ClipSoftly/ClipSoftly.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ClipSoftly/ClipSoftlyProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ClipSoftly() { return new ClipSoftly(nullptr); }
}

