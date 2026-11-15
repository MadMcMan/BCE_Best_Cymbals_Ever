// WinVST compilation wrapper for Hermepass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Hermepass

#include "../../../../../deps/airwindows/plugins/WinVST/Hermepass/Hermepass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Hermepass/HermepassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Hermepass() { return new Hermepass(nullptr); }
}

