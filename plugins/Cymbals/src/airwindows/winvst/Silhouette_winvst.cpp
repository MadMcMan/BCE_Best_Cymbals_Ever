// WinVST compilation wrapper for Silhouette
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Silhouette

#include "../../../../../deps/airwindows/plugins/WinVST/Silhouette/Silhouette.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Silhouette/SilhouetteProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Silhouette() { return new Silhouette(nullptr); }
}

