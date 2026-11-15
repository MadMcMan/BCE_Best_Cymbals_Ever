// WinVST compilation wrapper for AtmosphereBuss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_AtmosphereBuss

#include "../../../../../deps/airwindows/plugins/WinVST/AtmosphereBuss/AtmosphereBuss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/AtmosphereBuss/AtmosphereBussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_AtmosphereBuss() { return new AtmosphereBuss(nullptr); }
}

