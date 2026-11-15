// WinVST compilation wrapper for ZLowpass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ZLowpass

#include "../../../../../deps/airwindows/plugins/WinVST/ZLowpass/ZLowpass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ZLowpass/ZLowpassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ZLowpass() { return new ZLowpass(nullptr); }
}

