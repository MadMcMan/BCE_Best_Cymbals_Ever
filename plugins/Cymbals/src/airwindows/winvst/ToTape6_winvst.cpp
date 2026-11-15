// WinVST compilation wrapper for ToTape6
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ToTape6

#include "../../../../../deps/airwindows/plugins/WinVST/ToTape6/ToTape6.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ToTape6/ToTape6Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ToTape6() { return new ToTape6(nullptr); }
}

