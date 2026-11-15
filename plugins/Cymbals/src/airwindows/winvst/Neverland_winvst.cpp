// WinVST compilation wrapper for Neverland
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Neverland

#include "../../../../../deps/airwindows/plugins/WinVST/Neverland/Neverland.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Neverland/NeverlandProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Neverland() { return new Neverland(nullptr); }
}

