// WinVST compilation wrapper for Huge
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Huge

#include "../../../../../deps/airwindows/plugins/WinVST/Huge/Huge.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Huge/HugeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Huge() { return new Huge(nullptr); }
}

