// WinVST compilation wrapper for XLowpass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_XLowpass

#include "../../../../../deps/airwindows/plugins/WinVST/XLowpass/XLowpass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/XLowpass/XLowpassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_XLowpass() { return new XLowpass(nullptr); }
}

