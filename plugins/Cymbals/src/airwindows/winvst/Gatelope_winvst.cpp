// WinVST compilation wrapper for Gatelope
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Gatelope

#include "../../../../../deps/airwindows/plugins/WinVST/Gatelope/Gatelope.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Gatelope/GatelopeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Gatelope() { return new Gatelope(nullptr); }
}

