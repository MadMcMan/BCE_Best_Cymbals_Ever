// WinVST compilation wrapper for Pyewacket
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Pyewacket

#include "../../../../../deps/airwindows/plugins/WinVST/Pyewacket/Pyewacket.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Pyewacket/PyewacketProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Pyewacket() { return new Pyewacket(nullptr); }
}

