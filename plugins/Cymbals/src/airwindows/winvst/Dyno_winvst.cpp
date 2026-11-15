// WinVST compilation wrapper for Dyno
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Dyno

#include "../../../../../deps/airwindows/plugins/WinVST/Dyno/Dyno.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Dyno/DynoProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Dyno() { return new Dyno(nullptr); }
}

