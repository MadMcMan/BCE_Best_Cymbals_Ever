// WinVST compilation wrapper for TransDesk
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TransDesk

#include "../../../../../deps/airwindows/plugins/WinVST/TransDesk/TransDesk.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TransDesk/TransDeskProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TransDesk() { return new TransDesk(nullptr); }
}

