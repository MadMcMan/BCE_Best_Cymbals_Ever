// WinVST compilation wrapper for IronOxideClassic
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_IronOxideClassic

#include "../../../../../deps/airwindows/plugins/WinVST/IronOxideClassic/IronOxideClassic.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/IronOxideClassic/IronOxideClassicProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_IronOxideClassic() { return new IronOxideClassic(nullptr); }
}

