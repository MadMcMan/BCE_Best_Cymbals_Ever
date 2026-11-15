// WinVST compilation wrapper for Tube2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Tube2

#include "../../../../../deps/airwindows/plugins/WinVST/Tube2/Tube2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Tube2/Tube2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Tube2() { return new Tube2(nullptr); }
}

