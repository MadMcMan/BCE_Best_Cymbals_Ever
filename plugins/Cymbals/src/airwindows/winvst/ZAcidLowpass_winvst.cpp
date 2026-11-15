// WinVST compilation wrapper for ZAcidLowpass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ZAcidLowpass

#include "../../../../../deps/airwindows/plugins/WinVST/ZAcidLowpass/ZAcidLowpass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ZAcidLowpass/ZAcidLowpassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ZAcidLowpass() { return new ZAcidLowpass(nullptr); }
}

