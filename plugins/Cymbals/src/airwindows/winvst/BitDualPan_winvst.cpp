// WinVST compilation wrapper for BitDualPan
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BitDualPan

#include "../../../../../deps/airwindows/plugins/WinVST/BitDualPan/BitDualPan.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BitDualPan/BitDualPanProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BitDualPan() { return new BitDualPan(nullptr); }
}

