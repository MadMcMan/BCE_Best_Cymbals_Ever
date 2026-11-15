// WinVST compilation wrapper for Ultralight2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Ultralight2

#include "../../../../../deps/airwindows/plugins/WinVST/Ultralight2/Ultralight2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Ultralight2/Ultralight2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Ultralight2() { return new Ultralight2(nullptr); }
}

