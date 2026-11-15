// WinVST compilation wrapper for CompresaTurator
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_CompresaTurator

#include "../../../../../deps/airwindows/plugins/WinVST/CompresaTurator/CompresaTurator.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/CompresaTurator/CompresaTuratorProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_CompresaTurator() { return new Compresaturator(nullptr); }
}

