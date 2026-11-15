// WinVST compilation wrapper for UltrasonicMed
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_UltrasonicMed

#include "../../../../../deps/airwindows/plugins/WinVST/UltrasonicMed/UltrasonicMed.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/UltrasonicMed/UltrasonicMedProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_UltrasonicMed() { return new UltrasonicMed(nullptr); }
}

