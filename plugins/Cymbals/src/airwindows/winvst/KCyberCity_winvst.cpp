// WinVST compilation wrapper for KCyberCity
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KCyberCity

#include "../../../../../deps/airwindows/plugins/WinVST/KCyberCity/KCyberCity.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/KCyberCity/KCyberCityProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KCyberCity() { return new kCyberCity(nullptr); }
}

