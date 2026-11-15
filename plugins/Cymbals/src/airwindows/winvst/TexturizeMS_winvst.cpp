// WinVST compilation wrapper for TexturizeMS
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_TexturizeMS

#include "../../../../../deps/airwindows/plugins/WinVST/TexturizeMS/TexturizeMS.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/TexturizeMS/TexturizeMSProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_TexturizeMS() { return new TexturizeMS(nullptr); }
}

