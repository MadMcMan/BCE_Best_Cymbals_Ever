#ifndef UI_COMMON_HPP
#define UI_COMMON_HPP

#include "DistrhoPlugin.hpp" // Required for START_NAMESPACE_DISTRHO
#include "lvgl.h"
#include "DistrhoPluginInfo.h"
#include <string>

START_NAMESPACE_DISTRHO

// tuning preset identifiers exposed to UI code
enum TuningPreset { PRESET_BESSEL = 0, PRESET_HARM, PRESET_PRIME,
                     PRESET_OCT, PRESET_FIFTH, PRESET_GOLD, PRESET_ET, PRESET_PRIMES,
                     PRESET_JUST };

// Interface to allow UI components to interact with the main UI
class AbstractCymbalsUI {
public:
    virtual ~AbstractCymbalsUI() {}
    // True while the UI is programmatically re-syncing widgets (host echo or
    // an XY-pad drag writing params). Widget callbacks that must only fire on
    // REAL user edits check this — programmatic VALUE_CHANGED events would
    // otherwise feed back and overwrite the values just written.
    virtual bool isInSync() const { return false; }
    virtual float getCurrentSampleRate() const { return 44100.0f; }
    virtual float getParamValue(uint32_t index) const = 0;
    // Display-only value: base + current modulation delta. The editor writes
    // it into arcs/curves so the UI follows LFOs; editing still uses
    // getParamValue(). Defaults to the base value.
    virtual float getModulatedParamValue(uint32_t index) const { return getParamValue(index); }
    virtual void setParamValue(uint32_t index, float value) = 0;
    virtual void editParameter(uint32_t index, bool start) = 0;
    // Round 2 Piece 1 (knob double-click reset): the canonical default value
    // the host exposes via Plugin::initParameter (Parameter::ranges.def).
    // Default 0.0f; the plugin UI overrides to read its real default table.
    virtual float getParameterDefault(uint32_t /*index*/) const { return 0.0f; }
    // Whether the parameter accepts a free continuous value (vs. enum/snap).
    // Default true; the plugin UI overrides to gate discrete (filter-type,
    // model, slope, sat-type) params from the reset path.
    virtual bool isParameterContinuous(uint32_t /*index*/) const { return true; }
    virtual void updateParamLabel(uint32_t index) = 0;
    virtual void syncParamWidget(uint32_t index, float value) = 0;
    virtual float getAnalyzerBand(int band) const = 0;
    virtual float getInharmBloomLevel() const = 0;
    virtual float getInharm2BloomLevel() const = 0;
    // Current stick-pitch MSEG playhead position (0..1), or -1 when no note is
    // sounding. Default hides the playhead; the plugin implements it.
    virtual float getStickMsegPhase() const { return -1.0f; }
    virtual void tuneGroupToBessel(int groupIdx) = 0;
    virtual void tuneGroupToHarmonic(int groupIdx) = 0;
    virtual void tuneGroupToPrime(int groupIdx) = 0;
    virtual void tuneGroupToPreset(int groupIdx, int preset) = 0;
    virtual void randomizeCombGroup(int groupIdx) = 0;

    virtual lv_event_cb_t getDragCallback() = 0;
    virtual void openPickerForSlot(int /*slotIndex*/) {}
    virtual void resetToInit() {}
    
    // Notify UI that a slot's algorithm type has changed (plugin picker selection)
    // Type is an AirSuiteUniversal::Type enum value (passed as int)
    virtual void notifySlotTypeChanged(int /*slot*/, int /*newType*/) {}

    // Modulation system: chip click opens the mod drawer focused on a param.
    // Default no-op; CymbalsUI implements it.
    virtual void openModDrawerForParam(int /*paramIndex*/) {}

    // --- Modulation system UI accessors (defaults keep tests/harnesses safe) ---
    virtual float getModSourceOutput(int /*s*/) const { return 0.0f; }
    virtual float getModSourcePhase(int /*s*/) const { return 0.0f; }
    virtual int getModSourceCycle(int /*s*/) const { return 0; }
    // How many parameters this modulation source currently drives (active
    // routes only). The header badge paints it as a route-count pip, which
    // is the source-side half of the routing signal — the target side is
    // the source-coloured mod ring on the routed knob.
    virtual int getModSourceRouteCount(int /*s*/) const { return 0; }
    // Audio-liveness + host tempo for the idle-preview gate (0 = idle; 120 = no transport).
    virtual uint32_t getModUiBlockCount() const { return 0; }
    virtual float getModTempoBpm() const { return 120.0f; }
    // Directory next to the plugin binary (for small UI sidecars such as FX
    // favourites). Empty means "no writable plugin directory known".
    virtual std::string getConfigDirectory() const { return std::string(); }
    virtual std::string parameterName(uint32_t /*index*/) const { return std::string(); }
    virtual bool parameterIsContinuous(uint32_t /*index*/) const { return false; }
    virtual int getLastModSource() const { return 0; }
    virtual void setLastModSource(int /*s*/) {}
    // Source-pill drag gesture (F4): the panel forwards LVGL events to the UI,
    // which owns the ghost/hit-test/amount-scrub state machine.
    virtual void onModSourceDragEvent(lv_event_t* /*e*/, int /*source*/) {}
    virtual bool modDragInProgress() const { return false; }
    // Matrix row containers, so syncParamWidget can toggle visibility on enable.
    virtual void setModRouteRow(int /*r*/, lv_obj_t* /*row*/) {}
    // Panel-initiated route changes (ADD ROUTE / remove / target picker). The
    // panel passes its own State so the UI can refresh the OTHER matrix surface
    // (the MOD stage vs the modal drawer) without rebuilding the changed one.
    // Default no-op keeps tests/harnesses safe.
    virtual void onModRoutesChanged(void* /*changedPanel*/) {}

    // A panel (mod drawer / source popup) was destroyed by its own UI (scrim
    // click, X button, external delete). The plugin must drop its pointer to the
    // freed State. Default no-op keeps tests/harnesses safe.
    virtual void onModPanelClosed(void* /*panel*/) {}
};

// ============================================================================
// "Azure studio" palette. Values were reconciled from live 1280x720 captures
// and cross-checked press-shot extractions: panel #2C2C2E, recessed #1E1E20,
// chrome/dividers #505050, engine-cyan mean #1A7491 with bright arc #17AFDD
// (hue ~197°). Modules read as flat plates separated by near-black seams; the
// accent (COL_HIGHLIGHT) is reserved for active states, values and indicators
// only — never chrome.
// ============================================================================
#define COL_BG         lv_color_hex(0x0A0B0C)  // chassis: window gaps (#070707, lifted to kill banding)
#define COL_PANEL      lv_color_hex(0x2C2C2E)  // module plate (research #2C2C2E)
#define COL_ACCENT     lv_color_hex(0x505050)  // neutral chrome: dividers/outlines (research #505050)
#define COL_HIGHLIGHT  lv_color_hex(0x17AFDD)  // azure accent (research bright arc #17AFDD, hue 197)
#define COL_TEXT       lv_color_hex(0xE8EAED)  // bright text (sampled #EEEEF4)
#define COL_TEXT_DIM   lv_color_hex(0x9DA0A6)  // dim gray labels (sampled #A6A6AB)

#define COL_TITLE      lv_color_hex(0xF2F3F5)  // section titles: bright near-white caps
#define COL_HAIRLINE   lv_color_hex(0x1A1B1D)  // plate seam: darker than plate (gap look)
#define COL_KNOB_LABEL lv_color_hex(0xA7ABB1)  // knob names

#define COL_SLIDER     COL_HIGHLIGHT
#define COL_KNOB       lv_color_hex(0xF0F1F3)
#define COL_KNOB_LIGHT lv_color_hex(0xDADDE1)
#define COL_KNOB_BORDER lv_color_hex(0x63676D)  // machined ticks on the cap
#define COL_WHITE      lv_color_hex(0xFFFFFF)
#define COL_BLACK      lv_color_hex(0x000000)
#define COL_CHART      COL_HIGHLIGHT
#define COL_CHART_BG   COL_PANEL

#define COL_KNOB_RING_BG    lv_color_hex(0x212326)
#define COL_KNOB_CAP        lv_color_hex(0x43464B)  // dark knob cap (#505050, tempered)
#define COL_KNOB_INDICATOR  COL_HIGHLIGHT
// Hover/press lifts for the knob ring + cap. Derived from the same azure and
// steel tokens rather than new hues, so the feedback reads as the knob waking
// up, not a different widget.
#define COL_KNOB_RING_BG_HOVER   lv_color_hex(0x33373C)  // track lifted on hover
#define COL_KNOB_INDICATOR_HOVER lv_color_hex(0x3FC2EA)  // azure lifted on hover
#define COL_KNOB_INDICATOR_PRESS lv_color_hex(0x6FD6F5)  // azure lit on press

// ----------------------------------------------------------------------------
// Machined metal cap material (black panel, silver knurled caps). Dark values
// read as metal in ~5bit RGB565 space, so every
// colour is deliberately > 0x20 with the lit side warm and the shadow side
// cool — the cap must read as a lit solid on the BLACK panel, not a sore thumb.
// Used by createSharedArcKnob's layoutKnobCapMaterial() (single definition, all
// sizes) and ONLY there. Text values above 0x7F are the only light source.
// ----------------------------------------------------------------------------
#define COL_KNOB_STEEL_LIT   lv_color_hex(0xDDE1E7)  // lit side: bright cool steel
#define COL_KNOB_STEEL_MID   lv_color_hex(0xA8AEB6)  // mid/crown of the knurl band
#define COL_KNOB_STEEL_SHAD  lv_color_hex(0x6E737A)  // shadow side: dark steel
#define COL_KNOB_STEEL_DEEP  lv_color_hex(0x3E4247)  // deep-shadow edge (shadow side)

// EQ band coding (kept; distinctive per-band colors are standard)
#define COL_EQ_LOW     lv_color_hex(0xEE7648)  // ENV AMP orange
#define COL_EQ_MID     lv_color_hex(0xECC346)  // LFO gold
#define COL_EQ_HIGH    lv_color_hex(0x3DAE49)  // FUNCTIONS green
#define COL_EQ_TOTAL   lv_color_hex(0xFFFFFF)

// Functional lane coding: envelope-section arcs take the ENV orange, time/rate
// arcs the yellow-olive lane. Everything else stays engine-cyan.
#define COL_ENV        lv_color_hex(0xFA7529)  // envelope arcs (ENV lane, saturated)
#define COL_TIME       lv_color_hex(0x908226)  // time/rate arcs (yellow-olive lane)

// Generic UI colors
#define COL_ERROR      lv_color_hex(0xE8574F)
#define COL_OFF        lv_color_hex(0x333538)
#define COL_SKIP       lv_color_hex(0x47494D)
#define COL_PANEL_DARK lv_color_hex(0x1E1F20)  // inset/card surface (research recessed #1E1E20)
#define COL_SHADOW     lv_color_hex(0x000000)
#define COL_ACCENT_DIM lv_color_hex(0x1A7491)  // dimmed azure = research mean engine-cyan
#define COL_BORDER     lv_color_hex(0x505050)  // standard border (research chrome #505050)
#define COL_CHECKED_BG lv_color_hex(0x0F87AF)  // checked button gradient accent

// Favourite dot colours (multi-coloured favourites; order matches kFavColours)
#define COL_FAV_RED     lv_color_hex(0xE8574F)
#define COL_FAV_ORANGE  lv_color_hex(0xEE7648)
#define COL_FAV_YELLOW  lv_color_hex(0xECC346)
#define COL_FAV_GREEN   lv_color_hex(0x3DAE49)
#define COL_FAV_BLUE    lv_color_hex(0x3D9BFF)
#define COL_FAV_PURPLE  lv_color_hex(0x9B7BE8)

// Button colors (consistent across UI)
#define COL_BTN_BG       COL_ACCENT
#define COL_BTN_TEXT     COL_TEXT
#define COL_BTN_ON       COL_HIGHLIGHT
#define COL_BTN_OFF      COL_OFF
#define COL_BTN_SKIP     COL_SKIP

// Keyboard colors
#define COL_KEYBOARD_BTN          lv_color_hex(0xDDE2E7)  // key face #B0C3D2, brightened
#define COL_KEYBOARD_BTN_PRESSED  lv_color_hex(0xB9C2CC)
#define COL_KEYBOARD_BORDER       lv_color_hex(0x14161A)
#define COL_KEYBOARD_TEXT         lv_color_hex(0x1A1D21)
#define COL_KEYBOARD_BLACK        lv_color_hex(0x1E2126)
#define COL_KEYBOARD_BLACK_PRESSED lv_color_hex(0x33383F)

// XY pad curve colors (curve 1 follows the accent; others stay as contrast hues)
#define COL_CURVE_1    COL_HIGHLIGHT
#define COL_CURVE_2    lv_color_hex(0xB488E8)
#define COL_CURVE_3    lv_color_hex(0x3ECFBF)
#define COL_CURVE_4    lv_color_hex(0xEE7648)

// XY pad heat colors
#define COL_HEAT_SHORT  COL_HIGHLIGHT
#define COL_HEAT_MEDIUM lv_color_hex(0xB488E8)
#define COL_HEAT_LONG   lv_color_hex(0x3ECFBF)

// XY pad circle colors
#define COL_CIRC_SELECTED COL_HIGHLIGHT
#define COL_CIRC_DEFAULT  lv_color_hex(0x505050)

// Modulation system (Phase: modulation)
#define COL_MOD        lv_color_hex(0x5DDDD0)   // bright teal: modulation ring / routed chip
#define COL_MOD_DIM    lv_color_hex(0x1E4A45)   // faint ring track on unmodulated knobs
#define COL_MACRO      lv_color_hex(0xB488E8)   // purple: macro knobs / macro-scaled routes

// Per-source identity palette (11 distinct; deliberately excludes COL_MOD teal
// and COL_MACRO purple so source cables never collide with the reserved mod
// colors). Mod rings take the routed source's color (COL_MACRO for macro routes).
#define COL_SRC_1      lv_color_hex(0x2EC4B6)   // teal-cyan
#define COL_SRC_2      lv_color_hex(0xD67AE6)   // orchid
#define COL_SRC_3      lv_color_hex(0xFF5A5A)   // red
#define COL_SRC_4      lv_color_hex(0xFF7AB8)   // pink
#define COL_SRC_5      lv_color_hex(0x8FD65C)   // green
#define COL_SRC_6      lv_color_hex(0x2E8BFF)   // blue (distinct from COL_MOD teal 0x5DDDD0)
#define COL_SRC_7      lv_color_hex(0x9B5CFF)   // violet (distinct from COL_MACRO 0xB488E8)
#define COL_SRC_8      lv_color_hex(0xB9C7E0)   // light blue-gray
#define COL_SRC_9      lv_color_hex(0xFFD54D)   // gold
#define COL_SRC_10     lv_color_hex(0xB0896A)   // brown
#define COL_SRC_11     lv_color_hex(0x9AA3AE)   // gray

// EQ editor colors (teal display canvas, matching the #283840 screens)
#define COL_EQ_CANVAS_BG     lv_color_hex(0x253037)  // display inset: desaturated teal-charcoal
#define COL_EQ_FREQ_LABEL    lv_color_hex(0xA7B3BC)
#define COL_EQ_DB_LABEL      lv_color_hex(0x9DAAB4)
#define COL_EQ_GRID          lv_color_hex(0x394851)
#define COL_EQ_GRID_ZERO     lv_color_hex(0x5A6C78)  // 0 dB datum: emphasized headroom divider
#define COL_EQ_SPECTRUM      COL_HIGHLIGHT
#define COL_EQ_SPECTRUM_TOP  lv_color_hex(0x9FE8FF)
#define COL_EQ_LINE          lv_color_hex(0xE8EAED)
#define COL_EQ_LABEL         lv_color_hex(0xE8EAED)
#define COL_EQ_HPF_BG        COL_HIGHLIGHT
#define COL_EQ_HPF_BORDER    lv_color_hex(0xE8EAED)
#define COL_EQ_LPF_BG        lv_color_hex(0xEE7648)
#define COL_EQ_LPF_BORDER    lv_color_hex(0xE8EAED)

// Modal/Browser colors (Phase 10)
#define UI_MODAL_OVERLAY_BG  lv_color_hex(0x00000088)  // Semi-transparent black
#define UI_BROWSER_BG        lv_color_hex(0xFFFFFF)    // White background
#define UI_BROWSER_BORDER    lv_color_hex(0x333333)    // Dark border

// --- Proportional GUI Scaling ---
// gUIScale = currentWidth / DISTRHO_UI_DEFAULT_WIDTH (1.280).
// scaled() multiplies any base pixel value by the current scale factor.
// getScaledFont() returns a progressively larger Montserrat as scale increases.
extern float gUIScale;
inline lv_coord_t scaled(lv_coord_t base) {
    return (lv_coord_t)(base * gUIScale + 0.5f);
}
inline const lv_font_t* getScaledFont() {
    if (gUIScale >= 1.5f) return &lv_font_montserrat_20;
    if (gUIScale >= 1.2f) return &lv_font_montserrat_16;
    return &lv_font_montserrat_14;
}
inline const lv_font_t* getScaledSmallFont() {
    if (gUIScale >= 1.5f) return &lv_font_montserrat_16;
    if (gUIScale >= 1.2f) return &lv_font_montserrat_14;
    return &lv_font_montserrat_12;  // compact small labels/values
}
// One step below getScaledSmallFont, for knob titles in NARROW cells. The FX
// card packs up to 8 arcs (7 algorithm params + WET) into ~395px, which is ~44px
// per cell - and at Montserrat 12 a 7-character name like "EncAmnt" needs ~49px,
// so every title ellipsised to "EncA...". The narrower face keeps the whole name.
inline const lv_font_t* getScaledTinyFont() {
    // One step below getScaledSmallFont, which is 12 at every scale below 1.5.
    // There is no larger tier to step up to here: the small face is already 12
    // at 1.2 and 1.5, so a 1.5 threshold returning 12 again would be dead code.
    return (gUIScale >= 1.2f) ? &lv_font_montserrat_12 : &lv_font_montserrat_10;
}
inline const lv_font_t* getScaledMicroFont() {
    if (gUIScale >= 1.5f) return &lv_font_montserrat_16;
    if (gUIScale >= 1.2f) return &lv_font_montserrat_14;
    return &lv_font_montserrat_12;  // chips/hints: tiny tracked labels
}
END_NAMESPACE_DISTRHO

#endif
