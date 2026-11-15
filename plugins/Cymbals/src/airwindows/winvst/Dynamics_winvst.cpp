// WinVST compilation wrapper for Dynamics
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Dynamics

#include "../../../../../deps/airwindows/plugins/WinVST/Dynamics/Dynamics.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Dynamics/DynamicsProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Dynamics() { return new Dynamics(nullptr); }
}

