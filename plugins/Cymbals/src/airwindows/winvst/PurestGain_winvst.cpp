// WinVST compilation wrapper for PurestGain
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestGain

#include "../../../../../deps/airwindows/plugins/WinVST/PurestGain/PurestGain.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestGain/PurestGainProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestGain() { return new PurestGain(nullptr); }
}

