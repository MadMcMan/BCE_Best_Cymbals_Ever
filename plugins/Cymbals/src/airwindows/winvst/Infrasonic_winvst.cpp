// WinVST compilation wrapper for Infrasonic
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Infrasonic

#include "../../../../../deps/airwindows/plugins/WinVST/Infrasonic/Infrasonic.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Infrasonic/InfrasonicProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Infrasonic() { return new Infrasonic(nullptr); }
}

