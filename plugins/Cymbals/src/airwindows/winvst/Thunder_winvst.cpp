// WinVST compilation wrapper for Thunder
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Thunder

#include "../../../../../deps/airwindows/plugins/WinVST/Thunder/Thunder.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Thunder/ThunderProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Thunder() { return new Thunder(nullptr); }
}

