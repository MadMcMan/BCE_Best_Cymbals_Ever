// WinVST compilation wrapper for FinalClip
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_FinalClip

#include "../../../../../deps/airwindows/plugins/WinVST/FinalClip/FinalClip.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/FinalClip/FinalClipProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_FinalClip() { return new FinalClip(nullptr); }
}

