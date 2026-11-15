// WinVST compilation wrapper for PurestDrive
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PurestDrive

#include "../../../../../deps/airwindows/plugins/WinVST/PurestDrive/PurestDrive.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PurestDrive/PurestDriveProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PurestDrive() { return new PurestDrive(nullptr); }
}

