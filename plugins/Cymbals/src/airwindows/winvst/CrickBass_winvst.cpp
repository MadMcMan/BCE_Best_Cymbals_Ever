// WinVST compilation wrapper for CrickBass
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_CrickBass

#include "../../../../../deps/airwindows/plugins/WinVST/CrickBass/CrickBass.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/CrickBass/CrickBassProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_CrickBass() { return new CrickBass(nullptr); }
}

