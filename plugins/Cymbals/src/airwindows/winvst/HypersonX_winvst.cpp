// WinVST compilation wrapper for HypersonX
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_HypersonX

#include "../../../../../deps/airwindows/plugins/WinVST/HypersonX/HypersonX.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/HypersonX/HypersonXProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_HypersonX() { return new HypersonX(nullptr); }
}

