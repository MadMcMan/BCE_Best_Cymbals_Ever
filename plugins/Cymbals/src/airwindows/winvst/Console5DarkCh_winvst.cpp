// WinVST compilation wrapper for Console5DarkCh
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console5DarkCh

#include "../../../../../deps/airwindows/plugins/WinVST/Console5DarkCh/Console5DarkCh.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console5DarkCh/Console5DarkChProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console5DarkCh() { return new Console5DarkCh(nullptr); }
}

