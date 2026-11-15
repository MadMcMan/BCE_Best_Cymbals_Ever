// WinVST compilation wrapper for EverySlew
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_EverySlew

#include "../../../../../deps/airwindows/plugins/WinVST/EverySlew/EverySlew.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/EverySlew/EverySlewProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_EverySlew() { return new EverySlew(nullptr); }
}

