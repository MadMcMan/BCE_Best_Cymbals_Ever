// WinVST compilation wrapper for Console8LiteBuss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console8LiteBuss

#include "../../../../../deps/airwindows/plugins/WinVST/Console8LiteBuss/Console8LiteBuss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console8LiteBuss/Console8LiteBussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console8LiteBuss() { return new Console8LiteBuss(nullptr); }
}

