// WinVST compilation wrapper for ContentHideD
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ContentHideD

#include "../../../../../deps/airwindows/plugins/WinVST/ContentHideD/ContentHideD.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ContentHideD/ContentHideDProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ContentHideD() { return new ContentHideD(nullptr); }
}

