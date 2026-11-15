// WinVST compilation wrapper for TubeDesk
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TubeDesk

#include "../../../../../deps/airwindows/plugins/WinVST/TubeDesk/TubeDesk.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TubeDesk/TubeDeskProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TubeDesk() { return new TubeDesk(nullptr); }
}

