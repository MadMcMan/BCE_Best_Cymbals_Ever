// WinVST compilation wrapper for CrunchyGrooveWear
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_CrunchyGrooveWear

#include "../../../../../deps/airwindows/plugins/WinVST/CrunchyGrooveWear/CrunchyGrooveWear.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/CrunchyGrooveWear/CrunchyGrooveWearProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_CrunchyGrooveWear() { return new CrunchyGrooveWear(nullptr); }
}

