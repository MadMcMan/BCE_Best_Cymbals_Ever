// WinVST compilation wrapper for CrunchCoat
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_CrunchCoat

#include "../../../../../deps/airwindows/plugins/WinVST/CrunchCoat/CrunchCoat.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/CrunchCoat/CrunchCoatProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_CrunchCoat() { return new CrunchCoat(nullptr); }
}

