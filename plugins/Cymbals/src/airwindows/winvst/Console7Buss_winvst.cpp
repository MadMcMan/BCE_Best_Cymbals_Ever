// WinVST compilation wrapper for Console7Buss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console7Buss

#include "../../../../../deps/airwindows/plugins/WinVST/Console7Buss/Console7Buss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console7Buss/Console7BussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console7Buss() { return new Console7Buss(nullptr); }
}

