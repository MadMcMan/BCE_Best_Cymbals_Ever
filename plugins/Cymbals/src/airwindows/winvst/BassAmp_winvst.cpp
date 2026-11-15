// WinVST compilation wrapper for BassAmp
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BassAmp

#include "../../../../../deps/airwindows/plugins/WinVST/BassAmp/BassAmp.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BassAmp/BassAmpProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BassAmp() { return new BassAmp(nullptr); }
}

