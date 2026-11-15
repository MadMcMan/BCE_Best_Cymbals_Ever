// WinVST compilation wrapper for Console8BussOut
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console8BussOut

#include "../../../../../deps/airwindows/plugins/WinVST/Console8BussOut/Console8BussOut.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console8BussOut/Console8BussOutProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console8BussOut() { return new Console8BussOut(nullptr); }
}

