// WinVST compilation wrapper for MSFlipTimer
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_MSFlipTimer

#include "../../../../../deps/airwindows/plugins/WinVST/MSFlipTimer/MSFlipTimer.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/MSFlipTimer/MSFlipTimerProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_MSFlipTimer() { return new MSFlipTimer(nullptr); }
}

