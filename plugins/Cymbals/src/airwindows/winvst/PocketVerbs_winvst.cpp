// WinVST compilation wrapper for PocketVerbs
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PocketVerbs

#include "../../../../../deps/airwindows/plugins/WinVST/PocketVerbs/PocketVerbs.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PocketVerbs/PocketVerbsProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PocketVerbs() { return new PocketVerbs(nullptr); }
}

