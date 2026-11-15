// WinVST compilation wrapper for TremoSquare
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TremoSquare

#include "../../../../../deps/airwindows/plugins/WinVST/TremoSquare/TremoSquare.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TremoSquare/TremoSquareProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TremoSquare() { return new TremoSquare(nullptr); }
}

