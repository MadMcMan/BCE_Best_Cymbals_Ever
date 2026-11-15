// WinVST compilation wrapper for PointyGuitar
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PointyGuitar

#include "../../../../../deps/airwindows/plugins/WinVST/PointyGuitar/PointyGuitar.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PointyGuitar/PointyGuitarProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PointyGuitar() { return new PointyGuitar(nullptr); }
}

