// WinVST compilation wrapper for BassDrive
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BassDrive

#include "../../../../../deps/airwindows/plugins/WinVST/BassDrive/BassDrive.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BassDrive/BassDriveProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BassDrive() { return new BassDrive(nullptr); }
}

