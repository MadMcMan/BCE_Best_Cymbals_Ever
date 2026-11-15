// WinVST compilation wrapper for KWoodRoom
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KWoodRoom

#include "../../../../../deps/airwindows/plugins/WinVST/kWoodRoom/kWoodRoom.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/kWoodRoom/kWoodRoomProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KWoodRoom() { return new kWoodRoom(nullptr); }
}

