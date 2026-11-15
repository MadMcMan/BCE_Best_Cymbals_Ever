// WinVST compilation wrapper for AngleFilter
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_AngleFilter

#include "../../../../../deps/airwindows/plugins/WinVST/AngleFilter/AngleFilter.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/AngleFilter/AngleFilterProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_AngleFilter() { return new AngleFilter(nullptr); }
}

