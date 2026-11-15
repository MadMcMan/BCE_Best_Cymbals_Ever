// WinVST compilation wrapper for ChorusEnsemble
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ChorusEnsemble

#include "../../../../../deps/airwindows/plugins/WinVST/ChorusEnsemble/ChorusEnsemble.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ChorusEnsemble/ChorusEnsembleProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ChorusEnsemble() { return new ChorusEnsemble(nullptr); }
}

