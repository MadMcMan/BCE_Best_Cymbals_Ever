// WinVST compilation wrapper for DustBunny
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_DustBunny

#include "../../../../../deps/airwindows/plugins/WinVST/DustBunny/DustBunny.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/DustBunny/DustBunnyProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_DustBunny() { return new DustBunny(nullptr); }
}

