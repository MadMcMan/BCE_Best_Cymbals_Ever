// WinVST compilation wrapper for Console0Buss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console0Buss

#include "../../../../../deps/airwindows/plugins/WinVST/Console0Buss/Console0Buss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console0Buss/Console0BussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console0Buss() { return new Console0Buss(nullptr); }
}

