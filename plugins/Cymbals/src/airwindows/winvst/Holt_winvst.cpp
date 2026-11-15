// WinVST compilation wrapper for Holt
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Holt

#include "../../../../../deps/airwindows/plugins/WinVST/Holt/Holt.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Holt/HoltProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Holt() { return new Holt(nullptr); }
}

