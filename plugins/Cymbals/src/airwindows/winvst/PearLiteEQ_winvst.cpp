// WinVST compilation wrapper for PearLiteEQ
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PearLiteEQ

#include "../../../../../deps/airwindows/plugins/WinVST/PearLiteEQ/PearLiteEQ.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PearLiteEQ/PearLiteEQProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PearLiteEQ() { return new PearLiteEQ(nullptr); }
}

