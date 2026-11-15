// WinVST compilation wrapper for PodcastDeluxe
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_PodcastDeluxe

#include "../../../../../deps/airwindows/plugins/WinVST/PodcastDeluxe/PodcastDeluxe.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/PodcastDeluxe/PodcastDeluxeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_PodcastDeluxe() { return new PodcastDeluxe(nullptr); }
}

