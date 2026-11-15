// WinVST compilation wrapper for AutoPan
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_AutoPan

#include "../../../../../deps/airwindows/plugins/WinVST/AutoPan/AutoPan.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/AutoPan/AutoPanProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_AutoPan() { return new AutoPan(nullptr); }
}

