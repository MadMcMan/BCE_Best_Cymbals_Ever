// WinVST compilation wrapper for ConsoleX3Buss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleX3Buss

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleX3Buss/ConsoleX3Buss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleX3Buss/ConsoleX3BussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleX3Buss() { return new ConsoleX3Buss(nullptr); }
}

