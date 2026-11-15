// WinVST compilation wrapper for Galactic
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Galactic

#include "../../../../../deps/airwindows/plugins/WinVST/Galactic/Galactic.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Galactic/GalacticProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Galactic() { return new Galactic(nullptr); }
}

