// WinVST compilation wrapper for Deckwrecka
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Deckwrecka

#include "../../../../../deps/airwindows/plugins/WinVST/Deckwrecka/Deckwrecka.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Deckwrecka/DeckwreckaProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Deckwrecka() { return new Deckwrecka(nullptr); }
}

