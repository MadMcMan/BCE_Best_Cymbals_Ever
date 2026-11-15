// WinVST compilation wrapper for CreamCoat
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_CreamCoat

#include "../../../../../deps/airwindows/plugins/WinVST/CreamCoat/CreamCoat.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/CreamCoat/CreamCoatProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_CreamCoat() { return new CreamCoat(nullptr); }
}

