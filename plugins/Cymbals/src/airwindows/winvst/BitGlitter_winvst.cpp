// WinVST compilation wrapper for BitGlitter
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BitGlitter

#include "../../../../../deps/airwindows/plugins/WinVST/BitGlitter/BitGlitter.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BitGlitter/BitGlitterProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BitGlitter() { return new BitGlitter(nullptr); }
}

