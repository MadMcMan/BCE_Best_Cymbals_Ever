// WinVST compilation wrapper for HipCrush
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_HipCrush

#include "../../../../../deps/airwindows/plugins/WinVST/HipCrush/HipCrush.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/HipCrush/HipCrushProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_HipCrush() { return new HipCrush(nullptr); }
}

