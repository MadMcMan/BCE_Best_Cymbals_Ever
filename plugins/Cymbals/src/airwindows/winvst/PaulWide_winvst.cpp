// WinVST compilation wrapper for PaulWide
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PaulWide

#include "../../../../../deps/airwindows/plugins/WinVST/PaulWide/PaulWide.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PaulWide/PaulWideProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PaulWide() { return new PaulWide(nullptr); }
}

