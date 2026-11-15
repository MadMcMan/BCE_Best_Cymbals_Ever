// WinVST compilation wrapper for Chorus
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Chorus

#include "../../../../../deps/airwindows/plugins/WinVST/Chorus/Chorus.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Chorus/ChorusProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Chorus() { return new Chorus(nullptr); }
}

