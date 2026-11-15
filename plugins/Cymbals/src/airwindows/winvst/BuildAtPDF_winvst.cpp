// WinVST compilation wrapper for BuildAtPDF
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_BuildAtPDF

#include "../../../../../deps/airwindows/plugins/WinVST/BuildATPDF/BuildATPDF.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/BuildATPDF/BuildATPDFProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_BuildAtPDF() { return new BuildATPDF(nullptr); }
}

