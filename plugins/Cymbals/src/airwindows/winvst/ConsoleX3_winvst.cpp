// WinVST compilation wrapper for ConsoleX3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleX3

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleX3/ConsoleX3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleX3/ConsoleX3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleX3() { return new ConsoleX3(nullptr); }
}

