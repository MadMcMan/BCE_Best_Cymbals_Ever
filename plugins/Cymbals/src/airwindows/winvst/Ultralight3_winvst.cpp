// WinVST compilation wrapper for Ultralight3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Ultralight3

#include "../../../../../deps/airwindows/plugins/WinVST/Ultralight3/Ultralight3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Ultralight3/Ultralight3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Ultralight3() { return new Ultralight3(nullptr); }
}

