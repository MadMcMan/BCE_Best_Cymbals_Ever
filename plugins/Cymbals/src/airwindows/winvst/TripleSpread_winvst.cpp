// WinVST compilation wrapper for TripleSpread
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TripleSpread

#include "../../../../../deps/airwindows/plugins/WinVST/TripleSpread/TripleSpread.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TripleSpread/TripleSpreadProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TripleSpread() { return new TripleSpread(nullptr); }
}

