// WinVST compilation wrapper for Silken
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Silken

#include "../../../../../deps/airwindows/plugins/WinVST/Silken/Silken.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Silken/SilkenProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Silken() { return new Silken(nullptr); }
}

