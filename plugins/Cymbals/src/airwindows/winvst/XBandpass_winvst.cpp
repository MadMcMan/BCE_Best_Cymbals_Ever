// WinVST compilation wrapper for XBandpass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_XBandpass

#include "../../../../../deps/airwindows/plugins/WinVST/XBandpass/XBandpass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/XBandpass/XBandpassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_XBandpass() { return new XBandpass(nullptr); }
}

