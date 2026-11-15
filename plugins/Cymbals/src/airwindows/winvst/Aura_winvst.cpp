// WinVST compilation wrapper for Aura
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Aura

#include "../../../../../deps/airwindows/plugins/WinVST/Aura/Aura.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Aura/AuraProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Aura() { return new Aura(nullptr); }
}

