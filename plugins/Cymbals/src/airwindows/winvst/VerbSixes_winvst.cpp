// WinVST compilation wrapper for VerbSixes
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_VerbSixes

#include "../../../../../deps/airwindows/plugins/WinVST/VerbSixes/VerbSixes.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/VerbSixes/VerbSixesProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_VerbSixes() { return new VerbSixes(nullptr); }
}

