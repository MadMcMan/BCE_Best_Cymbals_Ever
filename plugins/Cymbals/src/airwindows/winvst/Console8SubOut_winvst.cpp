// WinVST compilation wrapper for Console8SubOut
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console8SubOut

#include "../../../../../deps/airwindows/plugins/WinVST/Console8SubOut/Console8SubOut.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console8SubOut/Console8SubOutProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console8SubOut() { return new Console8SubOut(nullptr); }
}

