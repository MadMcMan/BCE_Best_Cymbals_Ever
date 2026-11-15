// WinVST compilation wrapper for ElectroHat
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ElectroHat

#include "../../../../../deps/airwindows/plugins/WinVST/ElectroHat/ElectroHat.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ElectroHat/ElectroHatProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ElectroHat() { return new ElectroHat(nullptr); }
}

