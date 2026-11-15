#ifndef DISTRHO_PLUGIN_INFO_H_INCLUDED
#define DISTRHO_PLUGIN_INFO_H_INCLUDED

#define DISTRHO_PLUGIN_BRAND   "MadMcMans"
#define DISTRHO_PLUGIN_NAME    "MadMcMan's BCE (Best Cymbals Ever)"
#define DISTRHO_PLUGIN_URI     "https://www.patreon.com/c/MadMcMan"
#define DISTRHO_PLUGIN_CLAP_ID "com.madmcman.bce"
#ifndef DISTRHO_PLUGIN_VERSION
#define DISTRHO_PLUGIN_VERSION 0x010000
#endif

// Remains an instrument for preset/host compatibility, but exposes a stereo
// audio input in addition to MIDI. DPF wires VST3/CLAP/LV2 input buses from
// NUM_INPUTS, not from IS_SYNTH, so the input is available without changing
// the plugin category.
#define DISTRHO_PLUGIN_IS_SYNTH  1
#define DISTRHO_PLUGIN_NUM_INPUTS  2
#define DISTRHO_PLUGIN_NUM_OUTPUTS 2
#define DISTRHO_PLUGIN_WANT_PROGRAMS 1
#define DISTRHO_PLUGIN_WANT_STATE  1
#define DISTRHO_PLUGIN_WANT_FULL_STATE 1
#define DISTRHO_PLUGIN_WANT_MIDI_INPUT 1
#define DISTRHO_PLUGIN_WANT_TIMEPOS 1
#define DISTRHO_PLUGIN_IS_RT_SAFE 1
#define DISTRHO_PLUGIN_WANT_DIRECT_ACCESS 1

/* Enable LVGL-based UI */
#define DISTRHO_PLUGIN_HAS_UI 1
#define DISTRHO_UI_USE_CUSTOM 1
#define DISTRHO_UI_CUSTOM_INCLUDE_PATH "LVGL.hpp"
#define DISTRHO_UI_CUSTOM_WIDGET_TYPE DGL_NAMESPACE::LVGLTopLevelWidget

#define DISTRHO_UI_DEFAULT_WIDTH  1280
#define DISTRHO_UI_DEFAULT_HEIGHT 720
#define DISTRHO_UI_USE_NANOVG 0
#define DISTRHO_UI_USER_RESIZABLE 1
#define DISTRHO_UI_URI DISTRHO_PLUGIN_URI "#UI"

#define DISTRHO_PLUGIN_BRAND_ID Cymb
#define DISTRHO_PLUGIN_UNIQUE_ID Cymb

#define DISTRHO_PLUGIN_LV2_CATEGORY  "lv2:InstrumentPlugin"
#define DISTRHO_PLUGIN_VST3_CATEGORIES "Instrument|Synth"
#define DISTRHO_PLUGIN_CLAP_FEATURES "instrument", "synthesizer", "stereo"

#endif
