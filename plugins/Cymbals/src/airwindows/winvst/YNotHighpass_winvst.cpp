// WinVST compilation wrapper for YNotHighpass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_YNotHighpass

#include "../../../../../deps/airwindows/plugins/WinVST/YNotHighpass/YNotHighpass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/YNotHighpass/YNotHighpassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_YNotHighpass() { return new YNotHighpass(nullptr); }
}

