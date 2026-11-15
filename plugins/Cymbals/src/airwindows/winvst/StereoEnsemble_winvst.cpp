// WinVST compilation wrapper for StereoEnsemble
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_StereoEnsemble

#include "../../../../../deps/airwindows/plugins/WinVST/StereoEnsemble/StereoEnsemble.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/StereoEnsemble/StereoEnsembleProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_StereoEnsemble() { return new StereoEnsemble(nullptr); }
}

