// WinVST compilation wrapper for Calibre
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Calibre

#include "../../../../../deps/airwindows/plugins/WinVST/Calibre/Calibre.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Calibre/CalibreProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Calibre() { return new Calibre(nullptr); }
}

