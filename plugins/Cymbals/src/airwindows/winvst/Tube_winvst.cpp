// WinVST compilation wrapper for Tube
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Tube

#include "../../../../../deps/airwindows/plugins/WinVST/Tube/Tube.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Tube/TubeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Tube() { return new Tube(nullptr); }
}

