// WinVST compilation wrapper for RightoMono
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_RightoMono

#include "../../../../../deps/airwindows/plugins/WinVST/RightoMono/RightoMono.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/RightoMono/RightoMonoProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_RightoMono() { return new RightoMono(nullptr); }
}

