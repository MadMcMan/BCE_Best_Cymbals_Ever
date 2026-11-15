// WinVST compilation wrapper for MidAmp
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_MidAmp

#include "../../../../../deps/airwindows/plugins/WinVST/MidAmp/MidAmp.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/MidAmp/MidAmpProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_MidAmp() { return new MidAmp(nullptr); }
}

