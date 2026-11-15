// WinVST compilation wrapper for Air
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Air

#include "../../../../../deps/airwindows/plugins/WinVST/Air/Air.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Air/AirProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Air() { return new Air(nullptr); }
}

