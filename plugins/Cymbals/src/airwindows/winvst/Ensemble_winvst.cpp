// WinVST compilation wrapper for Ensemble
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Ensemble

#include "../../../../../deps/airwindows/plugins/WinVST/Ensemble/Ensemble.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Ensemble/EnsembleProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Ensemble() { return new Ensemble(nullptr); }
}

