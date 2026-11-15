// WinVST compilation wrapper for PurestDualPan
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestDualPan

#include "../../../../../deps/airwindows/plugins/WinVST/PurestDualPan/PurestDualPan.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestDualPan/PurestDualPanProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestDualPan() { return new PurestDualPan(nullptr); }
}

