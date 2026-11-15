// WinVST compilation wrapper for BeziComp
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BeziComp

#include "../../../../../deps/airwindows/plugins/WinVST/BeziComp/BeziComp.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BeziComp/BeziCompProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BeziComp() { return new BeziComp(nullptr); }
}

