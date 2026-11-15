// WinVST compilation wrapper for KRockstar
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KRockstar

#include "../../../../../deps/airwindows/plugins/WinVST/KRockstar/KRockstar.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/KRockstar/KRockstarProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KRockstar() { return new kRockstar(nullptr); }
}

