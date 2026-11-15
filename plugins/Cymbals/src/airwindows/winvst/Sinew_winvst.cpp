// WinVST compilation wrapper for Sinew
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Sinew

#include "../../../../../deps/airwindows/plugins/WinVST/Sinew/Sinew.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Sinew/SinewProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Sinew() { return new Sinew(nullptr); }
}

