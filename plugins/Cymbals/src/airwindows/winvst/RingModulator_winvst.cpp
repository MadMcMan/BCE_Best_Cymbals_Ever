// WinVST compilation wrapper for RingModulator
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_RingModulator

#include "../../../../../deps/airwindows/plugins/WinVST/RingModulator/RingModulator.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/RingModulator/RingModulatorProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_RingModulator() { return new RingModulator(nullptr); }
}

