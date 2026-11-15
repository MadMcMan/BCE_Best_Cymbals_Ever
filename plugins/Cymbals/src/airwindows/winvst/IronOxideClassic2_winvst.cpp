// WinVST compilation wrapper for IronOxideClassic2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_IronOxideClassic2

#include "../../../../../deps/airwindows/plugins/WinVST/IronOxideClassic2/IronOxideClassic2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/IronOxideClassic2/IronOxideClassic2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_IronOxideClassic2() { return new IronOxideClassic2(nullptr); }
}

