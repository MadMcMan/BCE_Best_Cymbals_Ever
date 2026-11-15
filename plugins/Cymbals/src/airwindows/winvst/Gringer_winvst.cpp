// WinVST compilation wrapper for Gringer
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Gringer

#include "../../../../../deps/airwindows/plugins/WinVST/Gringer/Gringer.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Gringer/GringerProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Gringer() { return new Gringer(nullptr); }
}

