// WinVST compilation wrapper for FromTape
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_FromTape

#include "../../../../../deps/airwindows/plugins/WinVST/FromTape/FromTape.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/FromTape/FromTapeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_FromTape() { return new FromTape(nullptr); }
}

