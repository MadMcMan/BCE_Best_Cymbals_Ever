// WinVST compilation wrapper for Console7Crunch
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Console7Crunch

#include "../../../../../deps/airwindows/plugins/WinVST/Console7Crunch/Console7Crunch.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Console7Crunch/Console7CrunchProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Console7Crunch() { return new Console7Crunch(nullptr); }
}

