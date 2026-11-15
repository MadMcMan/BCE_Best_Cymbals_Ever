// WinVST compilation wrapper for PurestEcho
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestEcho

#include "../../../../../deps/airwindows/plugins/WinVST/PurestEcho/PurestEcho.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestEcho/PurestEchoProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestEcho() { return new PurestEcho(nullptr); }
}

