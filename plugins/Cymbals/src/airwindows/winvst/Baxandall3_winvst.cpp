// WinVST compilation wrapper for Baxandall3
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Baxandall3

#include "../../../../../deps/airwindows/plugins/WinVST/Baxandall3/Baxandall3.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Baxandall3/Baxandall3Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Baxandall3() { return new Baxandall3(nullptr); }
}

