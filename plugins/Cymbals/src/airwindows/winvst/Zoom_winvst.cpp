// WinVST compilation wrapper for Zoom
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Zoom

#include "../../../../../deps/airwindows/plugins/WinVST/Zoom/Zoom.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Zoom/ZoomProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Zoom() { return new Zoom(nullptr); }
}

