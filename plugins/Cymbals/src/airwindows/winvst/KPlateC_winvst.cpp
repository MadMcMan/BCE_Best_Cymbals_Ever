// WinVST compilation wrapper for KPlateC
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KPlateC

#include "../../../../../deps/airwindows/plugins/WinVST/KPlateC/KPlateC.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/KPlateC/KPlateCProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KPlateC() { return new kPlateC(nullptr); }
}

