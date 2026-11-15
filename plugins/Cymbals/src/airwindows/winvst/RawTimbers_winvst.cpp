// WinVST compilation wrapper for RawTimbers
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_RawTimbers

#include "../../../../../deps/airwindows/plugins/WinVST/RawTimbers/RawTimbers.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/RawTimbers/RawTimbersProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_RawTimbers() { return new RawTimbers(nullptr); }
}

