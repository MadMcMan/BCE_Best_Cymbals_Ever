// WinVST compilation wrapper for EveryTrim
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_EveryTrim

#include "../../../../../deps/airwindows/plugins/WinVST/EveryTrim/EveryTrim.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/EveryTrim/EveryTrimProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_EveryTrim() { return new EveryTrim(nullptr); }
}

