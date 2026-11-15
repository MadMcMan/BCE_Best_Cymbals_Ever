// WinVST compilation wrapper for StoneFireComp
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_StoneFireComp

#include "../../../../../deps/airwindows/plugins/WinVST/StoneFireComp/StoneFireComp.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/StoneFireComp/StoneFireCompProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_StoneFireComp() { return new StoneFireComp(nullptr); }
}

