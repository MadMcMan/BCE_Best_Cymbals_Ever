// WinVST compilation wrapper for KCathedral3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KCathedral3

#include "../../../../../deps/airwindows/plugins/WinVST/KCathedral3/KCathedral3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/KCathedral3/KCathedral3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KCathedral3() { return new kCathedral3(nullptr); }
}

