// WinVST compilation wrapper for Dark
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Dark

#include "../../../../../deps/airwindows/plugins/WinVST/Dark/Dark.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Dark/DarkProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Dark() { return new Dark(nullptr); }
}

