// WinVST compilation wrapper for Slew
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Slew

#include "../../../../../deps/airwindows/plugins/WinVST/Slew/Slew.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Slew/SlewProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Slew() { return new Slew(nullptr); }
}

