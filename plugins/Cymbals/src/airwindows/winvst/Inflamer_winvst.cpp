// WinVST compilation wrapper for Inflamer
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Inflamer

#include "../../../../../deps/airwindows/plugins/WinVST/Inflamer/Inflamer.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Inflamer/InflamerProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Inflamer() { return new Inflamer(nullptr); }
}

