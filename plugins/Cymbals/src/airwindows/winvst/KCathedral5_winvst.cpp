// WinVST compilation wrapper for KCathedral5
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KCathedral5

#include "../../../../../deps/airwindows/plugins/WinVST/KCathedral5/KCathedral5.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/KCathedral5/KCathedral5Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KCathedral5() { return new kCathedral5(nullptr); }
}

