// WinVST compilation wrapper for KAlienSpaceship
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KAlienSpaceship

#include "../../../../../deps/airwindows/plugins/WinVST/KAlienSpaceship/KAlienSpaceship.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/KAlienSpaceship/KAlienSpaceshipProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KAlienSpaceship() { return new kAlienSpaceship(nullptr); }
}

