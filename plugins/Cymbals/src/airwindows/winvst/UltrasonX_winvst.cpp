// WinVST compilation wrapper for UltrasonX
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_UltrasonX

#include "../../../../../deps/airwindows/plugins/WinVST/UltrasonX/UltrasonX.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/UltrasonX/UltrasonXProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_UltrasonX() { return new UltrasonX(nullptr); }
}

