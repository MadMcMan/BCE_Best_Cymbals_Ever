// WinVST compilation wrapper for LRFlipTimer
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_LRFlipTimer

#include "../../../../../deps/airwindows/plugins/WinVST/LRFlipTimer/LRFlipTimer.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/LRFlipTimer/LRFlipTimerProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_LRFlipTimer() { return new LRFlipTimer(nullptr); }
}

