// WinVST compilation wrapper for TakeCare
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TakeCare

#include "../../../../../deps/airwindows/plugins/WinVST/TakeCare/TakeCare.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TakeCare/TakeCareProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TakeCare() { return new TakeCare(nullptr); }
}

