// WinVST compilation wrapper for Density
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Density

#include "../../../../../deps/airwindows/plugins/WinVST/Density/Density.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Density/DensityProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Density() { return new Density(nullptr); }
}

