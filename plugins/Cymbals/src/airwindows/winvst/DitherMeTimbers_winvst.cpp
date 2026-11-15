// WinVST compilation wrapper for DitherMeTimbers
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DitherMeTimbers

#include "../../../../../deps/airwindows/plugins/WinVST/DitherMeTimbers/DitherMeTimbers.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DitherMeTimbers/DitherMeTimbersProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DitherMeTimbers() { return new DitherMeTimbers(nullptr); }
}

