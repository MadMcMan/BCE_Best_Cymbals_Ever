// WinVST compilation wrapper for Tremolo
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Tremolo

#include "../../../../../deps/airwindows/plugins/WinVST/Tremolo/Tremolo.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Tremolo/TremoloProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Tremolo() { return new Tremolo(nullptr); }
}

