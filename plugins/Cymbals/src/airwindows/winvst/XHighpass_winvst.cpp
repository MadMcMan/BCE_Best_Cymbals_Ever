// WinVST compilation wrapper for XHighpass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_XHighpass

#include "../../../../../deps/airwindows/plugins/WinVST/XHighpass/XHighpass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/XHighpass/XHighpassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_XHighpass() { return new XHighpass(nullptr); }
}

