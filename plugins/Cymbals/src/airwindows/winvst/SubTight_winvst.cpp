// WinVST compilation wrapper for SubTight
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_SubTight

#include "../../../../../deps/airwindows/plugins/WinVST/SubTight/SubTight.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/SubTight/SubTightProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_SubTight() { return new SubTight(nullptr); }
}

