// WinVST compilation wrapper for LilAmp
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_LilAmp

#include "../../../../../deps/airwindows/plugins/WinVST/LilAmp/LilAmp.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/LilAmp/LilAmpProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_LilAmp() { return new LilAmp(nullptr); }
}

