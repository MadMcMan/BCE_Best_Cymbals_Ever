// WinVST compilation wrapper for PurestConsoleBuss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestConsoleBuss

#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsoleBuss/PurestConsoleBuss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsoleBuss/PurestConsoleBussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestConsoleBuss() { return new PurestConsoleBuss(nullptr); }
}

