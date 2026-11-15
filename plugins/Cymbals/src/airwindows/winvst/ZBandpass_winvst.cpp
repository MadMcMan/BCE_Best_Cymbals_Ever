// WinVST compilation wrapper for ZBandpass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ZBandpass

#include "../../../../../deps/airwindows/plugins/WinVST/ZBandpass/ZBandpass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ZBandpass/ZBandpassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ZBandpass() { return new ZBandpass(nullptr); }
}

