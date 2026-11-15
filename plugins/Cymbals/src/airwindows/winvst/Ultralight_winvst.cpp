// WinVST compilation wrapper for Ultralight
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Ultralight

#include "../../../../../deps/airwindows/plugins/WinVST/Ultralight/Ultralight.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Ultralight/UltralightProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Ultralight() { return new Ultralight(nullptr); }
}

