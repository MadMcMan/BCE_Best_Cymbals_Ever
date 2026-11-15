// WinVST compilation wrapper for EveryConsole
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_EveryConsole

#include "../../../../../deps/airwindows/plugins/WinVST/EveryConsole/EveryConsole.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/EveryConsole/EveryConsoleProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_EveryConsole() { return new EveryConsole(nullptr); }
}

