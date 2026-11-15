// WinVST compilation wrapper for VoiceTrick
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_VoiceTrick

#include "../../../../../deps/airwindows/plugins/WinVST/VoiceTrick/VoiceTrick.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/VoiceTrick/VoiceTrickProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_VoiceTrick() { return new VoiceTrick(nullptr); }
}

