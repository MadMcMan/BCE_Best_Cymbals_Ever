// WinVST compilation wrapper for TPDFWide
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TPDFWide

#include "../../../../../deps/airwindows/plugins/WinVST/TPDFWide/TPDFWide.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TPDFWide/TPDFWideProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TPDFWide() { return new TPDFWide(nullptr); }
}

