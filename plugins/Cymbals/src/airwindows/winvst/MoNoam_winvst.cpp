// WinVST compilation wrapper for MoNoam
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_MoNoam

#include "../../../../../deps/airwindows/plugins/WinVST/MoNoam/MoNoam.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/MoNoam/MoNoamProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_MoNoam() { return new MoNoam(nullptr); }
}

