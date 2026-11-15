// WinVST compilation wrapper for Focus
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Focus

#include "../../../../../deps/airwindows/plugins/WinVST/Focus/Focus.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Focus/FocusProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Focus() { return new Focus(nullptr); }
}

