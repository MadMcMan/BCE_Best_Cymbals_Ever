// WinVST compilation wrapper for KGuitarHall
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_KGuitarHall

#include "../../../../../deps/airwindows/plugins/WinVST/KGuitarHall/KGuitarHall.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/KGuitarHall/KGuitarHallProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_KGuitarHall() { return new kGuitarHall(nullptr); }
}

