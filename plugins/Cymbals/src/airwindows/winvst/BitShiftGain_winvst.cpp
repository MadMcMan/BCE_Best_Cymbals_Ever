// WinVST compilation wrapper for BitShiftGain
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BitShiftGain

#include "../../../../../deps/airwindows/plugins/WinVST/BitShiftGain/BitShiftGain.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BitShiftGain/BitShiftGainProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BitShiftGain() { return new BitShiftGain(nullptr); }
}

