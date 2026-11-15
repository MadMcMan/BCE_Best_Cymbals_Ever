// WinVST compilation wrapper for Precious
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Precious

#include "../../../../../deps/airwindows/plugins/WinVST/Precious/Precious.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Precious/PreciousProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Precious() { return new Precious(nullptr); }
}

