// WinVST compilation wrapper for ToTape5
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ToTape5

#include "../../../../../deps/airwindows/plugins/WinVST/ToTape5/ToTape5.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ToTape5/ToTape5Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ToTape5() { return new ToTape5(nullptr); }
}

