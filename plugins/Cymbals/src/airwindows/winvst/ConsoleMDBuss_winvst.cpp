// WinVST compilation wrapper for ConsoleMDBuss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleMDBuss

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleMDBuss/ConsoleMDBuss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleMDBuss/ConsoleMDBussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleMDBuss() { return new ConsoleMDBuss(nullptr); }
}

