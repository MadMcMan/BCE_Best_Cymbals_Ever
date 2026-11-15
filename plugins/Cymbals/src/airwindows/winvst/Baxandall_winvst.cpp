// WinVST compilation wrapper for Baxandall
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Baxandall

#include "../../../../../deps/airwindows/plugins/WinVST/Baxandall/Baxandall.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Baxandall/BaxandallProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Baxandall() { return new Baxandall(nullptr); }
}

