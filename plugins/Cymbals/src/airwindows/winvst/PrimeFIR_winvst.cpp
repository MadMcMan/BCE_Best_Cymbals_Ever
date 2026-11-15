// WinVST compilation wrapper for PrimeFIR
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PrimeFIR

#include "../../../../../deps/airwindows/plugins/WinVST/PrimeFIR/PrimeFIR.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PrimeFIR/PrimeFIRProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PrimeFIR() { return new PrimeFIR(nullptr); }
}

