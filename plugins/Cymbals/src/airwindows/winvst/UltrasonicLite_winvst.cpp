// WinVST compilation wrapper for UltrasonicLite
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_UltrasonicLite

#include "../../../../../deps/airwindows/plugins/WinVST/UltrasonicLite/UltrasonicLite.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/UltrasonicLite/UltrasonicLiteProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_UltrasonicLite() { return new UltrasonicLite(nullptr); }
}

