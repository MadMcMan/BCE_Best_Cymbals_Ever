// WinVST compilation wrapper for MV
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_MV

#include "../../../../../deps/airwindows/plugins/WinVST/MV/MV.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/MV/MVProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_MV() { return new MV(nullptr); }
}

