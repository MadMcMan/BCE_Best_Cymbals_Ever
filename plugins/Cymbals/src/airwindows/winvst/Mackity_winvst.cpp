// WinVST compilation wrapper for Mackity
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Mackity

#include "../../../../../deps/airwindows/plugins/WinVST/Mackity/Mackity.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Mackity/MackityProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Mackity() { return new Mackity(nullptr); }
}

