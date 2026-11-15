// WinVST compilation wrapper for BigAmp
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BigAmp

#include "../../../../../deps/airwindows/plugins/WinVST/BigAmp/BigAmp.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BigAmp/BigAmpProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BigAmp() { return new BigAmp(nullptr); }
}

