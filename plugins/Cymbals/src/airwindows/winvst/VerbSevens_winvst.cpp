// WinVST compilation wrapper for VerbSevens
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_VerbSevens

#include "../../../../../deps/airwindows/plugins/WinVST/VerbSevens/VerbSevens.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/VerbSevens/VerbSevensProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_VerbSevens() { return new VerbSevens(nullptr); }
}

