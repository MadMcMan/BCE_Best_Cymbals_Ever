// WinVST compilation wrapper for MackEQ
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_MackEQ

#include "../../../../../deps/airwindows/plugins/WinVST/MackEQ/MackEQ.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/MackEQ/MackEQProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_MackEQ() { return new MackEQ(nullptr); }
}

