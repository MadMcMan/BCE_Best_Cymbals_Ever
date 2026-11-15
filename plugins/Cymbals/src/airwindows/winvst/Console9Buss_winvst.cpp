// WinVST compilation wrapper for Console9Buss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console9Buss

#include "../../../../../deps/airwindows/plugins/WinVST/Console9Buss/Console9Buss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console9Buss/Console9BussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console9Buss() { return new Console9Buss(nullptr); }
}

