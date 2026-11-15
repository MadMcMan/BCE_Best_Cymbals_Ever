// WinVST compilation wrapper for BlockParty
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BlockParty

#include "../../../../../deps/airwindows/plugins/WinVST/BlockParty/BlockParty.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BlockParty/BlockPartyProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BlockParty() { return new BlockParty(nullptr); }
}

