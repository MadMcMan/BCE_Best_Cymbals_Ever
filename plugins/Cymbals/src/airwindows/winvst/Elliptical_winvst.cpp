// WinVST compilation wrapper for Elliptical
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Elliptical

#include "../../../../../deps/airwindows/plugins/WinVST/Elliptical/Elliptical.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Elliptical/EllipticalProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Elliptical() { return new Elliptical(nullptr); }
}

