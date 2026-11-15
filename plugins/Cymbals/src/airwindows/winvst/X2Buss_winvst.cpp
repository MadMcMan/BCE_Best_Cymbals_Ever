// WinVST compilation wrapper for X2Buss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_X2Buss

#include "../../../../../deps/airwindows/plugins/WinVST/X2Buss/X2Buss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/X2Buss/X2BussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_X2Buss() { return new X2Buss(nullptr); }
}

