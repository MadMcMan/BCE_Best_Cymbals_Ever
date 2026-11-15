// WinVST compilation wrapper for Crystal
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Crystal

#include "../../../../../deps/airwindows/plugins/WinVST/Crystal/Crystal.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Crystal/CrystalProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Crystal() { return new Crystal(nullptr); }
}

