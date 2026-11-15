// WinVST compilation wrapper for Console8BussHype
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console8BussHype

#include "../../../../../deps/airwindows/plugins/WinVST/Console8BussHype/Console8BussHype.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console8BussHype/Console8BussHypeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console8BussHype() { return new Console8BussHype(nullptr); }
}

