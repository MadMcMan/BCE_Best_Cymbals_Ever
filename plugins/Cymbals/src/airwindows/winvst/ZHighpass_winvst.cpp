// WinVST compilation wrapper for ZHighpass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ZHighpass

#include "../../../../../deps/airwindows/plugins/WinVST/ZHighpass/ZHighpass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ZHighpass/ZHighpassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ZHighpass() { return new ZHighpass(nullptr); }
}

