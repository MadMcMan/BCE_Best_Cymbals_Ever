// WinVST compilation wrapper for Console8SubHype
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console8SubHype

#include "../../../../../deps/airwindows/plugins/WinVST/Console8SubHype/Console8SubHype.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console8SubHype/Console8SubHypeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console8SubHype() { return new Console8SubHype(nullptr); }
}

