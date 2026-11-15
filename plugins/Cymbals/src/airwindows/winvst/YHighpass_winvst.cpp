// WinVST compilation wrapper for YHighpass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_YHighpass

#include "../../../../../deps/airwindows/plugins/WinVST/YHighpass/YHighpass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/YHighpass/YHighpassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_YHighpass() { return new YHighpass(nullptr); }
}

