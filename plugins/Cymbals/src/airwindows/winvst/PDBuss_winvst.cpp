// WinVST compilation wrapper for PDBuss
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PDBuss

#include "../../../../../deps/airwindows/plugins/WinVST/PDBuss/PDBuss.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PDBuss/PDBussProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PDBuss() { return new PDBuss(nullptr); }
}

