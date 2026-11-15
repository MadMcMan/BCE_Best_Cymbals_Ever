// WinVST compilation wrapper for PhaseNudge
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PhaseNudge

#include "../../../../../deps/airwindows/plugins/WinVST/PhaseNudge/PhaseNudge.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PhaseNudge/PhaseNudgeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PhaseNudge() { return new PhaseNudge(nullptr); }
}

