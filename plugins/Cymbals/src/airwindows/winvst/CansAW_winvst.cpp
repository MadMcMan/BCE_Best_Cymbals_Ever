// WinVST compilation wrapper for CansAW
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_CansAW

#include "../../../../../deps/airwindows/plugins/WinVST/CansAW/CansAW.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/CansAW/CansAWProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_CansAW() { return new CansAW(nullptr); }
}

