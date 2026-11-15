// WinVST compilation wrapper for ToTape9
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ToTape9

#include "../../../../../deps/airwindows/plugins/WinVST/ToTape9/ToTape9.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ToTape9/ToTape9Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ToTape9() { return new ToTape9(nullptr); }
}

