// WinVST compilation wrapper for Apicolypse
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Apicolypse

#include "../../../../../deps/airwindows/plugins/WinVST/Apicolypse/Apicolypse.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Apicolypse/ApicolypseProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Apicolypse() { return new Apicolypse(nullptr); }
}

