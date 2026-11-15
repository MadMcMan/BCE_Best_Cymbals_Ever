// WinVST compilation wrapper for RetroBass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_RetroBass

#include "../../../../../deps/airwindows/plugins/WinVST/RetroBass/RetroBass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/RetroBass/RetroBassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_RetroBass() { return new RetroBass(nullptr); }
}

