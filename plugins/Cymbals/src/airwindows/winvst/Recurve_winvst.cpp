// WinVST compilation wrapper for Recurve
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Recurve

#include "../../../../../deps/airwindows/plugins/WinVST/Recurve/Recurve.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Recurve/RecurveProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Recurve() { return new Recurve(nullptr); }
}

