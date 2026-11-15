// WinVST compilation wrapper for Hype
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Hype

#include "../../../../../deps/airwindows/plugins/WinVST/Hype/Hype.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Hype/HypeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Hype() { return new Hype(nullptr); }
}

