// WinVST compilation wrapper for Energy
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Energy

#include "../../../../../deps/airwindows/plugins/WinVST/Energy/Energy.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Energy/EnergyProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Energy() { return new Energy(nullptr); }
}

