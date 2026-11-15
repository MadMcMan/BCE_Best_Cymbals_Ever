// WinVST compilation wrapper for NCSeventeen
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_NCSeventeen

#include "../../../../../deps/airwindows/plugins/WinVST/NCSeventeen/NCSeventeen.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/NCSeventeen/NCSeventeenProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_NCSeventeen() { return new NCSeventeen(nullptr); }
}

