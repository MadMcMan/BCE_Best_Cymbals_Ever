// WinVST compilation wrapper for NonlinearSpace
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_NonlinearSpace

#include "../../../../../deps/airwindows/plugins/WinVST/NonlinearSpace/NonlinearSpace.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/NonlinearSpace/NonlinearSpaceProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_NonlinearSpace() { return new NonlinearSpace(nullptr); }
}

