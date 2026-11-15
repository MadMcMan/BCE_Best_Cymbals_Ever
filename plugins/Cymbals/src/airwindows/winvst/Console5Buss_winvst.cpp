// WinVST compilation wrapper for Console5Buss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console5Buss

#include "../../../../../deps/airwindows/plugins/WinVST/Console5Buss/Console5Buss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console5Buss/Console5BussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console5Buss() { return new Console5Buss(nullptr); }
}

