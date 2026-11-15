// WinVST compilation wrapper for Chamber
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Chamber

#include "../../../../../deps/airwindows/plugins/WinVST/Chamber/Chamber.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Chamber/ChamberProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Chamber() { return new Chamber(nullptr); }
}

