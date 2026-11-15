// WinVST compilation wrapper for TapeHack
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TapeHack

#include "../../../../../deps/airwindows/plugins/WinVST/TapeHack/TapeHack.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TapeHack/TapeHackProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TapeHack() { return new TapeHack(nullptr); }
}

