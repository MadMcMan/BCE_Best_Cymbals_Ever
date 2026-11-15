// WinVST compilation wrapper for ConsoleXBuss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleXBuss

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleXBuss/ConsoleXBuss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleXBuss/ConsoleXBussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleXBuss() { return new ConsoleXBuss(nullptr); }
}

