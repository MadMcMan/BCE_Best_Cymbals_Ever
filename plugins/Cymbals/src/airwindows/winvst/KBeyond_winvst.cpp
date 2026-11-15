// WinVST compilation wrapper for KBeyond
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KBeyond

#include "../../../../../deps/airwindows/plugins/WinVST/KBeyond/KBeyond.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/KBeyond/KBeyondProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KBeyond() { return new kBeyond(nullptr); }
}

