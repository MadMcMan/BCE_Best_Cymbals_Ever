// WinVST compilation wrapper for WoodenBox
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_WoodenBox

#include "../../../../../deps/airwindows/plugins/WinVST/WoodenBox/WoodenBox.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/WoodenBox/WoodenBoxProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_WoodenBox() { return new WoodenBox(nullptr); }
}

