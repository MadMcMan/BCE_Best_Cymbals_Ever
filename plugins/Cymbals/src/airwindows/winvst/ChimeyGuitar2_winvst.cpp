// WinVST compilation wrapper for ChimeyGuitar2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ChimeyGuitar2

#include "../../../../../deps/airwindows/plugins/WinVST/ChimeyGuitar2/ChimeyGuitar2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ChimeyGuitar2/ChimeyGuitar2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ChimeyGuitar2() { return new ChimeyGuitar2(nullptr); }
}

