// WinVST compilation wrapper for Desk
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Desk

#include "../../../../../deps/airwindows/plugins/WinVST/Desk/Desk.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Desk/DeskProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Desk() { return new Desk(nullptr); }
}

