// WinVST compilation wrapper for Isolator
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Isolator

#include "../../../../../deps/airwindows/plugins/WinVST/Isolator/Isolator.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Isolator/IsolatorProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Isolator() { return new Isolator(nullptr); }
}

