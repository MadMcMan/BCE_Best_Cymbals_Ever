// WinVST compilation wrapper for Flutter
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Flutter

#include "../../../../../deps/airwindows/plugins/WinVST/Flutter/Flutter.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Flutter/FlutterProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Flutter() { return new Flutter(nullptr); }
}

