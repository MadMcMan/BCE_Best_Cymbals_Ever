// WinVST compilation wrapper for AngleEQ
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_AngleEQ

#include "../../../../../deps/airwindows/plugins/WinVST/AngleEQ/AngleEQ.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/AngleEQ/AngleEQProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_AngleEQ() { return new AngleEQ(nullptr); }
}

