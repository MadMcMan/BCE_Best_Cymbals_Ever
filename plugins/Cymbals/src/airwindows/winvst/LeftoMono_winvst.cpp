// WinVST compilation wrapper for LeftoMono
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_LeftoMono

#include "../../../../../deps/airwindows/plugins/WinVST/LeftoMono/LeftoMono.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/LeftoMono/LeftoMonoProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_LeftoMono() { return new LeftoMono(nullptr); }
}

