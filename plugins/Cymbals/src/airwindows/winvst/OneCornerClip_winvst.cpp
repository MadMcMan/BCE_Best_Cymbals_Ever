// WinVST compilation wrapper for OneCornerClip
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_OneCornerClip

#include "../../../../../deps/airwindows/plugins/WinVST/OneCornerClip/OneCornerClip.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/OneCornerClip/OneCornerClipProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_OneCornerClip() { return new OneCornerClip(nullptr); }
}

