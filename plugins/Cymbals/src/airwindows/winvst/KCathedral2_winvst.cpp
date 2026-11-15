// WinVST compilation wrapper for KCathedral2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KCathedral2

#include "../../../../../deps/airwindows/plugins/WinVST/kCathedral2/kCathedral2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/kCathedral2/kCathedral2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KCathedral2() { return new kCathedral2(nullptr); }
}

