// WinVST compilation wrapper for Echojam
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Echojam

#include "../../../../../deps/airwindows/plugins/WinVST/Echojam/Echojam.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Echojam/EchojamProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Echojam() { return new Echojam(nullptr); }
}

