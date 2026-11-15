// WinVST compilation wrapper for SurgeTide
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_SurgeTide

#include "../../../../../deps/airwindows/plugins/WinVST/SurgeTide/SurgeTide.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/SurgeTide/SurgeTideProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_SurgeTide() { return new SurgeTide(nullptr); }
}

