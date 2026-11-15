// WinVST compilation wrapper for Overheads
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Overheads

#include "../../../../../deps/airwindows/plugins/WinVST/Overheads/Overheads.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Overheads/OverheadsProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Overheads() { return new Overheads(nullptr); }
}

