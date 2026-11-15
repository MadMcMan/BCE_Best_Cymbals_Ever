// WinVST compilation wrapper for uLawDecode
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_uLawDecode

#include "../../../../../deps/airwindows/plugins/WinVST/uLawDecode/uLawDecode.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/uLawDecode/uLawDecodeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_uLawDecode() { return new uLawDecode(nullptr); }
}

