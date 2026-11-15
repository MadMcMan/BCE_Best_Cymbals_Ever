// WinVST compilation wrapper for Creature
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Creature

#include "../../../../../deps/airwindows/plugins/WinVST/Creature/Creature.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Creature/CreatureProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Creature() { return new Creature(nullptr); }
}

