// WinVST compilation wrapper for FatEQ
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_FatEQ

#include "../../../../../deps/airwindows/plugins/WinVST/FatEQ/FatEQ.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/FatEQ/FatEQProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_FatEQ() { return new FatEQ(nullptr); }
}

