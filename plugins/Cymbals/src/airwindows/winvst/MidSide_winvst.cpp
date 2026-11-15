// WinVST compilation wrapper for MidSide
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_MidSide

#include "../../../../../deps/airwindows/plugins/WinVST/MidSide/MidSide.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/MidSide/MidSideProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_MidSide() { return new MidSide(nullptr); }
}

