// WinVST compilation wrapper for Srsly
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Srsly

#include "../../../../../deps/airwindows/plugins/WinVST/Srsly/Srsly.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Srsly/SrslyProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Srsly() { return new Srsly(nullptr); }
}

