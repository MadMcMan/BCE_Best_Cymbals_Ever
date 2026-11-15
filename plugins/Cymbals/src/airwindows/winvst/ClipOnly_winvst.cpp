// WinVST compilation wrapper for ClipOnly
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ClipOnly

#include "../../../../../deps/airwindows/plugins/WinVST/ClipOnly/ClipOnly.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ClipOnly/ClipOnlyProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ClipOnly() { return new ClipOnly(nullptr); }
}

