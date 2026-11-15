// WinVST compilation wrapper for Beam
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Beam

#include "../../../../../deps/airwindows/plugins/WinVST/Beam/Beam.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Beam/BeamProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Beam() { return new Beam(nullptr); }
}

