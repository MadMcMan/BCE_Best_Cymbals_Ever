// WinVST compilation wrapper for FathomFive
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_FathomFive

#include "../../../../../deps/airwindows/plugins/WinVST/FathomFive/FathomFive.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/FathomFive/FathomFiveProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_FathomFive() { return new FathomFive(nullptr); }
}

