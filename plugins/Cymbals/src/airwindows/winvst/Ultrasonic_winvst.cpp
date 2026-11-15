// WinVST compilation wrapper for Ultrasonic
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Ultrasonic

#include "../../../../../deps/airwindows/plugins/WinVST/Ultrasonic/Ultrasonic.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Ultrasonic/UltrasonicProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Ultrasonic() { return new Ultrasonic(nullptr); }
}

