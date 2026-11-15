// WinVST compilation wrapper for VariMu
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_VariMu

#include "../../../../../deps/airwindows/plugins/WinVST/VariMu/VariMu.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/VariMu/VariMuProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_VariMu() { return new VariMu(nullptr); }
}

