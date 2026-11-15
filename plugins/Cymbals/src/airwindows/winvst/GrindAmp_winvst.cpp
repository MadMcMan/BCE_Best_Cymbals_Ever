// WinVST compilation wrapper for GrindAmp
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_GrindAmp

#include "../../../../../deps/airwindows/plugins/WinVST/GrindAmp/GrindAmp.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/GrindAmp/GrindAmpProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_GrindAmp() { return new GrindAmp(nullptr); }
}

