// WinVST compilation wrapper for Hull2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Hull2

#include "../../../../../deps/airwindows/plugins/WinVST/Hull2/Hull2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Hull2/Hull2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Hull2() { return new Hull2(nullptr); }
}

