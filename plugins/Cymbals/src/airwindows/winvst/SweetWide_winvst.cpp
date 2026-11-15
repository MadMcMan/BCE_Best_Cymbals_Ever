// WinVST compilation wrapper for SweetWide
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_SweetWide

#include "../../../../../deps/airwindows/plugins/WinVST/SweetWide/SweetWide.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/SweetWide/SweetWideProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_SweetWide() { return new SweetWide(nullptr); }
}

