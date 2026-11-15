// WinVST compilation wrapper for KCathedral
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KCathedral

#include "../../../../../deps/airwindows/plugins/WinVST/kCathedral/kCathedral.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/kCathedral/kCathedralProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KCathedral() { return new kCathedral(nullptr); }
}

