// WinVST compilation wrapper for Wolfbot
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Wolfbot

#include "../../../../../deps/airwindows/plugins/WinVST/Wolfbot/Wolfbot.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Wolfbot/WolfbotProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Wolfbot() { return new Wolfbot(nullptr); }
}

