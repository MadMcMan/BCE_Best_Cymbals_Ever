// WinVST compilation wrapper for VerbTiny
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_VerbTiny

#include "../../../../../deps/airwindows/plugins/WinVST/VerbTiny/VerbTiny.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/VerbTiny/VerbTinyProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_VerbTiny() { return new VerbTiny(nullptr); }
}

