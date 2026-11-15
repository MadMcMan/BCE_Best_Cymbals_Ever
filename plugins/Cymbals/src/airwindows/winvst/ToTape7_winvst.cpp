// WinVST compilation wrapper for ToTape7
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ToTape7

#include "../../../../../deps/airwindows/plugins/WinVST/ToTape7/ToTape7.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ToTape7/ToTape7Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ToTape7() { return new ToTape7(nullptr); }
}

