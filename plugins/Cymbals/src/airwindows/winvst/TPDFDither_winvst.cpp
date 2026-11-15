// WinVST compilation wrapper for TPDFDither
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TPDFDither

#include "../../../../../deps/airwindows/plugins/WinVST/TPDFDither/TPDFDither.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TPDFDither/TPDFDitherProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TPDFDither() { return new TPDFDither(nullptr); }
}

