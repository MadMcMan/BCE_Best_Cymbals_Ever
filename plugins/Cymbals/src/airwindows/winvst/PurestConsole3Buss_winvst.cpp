// WinVST compilation wrapper for PurestConsole3Buss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestConsole3Buss

#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsole3Buss/PurestConsole3Buss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsole3Buss/PurestConsole3BussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestConsole3Buss() { return new PurestConsole3Buss(nullptr); }
}

