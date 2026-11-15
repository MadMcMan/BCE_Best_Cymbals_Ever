// WinVST compilation wrapper for NodeDither
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_NodeDither

#include "../../../../../deps/airwindows/plugins/WinVST/NodeDither/NodeDither.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/NodeDither/NodeDitherProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_NodeDither() { return new NodeDither(nullptr); }
}

