// WinVST compilation wrapper for ConsoleLABuss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ConsoleLABuss

#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleLABuss/ConsoleLABuss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ConsoleLABuss/ConsoleLABussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ConsoleLABuss() { return new ConsoleLABuss(nullptr); }
}

