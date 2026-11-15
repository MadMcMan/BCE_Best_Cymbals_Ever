// WinVST compilation wrapper for YNotLowpass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_YNotLowpass

#include "../../../../../deps/airwindows/plugins/WinVST/YNotLowpass/YNotLowpass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/YNotLowpass/YNotLowpassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_YNotLowpass() { return new YNotLowpass(nullptr); }
}

