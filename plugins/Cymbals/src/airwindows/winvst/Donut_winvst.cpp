// WinVST compilation wrapper for Donut
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Donut

#include "../../../../../deps/airwindows/plugins/WinVST/Donut/Donut.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Donut/DonutProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Donut() { return new Donut(nullptr); }
}

