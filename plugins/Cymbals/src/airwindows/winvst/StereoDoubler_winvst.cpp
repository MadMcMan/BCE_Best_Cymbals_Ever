// WinVST compilation wrapper for StereoDoubler
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_StereoDoubler

#include "../../../../../deps/airwindows/plugins/WinVST/StereoDoubler/StereoDoubler.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/StereoDoubler/StereoDoublerProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_StereoDoubler() { return new StereoDoubler(nullptr); }
}

