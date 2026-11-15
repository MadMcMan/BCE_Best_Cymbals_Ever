// WinVST compilation wrapper for KCathedral4
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KCathedral4

#include "../../../../../deps/airwindows/plugins/WinVST/KCathedral4/KCathedral4.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/KCathedral4/KCathedral4Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KCathedral4() { return new kCathedral4(nullptr); }
}

