// WinVST compilation wrapper for NotJustAnotherDither
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_NotJustAnotherDither

#include "../../../../../deps/airwindows/plugins/WinVST/NotJustAnotherDither/NotJustAnotherDither.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/NotJustAnotherDither/NotJustAnotherDitherProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_NotJustAnotherDither() { return new NotJustAnotherDither(nullptr); }
}

