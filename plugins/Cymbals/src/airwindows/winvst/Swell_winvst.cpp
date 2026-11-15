// WinVST compilation wrapper for Swell
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Swell

#include "../../../../../deps/airwindows/plugins/WinVST/Swell/Swell.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Swell/SwellProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Swell() { return new Swell(nullptr); }
}

