// WinVST compilation wrapper for Sweeten
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Sweeten

#include "../../../../../deps/airwindows/plugins/WinVST/Sweeten/Sweeten.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Sweeten/SweetenProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Sweeten() { return new Sweeten(nullptr); }
}

