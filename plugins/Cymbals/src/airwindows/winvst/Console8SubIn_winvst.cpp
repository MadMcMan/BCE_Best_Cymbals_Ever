// WinVST compilation wrapper for Console8SubIn
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console8SubIn

#include "../../../../../deps/airwindows/plugins/WinVST/Console8SubIn/Console8SubIn.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console8SubIn/Console8SubInProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console8SubIn() { return new Console8SubIn(nullptr); }
}

