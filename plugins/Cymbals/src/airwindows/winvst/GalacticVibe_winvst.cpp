// WinVST compilation wrapper for GalacticVibe
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_GalacticVibe

#include "../../../../../deps/airwindows/plugins/WinVST/GalacticVibe/GalacticVibe.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/GalacticVibe/GalacticVibeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_GalacticVibe() { return new GalacticVibe(nullptr); }
}

