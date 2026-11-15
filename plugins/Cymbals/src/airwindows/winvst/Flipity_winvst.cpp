// WinVST compilation wrapper for Flipity
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Flipity

#include "../../../../../deps/airwindows/plugins/WinVST/Flipity/Flipity.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Flipity/FlipityProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Flipity() { return new Flipity(nullptr); }
}

