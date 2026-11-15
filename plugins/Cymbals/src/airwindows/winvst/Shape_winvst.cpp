// WinVST compilation wrapper for Shape
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Shape

#include "../../../../../deps/airwindows/plugins/WinVST/Shape/Shape.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Shape/ShapeProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Shape() { return new Shape(nullptr); }
}

