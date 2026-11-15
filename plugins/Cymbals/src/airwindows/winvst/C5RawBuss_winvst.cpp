// WinVST compilation wrapper for C5RawBuss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_C5RawBuss

#include "../../../../../deps/airwindows/plugins/WinVST/C5RawBuss/C5RawBuss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/C5RawBuss/C5RawBussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_C5RawBuss() { return new C5RawBuss(nullptr); }
}

