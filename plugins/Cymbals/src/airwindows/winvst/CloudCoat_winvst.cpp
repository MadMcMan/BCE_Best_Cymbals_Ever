// WinVST compilation wrapper for CloudCoat
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_CloudCoat

#include "../../../../../deps/airwindows/plugins/WinVST/CloudCoat/CloudCoat.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/CloudCoat/CloudCoatProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_CloudCoat() { return new CloudCoat(nullptr); }
}

