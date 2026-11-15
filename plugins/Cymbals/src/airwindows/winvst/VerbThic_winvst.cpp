// WinVST compilation wrapper for VerbThic
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_VerbThic

#include "../../../../../deps/airwindows/plugins/WinVST/VerbThic/VerbThic.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/VerbThic/VerbThicProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_VerbThic() { return new VerbThic(nullptr); }
}

