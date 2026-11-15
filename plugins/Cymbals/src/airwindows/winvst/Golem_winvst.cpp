// WinVST compilation wrapper for Golem
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Golem

#include "../../../../../deps/airwindows/plugins/WinVST/Golem/Golem.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Golem/GolemProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Golem() { return new Golem(nullptr); }
}

