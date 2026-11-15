// WinVST compilation wrapper for PurestConsole2Buss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestConsole2Buss

#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsole2Buss/PurestConsole2Buss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestConsole2Buss/PurestConsole2BussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestConsole2Buss() { return new PurestConsole2Buss(nullptr); }
}

