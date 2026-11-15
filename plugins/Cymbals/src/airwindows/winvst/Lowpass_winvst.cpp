// WinVST compilation wrapper for Lowpass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Lowpass

#include "../../../../../deps/airwindows/plugins/WinVST/Lowpass/Lowpass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Lowpass/LowpassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Lowpass() { return new Lowpass(nullptr); }
}

