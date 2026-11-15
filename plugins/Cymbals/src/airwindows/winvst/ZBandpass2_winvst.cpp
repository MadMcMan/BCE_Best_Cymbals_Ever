// WinVST compilation wrapper for ZBandpass2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ZBandpass2

#include "../../../../../deps/airwindows/plugins/WinVST/ZBandpass2/ZBandpass2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ZBandpass2/ZBandpass2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ZBandpass2() { return new ZBandpass2(nullptr); }
}

