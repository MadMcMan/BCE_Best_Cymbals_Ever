// WinVST compilation wrapper for Console8BussIn
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console8BussIn

#include "../../../../../deps/airwindows/plugins/WinVST/Console8BussIn/Console8BussIn.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console8BussIn/Console8BussInProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console8BussIn() { return new Console8BussIn(nullptr); }
}

