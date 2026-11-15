// WinVST compilation wrapper for Cider
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Cider

#include "../../../../../deps/airwindows/plugins/WinVST/Cider/Cider.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Cider/CiderProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Cider() { return new Cider(nullptr); }
}

