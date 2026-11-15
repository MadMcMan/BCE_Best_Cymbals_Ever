// WinVST compilation wrapper for ConsoleHPre
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleHPre

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleHPre/ConsoleHPre.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleHPre/ConsoleHPreProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleHPre() { return new ConsoleHPre(nullptr); }
}

