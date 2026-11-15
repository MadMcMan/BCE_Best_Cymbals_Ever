// WinVST compilation wrapper for Ditherbox
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Ditherbox

#include "../../../../../deps/airwindows/plugins/WinVST/Ditherbox/Ditherbox.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Ditherbox/DitherboxProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Ditherbox() { return new Ditherbox(nullptr); }
}

