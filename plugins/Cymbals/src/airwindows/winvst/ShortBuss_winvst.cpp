// WinVST compilation wrapper for ShortBuss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ShortBuss

#include "../../../../../deps/airwindows/plugins/WinVST/ShortBuss/ShortBuss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ShortBuss/ShortBussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ShortBuss() { return new ShortBuss(nullptr); }
}

