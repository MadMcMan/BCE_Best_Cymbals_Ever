// WinVST compilation wrapper for Tape
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Tape

#include "../../../../../deps/airwindows/plugins/WinVST/Tape/Tape.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Tape/TapeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Tape() { return new Tape(nullptr); }
}

