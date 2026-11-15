// WinVST compilation wrapper for ChromeOxide
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_ChromeOxide

#include "../../../../../deps/airwindows/plugins/WinVST/ChromeOxide/ChromeOxide.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/ChromeOxide/ChromeOxideProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_ChromeOxide() { return new ChromeOxide(nullptr); }
}

