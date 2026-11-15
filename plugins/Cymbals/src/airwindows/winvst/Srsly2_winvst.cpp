// WinVST compilation wrapper for Srsly2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Srsly2

#include "../../../../../deps/airwindows/plugins/WinVST/Srsly2/Srsly2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Srsly2/Srsly2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Srsly2() { return new Srsly2(nullptr); }
}

