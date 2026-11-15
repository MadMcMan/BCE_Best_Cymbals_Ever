// WinVST compilation wrapper for ChimeyGuitar
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ChimeyGuitar

#include "../../../../../deps/airwindows/plugins/WinVST/ChimeyGuitar/ChimeyGuitar.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ChimeyGuitar/ChimeyGuitarProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ChimeyGuitar() { return new ChimeyGuitar(nullptr); }
}

