// WinVST compilation wrapper for Mojo
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Mojo

#include "../../../../../deps/airwindows/plugins/WinVST/Mojo/Mojo.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Mojo/MojoProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Mojo() { return new Mojo(nullptr); }
}

