// WinVST compilation wrapper for Hombre
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Hombre

#include "../../../../../deps/airwindows/plugins/WinVST/Hombre/Hombre.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Hombre/HombreProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Hombre() { return new Hombre(nullptr); }
}

