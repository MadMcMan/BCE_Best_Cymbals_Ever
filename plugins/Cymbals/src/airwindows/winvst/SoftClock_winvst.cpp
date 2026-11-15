// WinVST compilation wrapper for SoftClock
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_SoftClock

#include "../../../../../deps/airwindows/plugins/WinVST/SoftClock/SoftClock.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/SoftClock/SoftClockProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_SoftClock() { return new SoftClock(nullptr); }
}

