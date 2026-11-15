// WinVST compilation wrapper for Console4Buss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console4Buss

#include "../../../../../deps/airwindows/plugins/WinVST/Console4Buss/Console4Buss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console4Buss/Console4BussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console4Buss() { return new Console4Buss(nullptr); }
}

