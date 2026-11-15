// WinVST compilation wrapper for ZNotch2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ZNotch2

#include "../../../../../deps/airwindows/plugins/WinVST/ZNotch2/ZNotch2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ZNotch2/ZNotch2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ZNotch2() { return new ZNotch2(nullptr); }
}

