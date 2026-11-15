// WinVST compilation wrapper for HardVacuum
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_HardVacuum

#include "../../../../../deps/airwindows/plugins/WinVST/HardVacuum/HardVacuum.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/HardVacuum/HardVacuumProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_HardVacuum() { return new HardVacuum(nullptr); }
}

