// WinVST compilation wrapper for Biquad2
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Biquad2

#include "../../../../../deps/airwindows/plugins/WinVST/Biquad2/Biquad2.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Biquad2/Biquad2Proc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Biquad2() { return new Biquad2(nullptr); }
}

