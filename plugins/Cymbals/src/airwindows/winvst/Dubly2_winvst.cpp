// WinVST compilation wrapper for Dubly2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Dubly2

#include "../../../../../deps/airwindows/plugins/WinVST/Dubly2/Dubly2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Dubly2/Dubly2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Dubly2() { return new Dubly2(nullptr); }
}

