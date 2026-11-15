// WinVST compilation wrapper for ConsoleHBuss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleHBuss

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleHBuss/ConsoleHBuss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleHBuss/ConsoleHBussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleHBuss() { return new ConsoleHBuss(nullptr); }
}

