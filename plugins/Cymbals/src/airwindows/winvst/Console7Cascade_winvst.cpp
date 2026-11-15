// WinVST compilation wrapper for Console7Cascade
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console7Cascade

#include "../../../../../deps/airwindows/plugins/WinVST/Console7Cascade/Console7Cascade.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console7Cascade/Console7CascadeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console7Cascade() { return new Console7Cascade(nullptr); }
}

