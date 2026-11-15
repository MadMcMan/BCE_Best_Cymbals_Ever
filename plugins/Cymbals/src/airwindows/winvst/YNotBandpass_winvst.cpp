// WinVST compilation wrapper for YNotBandpass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_YNotBandpass

#include "../../../../../deps/airwindows/plugins/WinVST/YNotBandpass/YNotBandpass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/YNotBandpass/YNotBandpassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_YNotBandpass() { return new YNotBandpass(nullptr); }
}

