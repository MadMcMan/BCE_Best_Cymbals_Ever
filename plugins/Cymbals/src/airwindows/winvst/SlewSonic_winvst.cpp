// WinVST compilation wrapper for SlewSonic
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_SlewSonic

#include "../../../../../deps/airwindows/plugins/WinVST/SlewSonic/SlewSonic.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/SlewSonic/SlewSonicProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_SlewSonic() { return new SlewSonic(nullptr); }
}

