// WinVST compilation wrapper for ChimeyDeluxe
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ChimeyDeluxe

#include "../../../../../deps/airwindows/plugins/WinVST/ChimeyDeluxe/ChimeyDeluxe.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ChimeyDeluxe/ChimeyDeluxeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ChimeyDeluxe() { return new ChimeyDeluxe(nullptr); }
}

