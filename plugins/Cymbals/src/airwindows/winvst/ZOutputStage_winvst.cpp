// WinVST compilation wrapper for ZOutputStage
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ZOutputStage

#include "../../../../../deps/airwindows/plugins/WinVST/ZOutputStage/ZOutputStage.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ZOutputStage/ZOutputStageProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ZOutputStage() { return new ZOutputStage(nullptr); }
}

