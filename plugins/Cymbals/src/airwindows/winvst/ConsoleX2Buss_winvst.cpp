// WinVST compilation wrapper for ConsoleX2Buss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleX2Buss

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleX2Buss/ConsoleX2Buss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleX2Buss/ConsoleX2BussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleX2Buss() { return new ConsoleX2Buss(nullptr); }
}

