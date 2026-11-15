// WinVST compilation wrapper for DeHiss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DeHiss

#include "../../../../../deps/airwindows/plugins/WinVST/DeHiss/DeHiss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DeHiss/DeHissProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DeHiss() { return new DeHiss(nullptr); }
}

