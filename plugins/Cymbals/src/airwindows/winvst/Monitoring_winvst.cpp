// WinVST compilation wrapper for Monitoring
// Auto-generated - do not edit manually.
#include "../vst2stub/audioeffectx.h"

// To prevent multiple definitions, we define a local createEffectInstance
// that will be static to this translation unit
#define createEffectInstance createEffectInstance_local_Monitoring

#include "../../../../../deps/airwindows/plugins/WinVST/Monitoring/Monitoring.cpp"
#include "../../../../../deps/airwindows/plugins/WinVST/Monitoring/MonitoringProc.cpp"

// Undefine to avoid issues elsewhere
#undef createEffectInstance

extern "C" {
AudioEffectX* aw_create_Monitoring() { return new Monitoring(nullptr); }
}

