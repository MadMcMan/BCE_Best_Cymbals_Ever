// WinVST compilation wrapper for BitShiftPan
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BitShiftPan

#include "../../../../../deps/airwindows/plugins/WinVST/BitShiftPan/BitShiftPan.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BitShiftPan/BitShiftPanProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BitShiftPan() { return new BitShiftPan(nullptr); }
}

