// WinVST compilation wrapper for Console6Buss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console6Buss

#include "../../../../../deps/airwindows/plugins/WinVST/Console6Buss/Console6Buss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console6Buss/Console6BussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console6Buss() { return new Console6Buss(nullptr); }
}

