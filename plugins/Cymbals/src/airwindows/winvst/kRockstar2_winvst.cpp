// WinVST compilation wrapper for kRockstar2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_kRockstar2

#include "../../../../../deps/airwindows/plugins/WinVST/kRockstar2/kRockstar2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/kRockstar2/kRockstar2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_kRockstar2() { return new kRockstar2(nullptr); }
}

