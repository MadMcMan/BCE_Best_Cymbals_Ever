// WinVST compilation wrapper for ToTape8
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ToTape8

#include "../../../../../deps/airwindows/plugins/WinVST/ToTape8/ToTape8.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ToTape8/ToTape8Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ToTape8() { return new ToTape8(nullptr); }
}

