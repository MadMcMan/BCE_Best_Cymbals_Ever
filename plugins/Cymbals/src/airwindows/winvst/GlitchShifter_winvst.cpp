// WinVST compilation wrapper for GlitchShifter
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_GlitchShifter

#include "../../../../../deps/airwindows/plugins/WinVST/GlitchShifter/GlitchShifter.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/GlitchShifter/GlitchShifterProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_GlitchShifter() { return new GlitchShifter(nullptr); }
}

