#ifndef UI_EQ_EDITOR_HPP
#define UI_EQ_EDITOR_HPP

#include "lvgl.h"
#include "UICommon.hpp"
#include "UIWidgets.hpp"
#include "../PluginCymbals.hpp"
#include "../FFTAnalyzer.hpp"
#include "../UnifiedFilter.hpp"
#include "UIStyles.hpp"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <complex>
#include <array>

START_NAMESPACE_DISTRHO

struct UIEQEditor {
    // Live modulated value of a param: base clamped value + the current
    // accumulated route delta (mirrors updateModRings' per-target delta in
    // PluginUI.cpp). Lets the EQ graph animate the ACTUAL frequency/gain when
    // an LFO etc. is routed to a node's param, instead of freezing at the
    // base value. Reads only AbstractCymbalsUI accessors (shared kit stays
    // untouched); falls back to the base value when no route targets it.
    static float uiLiveValue(AbstractCymbalsUI* ui, uint32_t param) {
        return ui ? ui->getModulatedParamValue(param) : 0.0f;
    }


    static constexpr int kAdvancedTypeSelectionOff = 0;
    static constexpr int kAdvancedTypeSelectionLowpass2P = 1;
    static constexpr int kAdvancedTypeSelectionLowpass4P = 2;
    static constexpr int kAdvancedTypeSelectionLowpass6P = 3;
    static constexpr int kAdvancedTypeSelectionHighpass2P = 4;
    static constexpr int kAdvancedTypeSelectionHighpass4P = 5;

    struct PlotScale {
        float topDb = 24.0f;
        float bottomDb = -24.0f;
    };

    static constexpr int kUiDedicatedCutCharacterMax = 27;

    // ================================================================
    // PHASE 3: Unified Robust Policy for Preview Curves and DSP
    // ================================================================
    // Policy: All preview curve calculations route through the same 
    // robust model policy as runtime DSP to ensure:
    //   1. Consistent parameter safeguarding (clamping to valid ranges)
    //   2. No NaN/Inf propagation from intermediate calculations
    //   3. Finite guards applied to magnitude outputs
    //   4. Matching filter response across preview and DSP paths
    //
    // Implementation:
    //   - draw_main() preview uses UnifiedFilter::getMagnitudeForFrequency()
    //   - UnifiedFilter provides per-model frequency response
    //   - All 28 models are guarded with finite magnitude checks
    //   - Output log10() is protected by max(epsilon, value)
    // ================================================================

    struct ClassicQGestureState {
        bool active = false;
        int qParam = -1;
        int beginCount = 0;
        int endCount = 0;
    };

    static int poleCountFromSlope(int slope) {
        const int clamped = std::clamp(slope, 0, 3);
        return (clamped == 3) ? 8 : (2 + clamped * 2);
    }

    // Dropdown index → model value mapping for alphabetical ordering with None first.
    // Alphabetical order (None first), generated from UnifiedFilter::modelName.
    // Dropdown model order is generated from UnifiedFilter::modelName, not a
    // hand-maintained parallel array: None first, then case-insensitive name
    // order. This keeps the list correct as EQ models (including Airwindows
    // FILTER/BIQUAD/XYZ algorithms) are appended.
    static const std::vector<int>& modelDropdownOrder() {
        static const std::vector<int> order = [] {
            std::vector<int> v;
            v.reserve((size_t)UnifiedFilter::Count - 1);
            // The legacy Airwindows "Air" family (Air, Air2, Air3, Air4) is
            // catalogued as FILTER upstream but is a broadband exciter, not a
            // cutoff/Q filter. Positional mapping fed cutoff into "Air" and Q
            // into "Gnd", so those entries responded to the EQ controls with
            // the wrong parameters. Keep their model numbers for preset/DSP
            // compatibility but drop them from the selection list.
            auto hiddenLegacyAir = [](int m) {
                const int f = (int)UnifiedFilter::AirwindowsFirst;
                return m == f + 0 || m == f + 4 || m == f + 5 || m == f + 6 || m == f + 7;
            };
            for (int m = 0; m < (int)UnifiedFilter::Count; ++m)
                if (m != (int)UnifiedFilter::NoneModel && !hiddenLegacyAir(m)) v.push_back(m);
            std::sort(v.begin(), v.end(), [](int a, int b) {
                std::string na(UnifiedFilter::modelName(a));
                std::string nb(UnifiedFilter::modelName(b));
                std::transform(na.begin(), na.end(), na.begin(),
                               [](unsigned char c) { return (char)std::tolower(c); });
                std::transform(nb.begin(), nb.end(), nb.begin(),
                               [](unsigned char c) { return (char)std::tolower(c); });
                return (na != nb) ? (na < nb) : (a < b);
            });
            v.insert(v.begin(), (int)UnifiedFilter::NoneModel);
            return v;
        }();
        return order;
    }
    static int modelDropdownToValue(int idx) {
        const auto& order = modelDropdownOrder();
        return (idx >= 0 && idx < (int)order.size()) ? order[(size_t)idx] : 0;
    }
    static int modelValueToDropdown(int val) {
        const auto& order = modelDropdownOrder();
        for (size_t i = 0; i < order.size(); ++i)
            if (order[i] == val) return (int)i;
        return 0;
    }

    static const char* cutCharacterDropdownOptions() {
        static const std::string options = [] {
            std::string out;
            const auto& order = modelDropdownOrder();
            for (size_t i = 0; i < order.size(); ++i) {
                if (i) out.push_back('\n');
                out += UnifiedFilter::modelName(order[i]);
            }
            return out;
        }();
        return options.c_str();
    }

    static int cutSlopeModeFromArcValue(int arcValue) {
        const int clampedArcValue = std::clamp(arcValue, 0, 1000);
        return std::clamp((int)std::round((float)clampedArcValue * 3.0f / 1000.0f), 0, 3);
    }

    static int cutSlopeArcValueFromMode(int poleMode) {
        return (int)std::lround((float)std::clamp(poleMode, 0, 3) * (1000.0f / 3.0f));
    }

    static const char* cutSlopeDbPerOctLabelForPoles(int poles) {
        switch (std::clamp(poles, 2, 8)) {
            case 2: return "12 dB/oct";
            case 4: return "24 dB/oct";
            case 6: return "36 dB/oct";
            case 8: return "48 dB/oct";
            default: return "12 dB/oct";
        }
    }

    static const char* cutSlopeLabelForCharacterAndMode(int /*characterSelection*/, int poleMode) {
        const int poles = poleCountFromSlope(poleMode);
        return cutSlopeDbPerOctLabelForPoles(poles);
    }

    static int cutPolesParamForBand(int baseParam, int selectedBand) {
        if (selectedBand < 0 || selectedBand > 4) return -1;
        return slopeParamForBand(baseParam, selectedBand);
    }

    static PlotScale plotScaleForResponseExtrema(float minDb, float maxDb) {
        // The gain knobs (and every dB readout) span +/-24 dB, so the plot
        // must not auto-scale past that: a deep HP/LP stopband tail (down to
        // -96 dB) used to stretch the axis to +/-96, which made a maxed +24 dB
        // knob look like a quarter of the visible travel. Anything outside
        // +/-24 dB is clipped at the plot edge instead.
        constexpr float kMaxPlotDb = 24.0f;
        const float peak = std::max({std::fabs(minDb), std::fabs(maxDb), 1.0f});
        const float paddedPeak = peak * 1.12f + 0.75f;
        static const float kNiceLimits[] = {6.0f, 12.0f, 18.0f, 24.0f, 36.0f, 48.0f, 60.0f, 72.0f, 96.0f};

        float limit = kNiceLimits[sizeof(kNiceLimits) / sizeof(kNiceLimits[0]) - 1];
        for (float candidate : kNiceLimits) {
            if (paddedPeak <= candidate) {
                limit = candidate;
                break;
            }
        }

        limit = std::min(limit, kMaxPlotDb);
        return {limit, -limit};
    }

    // Improved scale contraction logic with hysteresis
    static PlotScale plotScaleForResponseExtremaWithHysteresis(float minDb, float maxDb, float currentTopDb, float currentBottomDb) {
        PlotScale newScale = plotScaleForResponseExtrema(minDb, maxDb);

        // If new scale is larger, always expand immediately (no hysteresis on expansion)
        if (newScale.topDb > currentTopDb || newScale.bottomDb < currentBottomDb) {
            return newScale;
        }

        // Only apply hysteresis when contracting: require at least 12dB reduction before shrinking
        const float contractionThreshold = 12.0f;
        if (std::abs(currentTopDb - newScale.topDb) > contractionThreshold) {
            return newScale;  // contract
        }

        return PlotScale{currentTopDb, currentBottomDb};  // keep current
    }

    static bool beginClassicQGesture(ClassicQGestureState& state, int qParam) {
        if (state.active && state.qParam == qParam) {
            return false;
        }
        state.active = true;
        state.qParam = qParam;
        ++state.beginCount;
        return true;
    }

    static bool endClassicQGesture(ClassicQGestureState& state, int qParam) {
        if (!state.active) {
            return false;
        }
        if (qParam >= 0 && state.qParam != qParam) {
            return false;
        }
        state.active = false;
        state.qParam = -1;
        ++state.endCount;
        return true;
    }

    static bool isArcReleaseEvent(lv_event_code_t code) {
        return code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST;
    }

    static float previewPassbandReferenceHz(bool highPass, float cutoffHz, float sampleRate) {
        const float safeSampleRate = std::max(8000.0f, sampleRate);
        const float maxHz = std::max(20.0f, safeSampleRate * 0.45f);
        if (highPass) {
            return std::clamp(std::max(cutoffHz * 4.0f, cutoffHz + 600.0f), std::min(maxHz, cutoffHz), maxHz);
        }
        return std::clamp(std::max(20.0f, cutoffHz * 0.25f), 20.0f, std::max(20.0f, std::min(maxHz, cutoffHz)));
    }

    static float previewBiquadMagnitude(DSPUtils::Biquad::Type type,
                                        float f0,
                                        float gainDb,
                                        float Q,
                                        float sampleRate,
                                        float freqHz) {
        DSPUtils::Biquad biquad;
        biquad.setParams(type, f0, gainDb, Q, sampleRate);
        return previewBiquadMagnitude(biquad, sampleRate, freqHz);
    }

    static float previewBiquadMagnitude(const DSPUtils::Biquad& biquad,
                                        float sampleRate,
                                        float freqHz) {
        const float safeSampleRate = std::max(8000.0f, sampleRate);
        const float evalHz = std::clamp(freqHz, 20.0f, safeSampleRate * 0.49f);
        const float w = 2.0f * (float)M_PI * evalHz / safeSampleRate;
        const std::complex<float> z1 = std::exp(std::complex<float>(0.0f, -w));
        const std::complex<float> z2 = std::exp(std::complex<float>(0.0f, -2.0f * w));
        const std::complex<float> num = biquad.b0 + biquad.b1 * z1 + biquad.b2 * z2;
        const std::complex<float> den = 1.0f + biquad.a1 * z1 + biquad.a2 * z2;
        return std::max(1.0e-6f, std::abs(num / den));
    }

    struct State {
        // Ownership: heap-allocated with 'new', deleted in canvas_event_cb LV_EVENT_DELETE
        // Also contains LVGL timer that must be deleted explicitly in cleanup
        AbstractCymbalsUI* ui;
        int baseParam; // kParamEQ1Freq or kParamFinalEQ1Freq
        int selectedBand = -1;
        uint8_t selectedBands = 0; // bitmask: ctrl-free marquee/group selection
        bool marqueeActive = false;
        lv_point_t marqueeStart{0, 0};
        lv_point_t marqueeCurrent{0, 0};
        bool groupDragActive = false;
        lv_point_t groupDragStart{0, 0};
        float groupStartFreq[5] = {0, 0, 0, 0, 0};
        float groupStartGain[5] = {0, 0, 0, 0, 0};
        float groupStartQ[5] = {0, 0, 0, 0, 0};
        int tiltAmountParam = -1;
        int tiltPivotParam = -1;
        // Mod-target registration: widgets[] entries for the selected band's
        // routable arcs (freq/gain/q/sat). Rebound by registerModArcs() whenever
        // the band selection changes; cleared on teardown.
        lv_obj_t** widgets = nullptr;
        uint32_t modParams[4] = {};
        lv_obj_t* canvas = nullptr;              // Ownership: parent (LVGL)
        lv_obj_t* freqAxis = nullptr;            // Ownership: parent (LVGL)
        lv_obj_t* settingsPanel = nullptr;       // Ownership: parent (LVGL)
        lv_obj_t* bandRow = nullptr;             // Ownership: parent (LVGL)
        lv_obj_t* bandButtons[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};
        lv_obj_t* typeArc = nullptr;             // Ownership: parent (LVGL)
        lv_obj_t* typeValueLabel = nullptr;      // Ownership: parent (LVGL)
        lv_obj_t* qArc = nullptr;                // Ownership: parent (LVGL)
        lv_obj_t* qValueLabel = nullptr;         // Ownership: parent (LVGL)
        lv_obj_t* typeCont = nullptr;            // Ownership: parent (LVGL)
        lv_obj_t* qCont = nullptr;               // Ownership: parent (LVGL)
        lv_obj_t* waspDriveCont = nullptr;       // Ownership: parent (LVGL)
        lv_obj_t* waspDriveArc = nullptr;        // Ownership: parent (LVGL)
        lv_obj_t* waspDriveValueLabel = nullptr; // Ownership: parent (LVGL)
        lv_obj_t* waspCharacterCont = nullptr;   // Ownership: parent (LVGL)
        lv_obj_t* waspCharacterArc = nullptr;    // Ownership: parent (LVGL)
        lv_obj_t* waspCharacterValueLabel = nullptr; // Ownership: parent (LVGL)
        lv_obj_t* poleCont = nullptr;            // Ownership: parent (LVGL)
        lv_obj_t* poleArc = nullptr;             // Ownership: parent (LVGL)
        lv_obj_t* poleValueLabel = nullptr;      // Ownership: parent (LVGL)
        lv_obj_t* poleLabel = nullptr;           // Ownership: parent (LVGL)
        lv_obj_t* charCont = nullptr;            // Ownership: parent (LVGL)
        lv_obj_t* charDropdown = nullptr;        // Ownership: parent (LVGL)
        lv_obj_t* charHelpLabel = nullptr;       // Ownership: parent (LVGL)
        lv_obj_t* freqCont = nullptr;            // Ownership: parent (LVGL)
        lv_obj_t* freqArc = nullptr;             // Ownership: parent (LVGL)
        lv_obj_t* freqValueLabel = nullptr;      // Ownership: parent (LVGL)
        lv_obj_t* gainCont = nullptr;            // Ownership: parent (LVGL)
        lv_obj_t* gainArc = nullptr;             // Ownership: parent (LVGL)
        lv_obj_t* gainValueLabel = nullptr;      // Ownership: parent (LVGL)
        lv_obj_t* satCont = nullptr;             // Ownership: parent (LVGL)
        lv_obj_t* satArc = nullptr;              // Ownership: parent (LVGL)
        lv_obj_t* satValueLabel = nullptr;       // Ownership: parent (LVGL)
        lv_obj_t* rippleCont = nullptr;          // Ownership: parent (LVGL)
        lv_obj_t* rippleArc = nullptr;           // Ownership: parent (LVGL)
        lv_obj_t* rippleValueLabel = nullptr;    // Ownership: parent (LVGL)
        lv_obj_t* stopbandCont = nullptr;        // Ownership: parent (LVGL)
        lv_obj_t* filterTypeDropdown = nullptr;   // Ownership: parent (LVGL)
        lv_timer_t* redrawTimer = nullptr;       // Ownership: LVGL timer (deleted in cleanup)
        // Preview response filters, allocated ONCE per editor instance. Reused across
        // redraws; magnitude evaluation is analytic (coefficients only), so the same
        // 5 instances are re-prepared each draw. Allocating them per-draw cost ~8MB of
        // malloc+memset at 30fps and stuttered the UI.
        std::unique_ptr<UnifiedFilter[]> previewFilters;
        std::vector<float> analyzerEnv;
        float analyzerVis = 0.0f;
        lv_obj_t* freqLabels[10] = {nullptr};    // Ownership: parent (LVGL)
        lv_obj_t* dbLabels[5] = {nullptr};       // Ownership: parent (LVGL)
        bool plotScaleDirty = false;
        // Smoothing state for Q parameter to prevent jumping when arc is dragged
        float qSmoothTarget = 0.0f;
        float qSmoothCurrent = 0.0f;
        const float kQSmoothFactor = 0.4f;  // Exponential moving average smoothing (0.0-1.0, higher = faster response)
        float plotTopDb = 24.0f;
        float plotBottomDb = -24.0f;
        ClassicQGestureState qGesture;
        bool poleGestureActive = false;
        uint8_t freqGestureBands = 0;
        uint8_t gainGestureBands = 0;
        uint8_t satGestureBands = 0;
        uint8_t qGestureBands = 0;
    };

    enum BandParamKind {
        kBandFreq = 0,
        kBandGain = 1,
        kBandQ = 2,
        kBandSat = 3
    };

    static int bandParamForKind(int baseParam, int band, BandParamKind kind) {
        if (band < 0 || band > 4) return -1;
        switch (kind) {
        case kBandFreq: return baseParam + band * 4;
        case kBandGain: return baseParam + band * 4 + 1;
        case kBandQ:    return baseParam + band * 4 + 2;
        case kBandSat:  return satParamForBand(baseParam, band);
        }
        return -1;
    }

    static int snapTypeToValid(int filterType, int model) {
        const uint32_t mask = UnifiedFilter::supportedTypesMask(model);
        filterType = std::clamp(filterType, 0, 7);
        if (mask & (1u << filterType)) return filterType;
        for (int t = 0; t < 8; ++t)
            if (mask & (1u << t)) return t;
        return 0;
    }

    // Returns the number of valid types for `model` and fills `outTypes` with them.
    static int getValidTypes(int model, int* outTypes, int maxCount = 8) {
        const uint32_t mask = UnifiedFilter::supportedTypesMask(model);
        int count = 0;
        for (int t = 0; t < 8 && count < maxCount; ++t)
            if (mask & (1u << t)) outTypes[count++] = t;
        return count;
    }

    // Maps arc value (0-1000) to a filter type for the given model.
    static int arcValueToType(int arcVal, int model) {
        int validTypes[8];
        int count = getValidTypes(model, validTypes);
        if (count <= 0) return 0;
        int idx = std::clamp((int)std::round(arcVal / 1000.0f * (count - 1)), 0, count - 1);
        return validTypes[idx];
    }

    // Maps a filter type to its arc position (0-1000) for the given model.
    static int typeToArcValue(int filterType, int model) {
        int validTypes[8];
        int count = getValidTypes(model, validTypes);
        if (count <= 1) return 0;
        for (int i = 0; i < count; ++i)
            if (validTypes[i] == filterType)
                return (int)((float)i / (float)(count - 1) * 1000.0f);
        return 0;
    }

    static lv_obj_t* create(lv_obj_t* parent, int baseParam, AbstractCymbalsUI* ui, UIStyles& styles, lv_obj_t* settingsHost = nullptr, lv_obj_t** widgets = nullptr, lv_coord_t extraTopHeight = 0, lv_obj_t** outTopBar = nullptr) {

        // --- Dense instrument layout (critic round 7): the Mix/Tilt/Pivot
        // strip lives INSIDE the graph container as a compact left-aligned
        // row above the canvas — no floating card band, no dead gap. The
        // band-editor card still appears in the strip when a band is edited.
        lv_obj_t* mainCont = lv_obj_create(parent);
        lv_obj_set_width(mainCont, lv_pct(100));
        lv_obj_set_height(mainCont, 0);
        lv_obj_set_flex_grow(mainCont, 1);
        lv_obj_set_style_bg_opa(mainCont, 0, 0);
        lv_obj_set_style_border_width(mainCont, 0, 0);
        lv_obj_set_style_pad_all(mainCont, 0, 0);
        lv_obj_set_style_pad_row(mainCont, 6, 0);
        lv_obj_clear_flag(mainCont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(mainCont, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_set_layout(mainCont, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(mainCont, LV_FLEX_FLOW_COLUMN);

        // --- Top strip: Mix/Tilt/Pivot (reparented from UIEQSection header)
        // plus the band-editor card (hidden until a band is selected).
        // Node-editor knobs (Type/Freq/Q/Slope/Sat/Gain): packed 36 rhythm,
        // universal with the rail (44px) and Mix (36px createArcEx) stacks —
        // title 15 + arc 36 + value 15 = 66 content, +4 value margin emulating
        // the packed createArcEx bottom air, in a 72px column (+2 slack, same
        // as rail 86-in-88). Card + topBar derive from containerH, so both
        // follow automatically. Defined here (not below) to size the strip.
        UIWidgets::ArcVisualSpec compactCutSpec = UIWidgets::smallArcVisualSpec();
        compactCutSpec.containerH = scaled(72);
        compactCutSpec.labelMarginBottom = 0;
        compactCutSpec.valueMarginTop = scaled(4);
        lv_obj_t* topBar = lv_obj_create(mainCont);
        lv_obj_set_width(topBar, lv_pct(100));
        // FIXED height = band-editor card height (knob column + card pad +
        // 2px border), top-aligned children: the header knob row used to
        // re-center vertically (and the canvas shift below it) every time the
        // card showed/hid. Knob Y is now identical in every state (batch 5).
        // Card pad is 4 (was 8) so the reserved strip is 84 not 92: the band
        // editor still fits exactly, and the canvas starts 8px higher — the
        // Mix/Tilt/Pivot knob Y itself does not move (still top-pinned).
        lv_obj_set_height(topBar, compactCutSpec.containerH + 2 * scaled(4) + 4 + extraTopHeight);
        lv_obj_set_style_bg_opa(topBar, 0, 0);
        lv_obj_set_style_border_width(topBar, 0, 0);
        lv_obj_set_style_pad_all(topBar, 0, 0);
        lv_obj_set_style_pad_column(topBar, 8, 0);
        lv_obj_set_layout(topBar, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(topBar, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(topBar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_scrollbar_mode(topBar, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(topBar, LV_OBJ_FLAG_SCROLLABLE);
        if (outTopBar) *outTopBar = topBar;

        if (settingsHost) {
            lv_obj_set_parent(settingsHost, topBar);
            lv_obj_set_width(settingsHost, LV_SIZE_CONTENT);
            lv_obj_set_flex_grow(settingsHost, 1);  // yields room when the settings card shows
            // Compact transparent strip: label + knobs directly on the panel.
            lv_obj_set_style_bg_opa(settingsHost, 0, 0);
            lv_obj_set_style_border_width(settingsHost, 0, 0);
            lv_obj_set_style_pad_all(settingsHost, 0, 0);
            lv_obj_set_style_pad_column(settingsHost, scaled(8), 0);
        }

        // --- Settings Panel: Filter Type / Q / Slope / Model (2nd card in topBar) ---
        lv_obj_t* settings = lv_obj_create(topBar);
        lv_obj_set_size(settings, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_add_style(settings, &styles.panel, 0);
        lv_obj_set_style_bg_opa(settings, LV_OPA_COVER, 0);
        // 4px pad (was 8) — must stay in lockstep with the topBar height
        // formula above so the card still fills the reserved strip exactly.
        lv_obj_set_style_pad_all(settings, scaled(4), 0);
        lv_obj_set_style_border_width(settings, 2, 0);
        lv_obj_set_style_border_color(settings, COL_ACCENT, 0);
        lv_obj_set_style_radius(settings, scaled(8), 0);
        lv_obj_set_style_shadow_width(settings, 0, 0);
        lv_obj_set_style_pad_column(settings, scaled(8), 0);
        lv_obj_set_layout(settings, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(settings, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(settings, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_scrollbar_mode(settings, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(settings, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(settings, LV_OBJ_FLAG_SCROLL_CHAIN);

        // --- EQ Display (Top) ---
        lv_obj_t* canvas = lv_obj_create(mainCont);
        lv_obj_set_size(canvas, lv_pct(100), 0);
        lv_obj_set_flex_grow(canvas, 1);
        // Scale labels are positioned absolutely against the FULL canvas
        // height (layout_scale_labels) — theme padding must be zero or they
        // shift down and clip past the bottom edge.
        lv_obj_set_style_pad_all(canvas, 0, 0);
        lv_obj_set_style_bg_color(canvas, COL_EQ_CANVAS_BG, 0);
        lv_obj_set_style_bg_opa(canvas, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(canvas, COL_ACCENT, 0);
        lv_obj_set_style_border_width(canvas, 1, 0);
        lv_obj_set_style_radius(canvas, 4, 0);
        lv_obj_clear_flag(canvas, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(canvas, LV_OBJ_FLAG_SCROLL_CHAIN);
        lv_obj_add_flag(canvas, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_t* freqAxis = lv_obj_create(mainCont);
        lv_obj_set_size(freqAxis, lv_pct(100), scaled(20));
        lv_obj_set_style_bg_opa(freqAxis, 0, 0);
        lv_obj_set_style_border_width(freqAxis, 0, 0);
        lv_obj_set_style_pad_all(freqAxis, 0, 0);
        lv_obj_set_scrollbar_mode(freqAxis, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(freqAxis, LV_OBJ_FLAG_SCROLLABLE);

        // Ownership: user-allocated, deleted in canvas_event_cb when LV_EVENT_DELETE fires
        State* state = new State();
        state->ui = ui;
        state->baseParam = baseParam;
        state->widgets = widgets;
        if (baseParam == PluginCymbals::kParamEQ1Freq) {
            state->tiltAmountParam = PluginCymbals::kParamTiltAmount;
            state->tiltPivotParam = PluginCymbals::kParamTiltPivot;
        } else {
            state->tiltAmountParam = PluginCymbals::kParamFinalTiltAmount;
            state->tiltPivotParam = PluginCymbals::kParamFinalTiltPivot;
        }
        state->canvas = canvas;
        state->freqAxis = freqAxis;
        state->settingsPanel = settings;
        activeEditors()[baseParam] = state;
        lv_obj_add_event_cb(settings, outside_press_cb, LV_EVENT_PRESSED, state);
        lv_obj_add_event_cb(freqAxis, [](lv_event_t* e) {
            State* s = (State*)lv_event_get_user_data(e);
            if (s) layout_scale_labels(s);
        }, LV_EVENT_SIZE_CHANGED, state);
        state->analyzerEnv.assign(FFTAnalyzer::kNumBands, 0.0f);
        state->previewFilters = std::make_unique<UnifiedFilter[]>(5);
        // Ownership: LVGL timer, deleted in canvas_event_cb LV_EVENT_DELETE
        state->redrawTimer = lv_timer_create(canvas_timer_cb, 33, state);

        create_scale_labels(state);
        layout_scale_labels(state);

        lv_obj_set_user_data(canvas, state);
        lv_obj_add_event_cb(canvas, canvas_event_cb, LV_EVENT_ALL, state);
        lv_obj_add_event_cb(mainCont, outside_press_cb, LV_EVENT_PRESSED, state);
        lv_obj_add_event_cb(mainCont, [](lv_event_t* e) {
            State* s = (State*)lv_event_get_user_data(e);
            if (!s) return;
            update_settings_width(s);
            layout_band_row(s);
            layout_scale_labels(s);
        }, LV_EVENT_SIZE_CHANGED, state);
        // OWNERSHIP CLEANUP: Delete heap-allocated state and timer when LVGL destroys object
        lv_obj_add_event_cb(canvas, [](lv_event_t* e) {
            State* s = (State*)lv_event_get_user_data(e);
            if (s) {
                finalize_classic_q_gesture(s, false);
                finalize_cut_arc_gestures(s);
                // Release the mod-target widgets[] bindings before the arcs die.
                if (s->widgets) {
                    for (int k = 0; k < 4; ++k) {
                        if (s->modParams[k] && s->modParams[k] < PluginCymbals::kParameterCount)
                            s->widgets[s->modParams[k]] = nullptr;
                    }
                }
                if (s->redrawTimer) lv_timer_delete(s->redrawTimer);
                auto it = activeEditors().find(s->baseParam);
                if (it != activeEditors().end() && it->second == s)
                    activeEditors().erase(it);
                delete s;
            }
        }, LV_EVENT_DELETE, state);

        // Numbered band strip directly below Mix. Same 20x18 toggle design as
        // the filter bank's per-stage enable row; clicking a number highlights
        // that node and opens its editor, without needing the small hit target.
        {
            lv_obj_t* bandRow = lv_obj_create(mainCont);
            lv_obj_set_size(bandRow, LV_SIZE_CONTENT, scaled(18));
            lv_obj_set_style_bg_opa(bandRow, 0, 0);
            lv_obj_set_style_border_width(bandRow, 0, 0);
            lv_obj_set_style_pad_all(bandRow, 0, 0);
            lv_obj_set_style_pad_column(bandRow, scaled(2), 0);
            lv_obj_set_layout(bandRow, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(bandRow, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(bandRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(bandRow, LV_OBJ_FLAG_SCROLLABLE);
            // Insert between the Mix/Tilt/Pivot strip and the graph.
            lv_obj_move_to_index(bandRow, lv_obj_get_index(canvas));
            for (int i = 0; i < 5; ++i) {
                lv_obj_t* b = lv_btn_create(bandRow);
                lv_obj_set_size(b, scaled(20), scaled(18));
                styles.applyToggleButton(b, false);
                lv_obj_set_style_pad_all(b, 0, 0);
                // Non-param payload: raw 0..4 would collide with kParamTone
                // family indices and repointWidgetsRecursive would steal
                // widgets[0..4] onto these buttons.
                lv_obj_set_user_data(b, (void*)uiEncodeNonParamPayload(i));
                lv_obj_add_event_cb(b, bandButtonCb, LV_EVENT_CLICKED, state);
                lv_obj_t* n = lv_label_create(b);
                lv_obj_remove_style_all(n);
                char nb[2] = {(char)('1' + i), '\0'};
                lv_label_set_text(n, nb);
                lv_obj_set_style_text_font(n, uiMicroFont(), 0);
                lv_obj_set_style_text_color(n, COL_TEXT, 0);
                lv_obj_center(n);
                state->bandButtons[i] = b;
            }
            state->bandRow = bandRow;
        }

        lv_obj_t* controls = lv_obj_create(settings);
        lv_obj_set_size(controls, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(controls, 0, 0);
        lv_obj_set_style_border_width(controls, 0, 0);
        lv_obj_set_style_pad_all(controls, 0, 0);
        lv_obj_set_style_pad_column(controls, 8, 0);
        lv_obj_set_layout(controls, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(controls, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(controls, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_scrollbar_mode(controls, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(controls, LV_OBJ_FLAG_SCROLLABLE);

        // compactCutSpec is defined above (sized the topBar); reused here for
        // the node-editor knob columns.

        // --- Model dropdown (first in row, hidden for basic types) ---
        lv_obj_t* charCont = UIWidgets::createCompactControlColumn(controls, 158, compactCutSpec.containerH, 0);
        state->charCont = charCont;

        UIWidgets::createKnobTitleLabel(charCont, "Model", styles, 158);

        state->charDropdown = UIWidgets::createCompactDropdown(charCont, styles, 154, scaled(32));
        // nudge dropdown box center down onto the knob-arc centers (bottom-aligned column)
        lv_obj_set_style_margin_top(state->charDropdown, scaled(2), 0);
        lv_dropdown_set_options(state->charDropdown, cutCharacterDropdownOptions());
        lv_obj_add_event_cb(state->charDropdown, settings_changed_cb, LV_EVENT_VALUE_CHANGED, state);
        uiEnableDropdownListScroll(state->charDropdown);
        uiFixDropdownPopup(state->charDropdown);

        state->charHelpLabel = lv_label_create(charCont);
        lv_label_set_text(state->charHelpLabel, "");
        lv_obj_set_style_text_font(state->charHelpLabel, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(state->charHelpLabel, COL_TEXT_DIM, 0);
        lv_obj_set_width(state->charHelpLabel, scaled(120));
        lv_label_set_long_mode(state->charHelpLabel, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_align(state->charHelpLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_add_flag(state->charHelpLabel, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_flag(state->charCont, LV_OBJ_FLAG_HIDDEN);

        // --- Type knob (replaces dropdown, snaps to 8 positions) ---
        lv_obj_t* typeCont = UIWidgets::createCompactControlColumn(controls,
                                       compactCutSpec.containerW,
                                       compactCutSpec.containerH,
                                       0);
        state->typeCont = typeCont;

        UIWidgets::createKnobTitleLabel(typeCont, "Type", styles, compactCutSpec.containerW);

        state->typeArc = UIWidgets::createSimpleArcWithDrag(typeCont, compactCutSpec.arcSize, 0.0f, styles,
                                    q_arc_drag_cb, state);
        lv_obj_add_event_cb(state->typeArc, settings_changed_cb, LV_EVENT_VALUE_CHANGED, state);

        state->typeValueLabel = UIWidgets::createCompactValueLabel(typeCont, styles,
                                       compactCutSpec.containerW,
                                       compactCutSpec.valueMarginTop);
        lv_obj_add_flag(state->typeCont, LV_OBJ_FLAG_HIDDEN);

        // --- Frequency Arc (always visible, controls node x-axis position) ---
        lv_obj_t* freqCont = UIWidgets::createCompactControlColumn(controls,
                                       compactCutSpec.containerW,
                                       compactCutSpec.containerH,
                                       0);
        state->freqCont = freqCont;

        UIWidgets::createKnobTitleLabel(freqCont, "Freq", styles, compactCutSpec.containerW);

        state->freqArc = UIWidgets::createSimpleArcWithDrag(freqCont, compactCutSpec.arcSize, 0.5f, styles,
                                                             q_arc_drag_cb, state);
        lv_obj_add_event_cb(state->freqArc, settings_changed_cb, LV_EVENT_VALUE_CHANGED, state);
        lv_obj_add_event_cb(state->freqArc, arc_release_cb, LV_EVENT_RELEASED, state);
        lv_obj_add_event_cb(state->freqArc, arc_release_cb, LV_EVENT_PRESS_LOST, state);

        state->freqValueLabel = UIWidgets::createCompactValueLabel(freqCont, styles,
                                       compactCutSpec.containerW,
                                       compactCutSpec.valueMarginTop);

        // --- Q Arc ---
        lv_obj_t* qCont = UIWidgets::createCompactControlColumn(controls,
                                       compactCutSpec.containerW,
                                       compactCutSpec.containerH,
                                       0);
        state->qCont = qCont;

        UIWidgets::createKnobTitleLabel(qCont, "Q", styles, compactCutSpec.containerW);

        state->qArc = UIWidgets::createSimpleArcWithDrag(qCont, compactCutSpec.arcSize, 0.5f, styles,
                                                         q_arc_drag_cb, state);

        state->qValueLabel = UIWidgets::createCompactValueLabel(qCont, styles,
                                       compactCutSpec.containerW,
                                       compactCutSpec.valueMarginTop);

        lv_obj_add_event_cb(state->qArc, settings_changed_cb, LV_EVENT_VALUE_CHANGED, state);
        lv_obj_add_event_cb(state->qArc, q_arc_finalize_cb, LV_EVENT_ALL, state);

        // --- Slope (hidden for non-filter types) ---
        lv_obj_t* poleCont = UIWidgets::createCompactControlColumn(controls,
                                       compactCutSpec.containerW,
                                       compactCutSpec.containerH,
                                       0);
        state->poleCont = poleCont;

        state->poleLabel = UIWidgets::createKnobTitleLabel(poleCont, "Slope", styles, compactCutSpec.containerW);
        lv_obj_set_style_margin_bottom(state->poleLabel, compactCutSpec.labelMarginBottom, 0);

        state->poleArc = UIWidgets::createSimpleArcWithDrag(poleCont, compactCutSpec.arcSize, 0.0f, styles,
                                    q_arc_drag_cb, state);
        lv_obj_add_event_cb(state->poleArc, settings_changed_cb, LV_EVENT_VALUE_CHANGED, state);

        state->poleValueLabel = UIWidgets::createCompactValueLabel(poleCont, styles,
                                       compactCutSpec.containerW,
                                       compactCutSpec.valueMarginTop);
        lv_obj_add_flag(state->poleCont, LV_OBJ_FLAG_HIDDEN);

        // --- Gain Arc (visible only for Bell/LowShelf/HighShelf types 4-6) ---
        lv_obj_t* gainCont = UIWidgets::createCompactControlColumn(controls,
                                       compactCutSpec.containerW,
                                       compactCutSpec.containerH,
                                       0);
        state->gainCont = gainCont;

        UIWidgets::createKnobTitleLabel(gainCont, "Gain", styles, compactCutSpec.containerW);

        state->gainArc = UIWidgets::createSimpleArcWithDrag(gainCont, compactCutSpec.arcSize, 0.5f, styles,
                                                             q_arc_drag_cb, state);
        lv_obj_add_event_cb(state->gainArc, settings_changed_cb, LV_EVENT_VALUE_CHANGED, state);
        lv_obj_add_event_cb(state->gainArc, arc_release_cb, LV_EVENT_RELEASED, state);
        lv_obj_add_event_cb(state->gainArc, arc_release_cb, LV_EVENT_PRESS_LOST, state);

        state->gainValueLabel = UIWidgets::createCompactValueLabel(gainCont, styles,
                                       compactCutSpec.containerW,
                                       compactCutSpec.valueMarginTop);
        lv_obj_add_flag(state->gainCont, LV_OBJ_FLAG_HIDDEN);

        // --- Saturation Arc (always visible, controls input drive) ---
        lv_obj_t* satCont = UIWidgets::createCompactControlColumn(controls,
                                       compactCutSpec.containerW,
                                       compactCutSpec.containerH,
                                       0);
        state->satCont = satCont;

        UIWidgets::createKnobTitleLabel(satCont, "Sat", styles, compactCutSpec.containerW);

        state->satArc = UIWidgets::createSimpleArcWithDrag(satCont, compactCutSpec.arcSize, 0.0f, styles,
                                                             q_arc_drag_cb, state);
        lv_obj_add_event_cb(state->satArc, settings_changed_cb, LV_EVENT_VALUE_CHANGED, state);
        lv_obj_add_event_cb(state->satArc, arc_release_cb, LV_EVENT_RELEASED, state);
        lv_obj_add_event_cb(state->satArc, arc_release_cb, LV_EVENT_PRESS_LOST, state);

        state->satValueLabel = UIWidgets::createCompactValueLabel(satCont, styles,
                                       compactCutSpec.containerW,
                                       compactCutSpec.valueMarginTop);

        // --- Wasp Drive Arc (hidden by default) ---
        lv_obj_t* waspDriveCont = lv_obj_create(controls);
        state->waspDriveCont = waspDriveCont;
        lv_obj_set_size(waspDriveCont, compactCutSpec.containerW, compactCutSpec.containerH);
        lv_obj_set_style_bg_opa(waspDriveCont, 0, 0);
        lv_obj_set_style_border_width(waspDriveCont, 0, 0);
        lv_obj_set_style_pad_all(waspDriveCont, 0, 0);
        lv_obj_set_style_pad_row(waspDriveCont, 1, 0);
        lv_obj_set_layout(waspDriveCont, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(waspDriveCont, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(waspDriveCont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_scrollbar_mode(waspDriveCont, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(waspDriveCont, LV_OBJ_FLAG_SCROLLABLE);

        UIWidgets::createKnobTitleLabel(waspDriveCont, "Drive", styles, compactCutSpec.containerW);

        state->waspDriveArc = UIWidgets::createSimpleArcWithDrag(waspDriveCont, compactCutSpec.arcSize, 0.0f, styles,
                                     q_arc_drag_cb, state);
        lv_obj_add_event_cb(state->waspDriveArc, settings_changed_cb, LV_EVENT_VALUE_CHANGED, state);

        state->waspDriveValueLabel = lv_label_create(waspDriveCont);
        lv_obj_add_style(state->waspDriveValueLabel, &styles.label, 0);
        lv_obj_set_style_text_font(state->waspDriveValueLabel, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_align(state->waspDriveValueLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_margin_top(state->waspDriveValueLabel, 1, 0);

        // --- Wasp Character Arc (hidden by default) ---
        lv_obj_t* waspCharacterCont = lv_obj_create(controls);
        state->waspCharacterCont = waspCharacterCont;
        lv_obj_set_size(waspCharacterCont, compactCutSpec.containerW, compactCutSpec.containerH);
        lv_obj_set_style_bg_opa(waspCharacterCont, 0, 0);
        lv_obj_set_style_border_width(waspCharacterCont, 0, 0);
        lv_obj_set_style_pad_all(waspCharacterCont, 0, 0);
        lv_obj_set_style_pad_row(waspCharacterCont, 1, 0);
        lv_obj_set_layout(waspCharacterCont, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(waspCharacterCont, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(waspCharacterCont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_scrollbar_mode(waspCharacterCont, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(waspCharacterCont, LV_OBJ_FLAG_SCROLLABLE);

        UIWidgets::createKnobTitleLabel(waspCharacterCont, "Char", styles, compactCutSpec.containerW);

        state->waspCharacterArc = UIWidgets::createSimpleArcWithDrag(waspCharacterCont, compactCutSpec.arcSize, 0.5f, styles,
                                         q_arc_drag_cb, state);
        lv_obj_add_event_cb(state->waspCharacterArc, settings_changed_cb, LV_EVENT_VALUE_CHANGED, state);

        state->waspCharacterValueLabel = lv_label_create(waspCharacterCont);
        lv_obj_add_style(state->waspCharacterValueLabel, &styles.label, 0);
        lv_obj_set_style_text_font(state->waspCharacterValueLabel, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_align(state->waspCharacterValueLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_margin_top(state->waspCharacterValueLabel, 1, 0);

        lv_obj_add_flag(state->waspDriveCont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(state->waspCharacterCont, LV_OBJ_FLAG_HIDDEN);

        // --- Ripple/Resonance Arc (hidden by default) ---
        lv_obj_t* rippleCont = UIWidgets::createCompactControlColumn(controls,
                                         compactCutSpec.containerW,
                                         compactCutSpec.containerH,
                                         0);
        state->rippleCont = rippleCont;

        lv_obj_t* rippleLabel = UIWidgets::createKnobTitleLabel(rippleCont, "Resonance", styles,
                         compactCutSpec.containerW);
        lv_obj_set_style_margin_bottom(rippleLabel, compactCutSpec.labelMarginBottom, 0);

        state->rippleArc = UIWidgets::createSimpleArcWithDrag(rippleCont, compactCutSpec.arcSize, 0.5f, styles,
                                                              q_arc_drag_cb, state);
        lv_obj_add_event_cb(state->rippleArc, settings_changed_cb, LV_EVENT_VALUE_CHANGED, state);

        state->rippleValueLabel = UIWidgets::createCompactValueLabel(rippleCont, styles,
                                         compactCutSpec.containerW,
                                         compactCutSpec.valueMarginTop);
        lv_obj_add_flag(state->rippleCont, LV_OBJ_FLAG_HIDDEN);

        // --- Filter Type dropdown (stopbandCont, hidden by default) ---
        const lv_coord_t kStopbandW = scaled(100);
        lv_obj_t* stopbandCont = UIWidgets::createCompactControlColumn(controls,
                                           kStopbandW,
                                           LV_SIZE_CONTENT,
                                           1);
        state->stopbandCont = stopbandCont;

        UIWidgets::createKnobTitleLabel(stopbandCont, "Type", styles,
                           kStopbandW);

        state->filterTypeDropdown = UIWidgets::createCompactDropdown(stopbandCont, styles, kStopbandW, scaled(32));
        lv_dropdown_set_options(state->filterTypeDropdown, "Low Pass\nBand Pass\nHigh Pass\nNotch\nBell\nLow Shelf\nHigh Shelf\nAll-Pass");
        lv_obj_add_event_cb(state->filterTypeDropdown, settings_changed_cb, LV_EVENT_VALUE_CHANGED, state);
        uiEnableDropdownListScroll(state->filterTypeDropdown);
        uiFixDropdownPopup(state->filterTypeDropdown);

        lv_obj_add_flag(state->stopbandCont, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_flag(settings, LV_OBJ_FLAG_HIDDEN); // Hidden by default
        update_settings_width(state);
        // Plugin-local cap material on all node knobs (softened bands).
        // Pack every band knob (exact-box card columns: kcW == arcSize) —
        // same absolute rhythm as rail/Mix, plus the ring hug + de-dot.
        lv_obj_t* cardArcs[] = {state->typeArc, state->freqArc, state->qArc, state->poleArc,
                                state->gainArc, state->satArc, state->rippleArc,
                                state->waspDriveArc, state->waspCharacterArc};
        for (lv_obj_t* a : cardArcs) uiPackKnob(a, compactCutSpec.arcSize, scaled(0), compactCutSpec.arcSize);
        lv_obj_update_layout(mainCont);
        layout_band_row(state);
        return mainCont;
    }

    static int advancedTypeParamForBand(int baseParam, int band) {
        if (band < 0 || band > 2) return -1;
        if (baseParam == PluginCymbals::kParamEQ1Freq) {
            return PluginCymbals::kParamEQ1AdvType + band * 2;
        }
        if (baseParam == PluginCymbals::kParamFinalEQ1Freq) {
            return PluginCymbals::kParamFinalEQ1AdvType + band * 2;
        }
        return -1;
    }

    // UnifiedFilter per-band params (all 5 nodes)
    static int eqBandModelParam(int baseParam, int band) {
        if (band < 0 || band > 4) return -1;
        if (baseParam == PluginCymbals::kParamEQ1Freq) return PluginCymbals::kParamEQ1Model + band * 3;
        if (baseParam == PluginCymbals::kParamFinalEQ1Freq) return PluginCymbals::kParamFinalEQ1Model + band * 3;
        return -1;
    }
    static int eqBandFilterTypeParam(int baseParam, int band) {
        const int m = eqBandModelParam(baseParam, band);
        return (m >= 0) ? (m + 1) : -1;
    }
    static int eqBandSlopeParam(int baseParam, int band) {
        const int m = eqBandModelParam(baseParam, band);
        return (m >= 0) ? (m + 2) : -1;
    }

    // Model dropdown options for all 5 filter nodes: index 0 = None, then every UnifiedFilter model
    static const char* eqBandModelDropdownOptions() {
        return cutCharacterDropdownOptions();
    }

    // Unified param lookup for ALL 5 nodes
    static int modelParamForBand(int baseParam, int selectedBand) {
        return eqBandModelParam(baseParam, selectedBand);
    }
    static int filterTypeParamForBand(int baseParam, int selectedBand) {
        return eqBandFilterTypeParam(baseParam, selectedBand);
    }
    static int slopeParamForBand(int baseParam, int selectedBand) {
        return eqBandSlopeParam(baseParam, selectedBand);
    }
    // A band is disabled when its model is NoneModel. The DSP skips it and the
    // magnitude is flat (0 dB); the graph must reflect that instead of painting
    // a live-looking coloured curve/node (reported: disabled bands stayed
    // colourful when switching EQ screens).
    static bool bandDisabled(const State* state, int band) {
        if (state == nullptr || band < 0 || band > 4) return false;
        const int mp = modelParamForBand(state->baseParam, band);
        if (mp < 0) return false;
        return (int)std::lround(state->ui->getParamValue((uint32_t)mp)) ==
               (int)UnifiedFilter::NoneModel;
    }

    static int satParamForBand(int baseParam, int band) {
        if (baseParam == PluginCymbals::kParamEQ1Freq) return PluginCymbals::kParamEQ1Sat + band;
        if (baseParam == PluginCymbals::kParamFinalEQ1Freq) return PluginCymbals::kParamFinalEQ1Sat + band;
        return -1;
    }
    static int classicTypeParamForBand(int baseParam, int selectedBand) {
        if (selectedBand < 0 || selectedBand > 4) return -1;
        return baseParam + selectedBand * 4 + 3;
    }
    static int qParamForBand(int baseParam, int selectedBand) {
        if (selectedBand < 0 || selectedBand > 4) return -1;
        return baseParam + selectedBand * 4 + 2;
    }
    static float readQNorm(State* state, int qParamIdx) {
        if (qParamIdx < 0) return 0.5f;
        return std::clamp(state->ui->getParamValue(qParamIdx), 0.0f, 1.0f);
    }
    static void writeQNorm(State* state, int qParamIdx, float qNorm) {
        if (qParamIdx < 0) return;
        state->ui->setParamValue(qParamIdx, std::clamp(qNorm, 0.0f, 1.0f));
    }

    static int advancedResParamForBand(int baseParam, int band) {
        const int typeParam = advancedTypeParamForBand(baseParam, band);
        return (typeParam >= 0) ? (typeParam + 1) : -1;
    }

    static int advancedDriveParamForBand(int baseParam, int band) {
        if (band < 0 || band > 2) return -1;
        if (baseParam == PluginCymbals::kParamEQ1Freq) {
            return PluginCymbals::kParamEQ1AdvDrive + band * 2;
        }
        if (baseParam == PluginCymbals::kParamFinalEQ1Freq) {
            return PluginCymbals::kParamFinalEQ1AdvDrive + band * 2;
        }
        return -1;
    }

    static int advancedCharacterParamForBand(int baseParam, int band) {
        const int driveParam = advancedDriveParamForBand(baseParam, band);
        return (driveParam >= 0) ? (driveParam + 1) : -1;
    }

    static int advancedTypeSelectionFromParam(float normValue) {
        const float clamped = std::clamp(normValue, 0.0f, 1.0f);
        if (clamped < 0.1f) return kAdvancedTypeSelectionOff;
        if (clamped < 0.3f) return kAdvancedTypeSelectionLowpass2P;
        if (clamped < 0.5f) return kAdvancedTypeSelectionLowpass4P;
        if (clamped < 0.7f) return kAdvancedTypeSelectionLowpass6P;
        if (clamped < 0.9f) return kAdvancedTypeSelectionHighpass2P;
        return kAdvancedTypeSelectionHighpass4P;
    }

    // Visualizer decode shares the same generic selector mapping as the UI dropdown.
    static int advancedVisualizerModeFromParam(float normValue) {
        return advancedTypeSelectionFromParam(normValue);
    }

    static float advancedTypeParamFromSelection(int selection) {
        switch (std::clamp(selection, 0, 5)) {
            case kAdvancedTypeSelectionLowpass2P: return 0.2f;
            case kAdvancedTypeSelectionLowpass4P: return 0.4f;
            case kAdvancedTypeSelectionLowpass6P: return 0.6f;
            case kAdvancedTypeSelectionHighpass2P: return 0.8f;
            case kAdvancedTypeSelectionHighpass4P: return 1.0f;
            case kAdvancedTypeSelectionOff:
            default: return 0.0f;
        }
    }

    static float resolveSampleRate(const AbstractCymbalsUI* ui) {
        constexpr float kFallbackSampleRate = 44100.0f;
        if (ui == nullptr) {
            return kFallbackSampleRate;
        }

        const float sr = ui->getCurrentSampleRate();
        if (!std::isfinite(sr) || sr <= 0.0f) {
            return kFallbackSampleRate;
        }

        return sr;
    }

    static int compactArcDragValueFromPressOrigin(int startValue, int dx, int dy) {
        // Same sensitivity as the app-wide arcDragCb (full range ~286px),
        // including its Shift fine-drag scaling.
        const float travel = (float)dx - (float)dy;
        const float fine = UIWidgets::isFineDragActive() ? 0.1f : 1.0f;
        return std::clamp((int)std::lround((float)startValue + travel * 3.5f * fine), 0, 1000);
    }

    // Test hook (headless render harness): mirrors the node-click path — select a
    // band, reveal the settings panel, and resync every settings knob + label
    // from the param state, exactly as the canvas click handler does.
    static void syncSettingsPanelForHarness(State* state) {
        if (!state) return;
        if (state->settingsPanel) lv_obj_remove_flag(state->settingsPanel, LV_OBJ_FLAG_HIDDEN);
        update_settings_ui(state);
    }

    static bool isBandSelected(const State* state, int band) {
        return state != nullptr && band >= 0 && band < 5 &&
               ((state->selectedBands >> band) & 1u) != 0;
    }

    static void clearBandSelection(State* state) {
        if (!state) return;
        state->selectedBands = 0;
        state->selectedBand = -1;
    }

    static void selectOnlyBand(State* state, int band) {
        if (!state || band < 0 || band > 4) return;
        state->selectedBands = (uint8_t)(1u << band);
        state->selectedBand = band;
    }

    static void setParamOneShot(State* state, int param, float value) {
        if (!state || !state->ui || param < 0) return;
        state->ui->editParameter((uint32_t)param, true);
        state->ui->setParamValue((uint32_t)param, value);
        state->ui->editParameter((uint32_t)param, false);
        state->ui->updateParamLabel((uint32_t)param);
    }

    static void writeSelectedBandParam(State* state, BandParamKind kind, float value,
                                       bool fromUserGesture) {
        if (!state || !state->ui) return;
        for (int i = 0; i < 5; ++i) {
            if (!isBandSelected(state, i)) continue;
            if (i != state->selectedBand && !fromUserGesture) continue;
            const int p = bandParamForKind(state->baseParam, i, kind);
            if (p < 0) continue;
            state->ui->setParamValue((uint32_t)p, value);
            state->ui->updateParamLabel((uint32_t)p);
        }
    }

    static std::unordered_map<int, State*>& activeEditors() {
        static std::unordered_map<int, State*> s;
        return s;
    }

    // Mapping used by CymbalsUI when a modulation route is created: if the
    // target is a selected band's freq/gain/Q/sat control and several bands
    // are selected, return the corresponding parameter for every selected
    // band so the same source/depth can be routed to all of them.
    static int selectedModulationTargets(uint32_t target, uint32_t* out, int maxCount) {
        if (!out || maxCount <= 0) return 0;
        for (auto& kv : activeEditors()) {
            State* st = kv.second;
            if (!st) continue;
            const int base = st->baseParam;
            for (int band = 0; band < 5; ++band) {
                int kind = -1;
                for (int k = kBandFreq; k <= kBandSat; ++k) {
                    if (target == (uint32_t)bandParamForKind(base, band, (BandParamKind)k)) {
                        kind = k;
                        break;
                    }
                }
                if (kind < 0 || !isBandSelected(st, band)) continue;

                int count = 0;
                for (int i = 0; i < 5 && count < maxCount; ++i) {
                    if (!isBandSelected(st, i)) continue;
                    const int p = bandParamForKind(base, i, (BandParamKind)kind);
                    if (p >= 0) out[count++] = (uint32_t)p;
                }
                return count;
            }
        }
        return 0;
    }

    // Numbered band strip under Mix: mirrors the filter bank's per-stage
    // buttons and selects the same band the node hit-test does.
    static void syncBandButtons(State* state) {
        if (!state) return;
        for (int i = 0; i < 5; ++i) {
            lv_obj_t* b = state->bandButtons[i];
            if (!b) continue;
            if (isBandSelected(state, i)) lv_obj_add_state(b, LV_STATE_CHECKED);
            else                          lv_obj_clear_state(b, LV_STATE_CHECKED);
        }
    }

    // Keep the numbered strip starting under the Mix knob. The COLOUR EQ's
    // header carries a title before Mix, so a plain left-aligned row would sit
    // under the title instead.
    static void layout_band_row(State* state) {
        if (!state || !state->bandRow || !state->widgets) return;
        const uint32_t mixParam = (state->baseParam == PluginCymbals::kParamEQ1Freq)
            ? (uint32_t)PluginCymbals::kParamEQMix
            : (uint32_t)PluginCymbals::kParamFinalEQMix;
        lv_obj_t* mixArc = state->widgets[mixParam];
        if (!mixArc) return;
        lv_obj_t* mixCol = lv_obj_get_parent(lv_obj_get_parent(mixArc));
        lv_obj_t* mainCont = lv_obj_get_parent(state->bandRow);
        if (!mixCol || !mainCont) return;
        lv_area_t mixArea{}, mainArea{};
        lv_obj_get_coords(mixCol, &mixArea);
        lv_obj_get_coords(mainCont, &mainArea);
        lv_coord_t left = mixArea.x1 - mainArea.x1;
        if (left < 0) left = 0;
        lv_obj_set_style_pad_left(state->bandRow, left, 0);
    }

    static void nodeCenter(const State* state, int band, int w, int plotTop, int plotBottom,
                           int& px, int& py) {
        px = 0;
        py = plotTop;
        if (!state || !state->ui || band < 0 || band > 4 || w <= 0) return;
        const float fxLive = UIEQEditor::uiLiveValue(state->ui, (uint32_t)(state->baseParam + band * 4));
        px = (int)(fxLive * w);
        const int tfp = filterTypeParamForBand(state->baseParam, band);
        const float typeRaw = (tfp >= 0) ? state->ui->getParamValue(tfp) : 0.0f;
        const int typeIdx = std::clamp((int)std::round(typeRaw), 0, 7);
        const bool gainRelevant = (typeIdx >= 4 && typeIdx <= 6);
        const bool qRelevant = (typeIdx <= 3 || typeIdx == 7);
        if (gainRelevant) {
            const float fy = 1.0f - UIEQEditor::uiLiveValue(state->ui, (uint32_t)(state->baseParam + band * 4 + 1));
            py = plotTop + (int)(fy * (plotBottom - plotTop));
        } else if (qRelevant) {
            const float fy = 1.0f - UIEQEditor::uiLiveValue(state->ui, (uint32_t)(state->baseParam + band * 4 + 2));
            py = plotTop + (int)(fy * (plotBottom - plotTop));
        } else {
            py = plotTop + (plotBottom - plotTop) / 2;
        }
    }

    static void selectBand(State* state, int band) {
        if (!state || band < 0 || band > 4) return;
        finalize_classic_q_gesture(state, true);
        finalize_cut_arc_gestures(state);
        selectOnlyBand(state, band);
        state->marqueeActive = false;
        state->groupDragActive = false;
        const float currentQ = state->ui->getParamValue(state->baseParam + band * 4 + 2);
        state->qSmoothTarget = currentQ;
        state->qSmoothCurrent = currentQ;
        if (state->settingsPanel) lv_obj_remove_flag(state->settingsPanel, LV_OBJ_FLAG_HIDDEN);
        update_settings_ui(state);
        update_settings_width(state);
        syncBandButtons(state);
        if (state->canvas) lv_obj_invalidate(state->canvas);
    }

    static void bandButtonCb(lv_event_t* e) {
        State* state = (State*)lv_event_get_user_data(e);
        lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
        if (!state || !btn) return;
        selectBand(state, uiDecodeNonParamPayload((intptr_t)lv_obj_get_user_data(btn)));
    }

private:
    struct ArcDragState {
        lv_point_t startPoint{0, 0};
        int startValue = 0;
        bool active = false;
    };

    static std::unordered_map<lv_obj_t*, ArcDragState>& qDragStates() {
        static std::unordered_map<lv_obj_t*, ArcDragState> s;
        return s;
    }

    static void q_arc_drag_cb(lv_event_t* e) {
        lv_obj_t* arc = static_cast<lv_obj_t*>(lv_event_get_target(e));
        State* state = (State*)lv_event_get_user_data(e);
        const lv_event_code_t code = lv_event_get_code(e);

        if (code == LV_EVENT_DELETE) {
            qDragStates().erase(arc);
            return;
        }

        lv_indev_t* indev = lv_indev_get_act();
        if (!indev) return;

        if (code == LV_EVENT_PRESSED) {
            lv_point_t p;
            lv_indev_get_point(indev, &p);
            ArcDragState& st = qDragStates()[arc];
            st.startPoint = p;
            st.startValue = lv_arc_get_value(arc);
            st.active = true;

            begin_cut_arc_gesture(state, arc);
            lv_event_stop_bubbling(e);
            return;
        }

        if (code == LV_EVENT_PRESSING) {
            auto it = qDragStates().find(arc);
            if (it == qDragStates().end() || !it->second.active) return;

            lv_point_t p;
            lv_indev_get_point(indev, &p);
            const int dx = p.x - it->second.startPoint.x;
            const int dy = p.y - it->second.startPoint.y;
            const int value = lv_arc_get_value(arc);
            const int next = compactArcDragValueFromPressOrigin(it->second.startValue, dx, dy);

            if (next != value) {
                lv_arc_set_value(arc, next);
                lv_obj_send_event(arc, LV_EVENT_VALUE_CHANGED, state);
            }
            lv_event_stop_bubbling(e);
            return;
        }

        if (isArcReleaseEvent(code)) {
            auto it = qDragStates().find(arc);
            if (it != qDragStates().end()) it->second.active = false;

            // Deterministic finalize path: if drag processing is stopped on release,
            // explicitly close the classic-Q gesture here.
            if (state && arc == state->qArc) {
                finalize_classic_q_gesture(state, true);
            }
            finalize_cut_arc_gesture(state, arc);

            lv_event_stop_bubbling(e);
        }
    }

    static int cut_arc_param_for_target(const State* state, lv_obj_t* target) {
        if (!state || state->selectedBand < 0 || state->selectedBand > 4 || target == nullptr) return -1;
        if (target == state->poleArc) {
            return slopeParamForBand(state->baseParam, state->selectedBand);
        }
        if (target == state->rippleArc) {
            return qParamForBand(state->baseParam, state->selectedBand);
        }
        return -1;
    }

    static bool* cut_arc_active_flag_for_target(State* state, lv_obj_t* target) {
        if (!state || target == nullptr) return nullptr;
        if (target == state->poleArc) return &state->poleGestureActive;
        return nullptr;
    }

    static void begin_cut_arc_gesture(State* state, lv_obj_t* target) {
        if (!state || !state->ui) return;
        const int param = cut_arc_param_for_target(state, target);
        bool* active = cut_arc_active_flag_for_target(state, target);
        if (param < 0 || active == nullptr || *active) return;
        state->ui->editParameter((uint32_t)param, true);
        *active = true;
    }

    static void finalize_cut_arc_gesture(State* state, lv_obj_t* target) {
        if (!state || !state->ui) return;
        const int param = cut_arc_param_for_target(state, target);
        bool* active = cut_arc_active_flag_for_target(state, target);
        if (param < 0 || active == nullptr || !*active) return;
        state->ui->editParameter((uint32_t)param, false);
        *active = false;
    }

    static void finalize_cut_arc_gestures(State* state) {
        if (!state) return;
        finalize_cut_arc_gesture(state, state->poleArc);
        finalize_cut_arc_gesture(state, state->rippleArc);
    }

    static int polesFromMode(float v) {
        return std::clamp((int)std::round(v), 0, 3);
    }

    static int remapCutCharacterForPoleMode(int /*poleMode*/, int requestedCharacter) {
        return std::clamp(requestedCharacter, 0, 27);
    }

    static int cascadeCharacterFromParam(float v) {
        return std::clamp((int)std::round(v), 0, 27);
    }

    static float cascadeStageQForIndex(int numStages, int stageIndex, int character, float rippleDb, float masterQ = 0.70710678f) {
        const int stages = std::clamp(numStages, 1, 4);
        const int idx = std::clamp(stageIndex, 0, stages - 1);
        const int ch = std::clamp(character, 0, 6);
        const float safeQ = std::clamp(masterQ, 0.1f, 10.0f);

        const float butter2[4] = {0.70710678f, 0.0f, 0.0f, 0.0f};
        const float butter4[4] = {0.54119610f, 1.30656300f, 0.0f, 0.0f};
        const float butter6[4] = {0.51763809f, 0.70710678f, 1.93185170f, 0.0f};
        const float butter8[4] = {0.50979558f, 0.60097662f, 0.89997622f, 2.56320671f};

        const float bessel2[4] = {0.57735027f, 0.0f, 0.0f, 0.0f};
        const float bessel4[4] = {0.52193458f, 0.80553828f, 0.0f, 0.0f};
        const float bessel6[4] = {0.51031780f, 0.61119455f, 0.87738370f, 0.0f};
        const float bessel8[4] = {0.50600102f, 0.55960916f, 0.71085207f, 1.22566943f};

        const float lr2[4] = {0.70710678f, 0.0f, 0.0f, 0.0f};
        const float lr4[4] = {0.70710678f, 0.70710678f, 0.0f, 0.0f};
        const float lr6[4] = {0.70710678f, 0.70710678f, 0.70710678f, 0.0f};
        const float lr8[4] = {0.70710678f, 0.70710678f, 0.70710678f, 0.70710678f};

        const float* base = butter2;
        if (stages == 2) base = butter4;
        if (stages == 3) base = butter6;
        if (stages == 4) base = butter8;

        if (ch == 1) {
            base = (stages == 1) ? bessel2 : (stages == 2 ? bessel4 : (stages == 3 ? bessel6 : bessel8));
        } else if (ch == 2) {
            base = (stages == 1) ? lr2 : (stages == 2 ? lr4 : (stages == 3 ? lr6 : lr8));
        }

        const float qScale = safeQ / 0.70710678f;
        float stageQ = base[idx] * qScale;

        if (ch == 3) {
            const float rippleNorm = std::clamp((std::clamp(rippleDb, 0.1f, 10.0f) - 0.1f) / 9.9f, 0.0f, 1.0f);
            const float stageTilt = (stages <= 1) ? 0.0f : ((float)idx / (float)(stages - 1));
            const float shape = 1.0f + rippleNorm * (0.30f + 0.35f * stageTilt);
            stageQ = base[idx] * qScale * shape;
        }

        return std::clamp(stageQ, 0.1f, 10.0f);
    }

    static void update_settings_width(State* state) {
        if (!state || !state->settingsPanel) return;
        // Settings panel is inside topBar ROW flex — no manual alignment needed.
    }

    static void update_adv_res_label(State* state, float resonanceNorm) {
        (void)state;
        (void)resonanceNorm;
    }

    static void update_wasp_drive_label(State* state, float driveNorm) {
        if (!state || !state->waspDriveValueLabel) return;
        char buf[20];
        snprintf(buf, sizeof(buf), "%.2f", std::clamp(driveNorm, 0.0f, 1.0f));
        lv_label_set_text(state->waspDriveValueLabel, buf);
    }

    static void update_wasp_character_label(State* state, float characterNorm) {
        if (!state || !state->waspCharacterValueLabel) return;
        const float asymmetry = std::clamp(characterNorm, 0.0f, 1.0f) * 2.0f - 1.0f;
        char buf[20];
        snprintf(buf, sizeof(buf), "%+.2f", asymmetry);
        lv_label_set_text(state->waspCharacterValueLabel, buf);
    }

    static void update_cut_ripple_label(State* state, float rippleDb) {
        if (!state || !state->rippleValueLabel) return;
        char buf[20];
        snprintf(buf, sizeof(buf), "%.2f", std::clamp(rippleDb, 0.1f, 10.0f));
        lv_label_set_text(state->rippleValueLabel, buf);
    }

    static void update_cut_slope_label(State* state, int characterSelection, int poleMode) {
        if (!state || !state->poleValueLabel) return;
        lv_label_set_text(state->poleValueLabel, cutSlopeLabelForCharacterAndMode(characterSelection, poleMode));
    }

    static void dismiss_selection(State* state) {
        if (!state) return;
        finalize_classic_q_gesture(state, true);
        finalize_cut_arc_gestures(state);
        clearBandSelection(state);
        state->marqueeActive = false;
        state->groupDragActive = false;
        syncBandButtons(state);
        if (state->settingsPanel) lv_obj_add_flag(state->settingsPanel, LV_OBJ_FLAG_HIDDEN);
        if (state->canvas) lv_obj_invalidate(state->canvas);
    }

    static void outside_press_cb(lv_event_t* e) {
        State* state = (State*)lv_event_get_user_data(e);
        lv_obj_t* current = (lv_obj_t*)lv_event_get_current_target(e);
        lv_obj_t* target = (lv_obj_t*)lv_event_get_target(e);

        // Only dismiss when clicking the container background itself,
        // not when interacting with child controls like dropdown/arc.
        if (current == target) {
            dismiss_selection(state);
            lv_event_stop_bubbling(e);
        }
    }

    static int x_from_hz(float hz, int w) {
        float xn = std::clamp(std::log10(std::max(20.0f, hz) / 20.0f) / 3.0f, 0.0f, 1.0f);
        return (int)(xn * (w - 1));
    }

    static float dbMarkForIndex(const State* state, int index) {
        if (!state) return 0.0f;
        const float topDb = std::max(1.0f, std::fabs(state->plotTopDb));
        switch (index) {
            case 0: return topDb;
            case 1: return topDb * 0.5f;
            case 2: return 0.0f;
            case 3: return -topDb * 0.5f;
            case 4: return -topDb;
            default: return 0.0f;
        }
    }

    static void update_db_label_text(State* state, int index) {
        if (!state || index < 0 || index >= 5 || !state->dbLabels[index]) return;
        char buf[16];
        const int rounded = (int)std::lround(dbMarkForIndex(state, index));
        if (rounded > 0) snprintf(buf, sizeof(buf), "+%d", rounded);
        else snprintf(buf, sizeof(buf), "%d", rounded);
        lv_label_set_text(state->dbLabels[index], buf);
    }

    static void create_scale_labels(State* state) {
        static const char* kFreqText[10] = {"20 Hz", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20 kHz"};

        for (int i = 0; i < 10; ++i) {
            state->freqLabels[i] = lv_label_create(state->freqAxis ? state->freqAxis : state->canvas);
            lv_obj_set_style_text_font(state->freqLabels[i], &lv_font_montserrat_12, 0);
            lv_obj_set_style_text_color(state->freqLabels[i], COL_EQ_FREQ_LABEL, 0);
            lv_label_set_text(state->freqLabels[i], kFreqText[i]);
        }
        for (int i = 0; i < 5; ++i) {
            state->dbLabels[i] = lv_label_create(state->canvas);
            lv_obj_set_style_text_font(state->dbLabels[i], &lv_font_montserrat_12, 0);
            lv_obj_set_style_text_color(state->dbLabels[i], COL_EQ_DB_LABEL, 0);
            update_db_label_text(state, i);
        }
    }

    static void layout_scale_labels(State* state) {
        if (!state || !state->canvas) return;
        int w = lv_obj_get_width(state->canvas);
        int h = lv_obj_get_height(state->canvas);
        if (w <= 1 || h <= 1) return;
        int wa = state->freqAxis ? lv_obj_get_width(state->freqAxis) : w;
        if (wa < 8) wa = w;
        int ha = state->freqAxis ? lv_obj_get_height(state->freqAxis) : 20;
        const int plotTop = 16;
        const int plotBottom = std::max(plotTop + 20, h - 24);  // was h-36: freq labels live in the freqAxis strip below, the plot needs no label zone (matches hit-test floor)
        const float topDb = std::max(1.0f, std::fabs(state->plotTopDb));
        const float bottomDb = -topDb;

        auto y_from_db = [&](float db) {
            float t = std::clamp((topDb - db) / (topDb - bottomDb), 0.0f, 1.0f);
            return plotTop + (int)(t * (plotBottom - plotTop));
        };

        const float freqMarks[10] = {20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f};
        for (int i = 0; i < 10; ++i) {
            int x = x_from_hz(freqMarks[i], wa);
            const int lw = lv_obj_get_width(state->freqLabels[i]);
            lv_obj_set_pos(state->freqLabels[i], std::clamp(x - lw / 2, 0, std::max(0, wa - lw)), std::max(0, ha - 16));
            lv_obj_move_foreground(state->freqLabels[i]);
        }

        for (int i = 0; i < 5; ++i) {
            update_db_label_text(state, i);
            int y = std::clamp(y_from_db(dbMarkForIndex(state, i)) - 6, 0, std::max(0, h - 14));
            lv_obj_set_pos(state->dbLabels[i], 4, y);
            // The canvas paints its spectrum FILL in DRAW_POST, which runs
            // AFTER the canvas's children - so the dB labels were buried
            // under the fill ("the left axis dB labels are clipped ... the one
            // measurement axis on the page is unusable" - round-5 critic). The
            // frequency labels are moved to the foreground for exactly this
            // reason; the dB labels need the same treatment.
            lv_obj_move_foreground(state->dbLabels[i]);
        }
    }

    static void canvas_timer_cb(lv_timer_t* t) {
        State* state = (State*)lv_timer_get_user_data(t);
        if (!state) return;

        if (state->plotScaleDirty) {
            layout_scale_labels(state);
            state->plotScaleDirty = false;
        }
        
        // Apply Q smoothing if currently editing
        if (state->selectedBand >= 0 && state->selectedBand <= 4) {
            const int qIdx = state->baseParam + state->selectedBand * 4 + 2;
            const float currentValue = state->ui->getParamValue(qIdx);
            
            // Apply exponential moving average smoothing
            state->qSmoothCurrent = currentValue + (state->qSmoothTarget - currentValue) * state->kQSmoothFactor;
            
            // Only update if there's meaningful change to avoid unnecessary DSP updates
            if (std::fabs(state->qSmoothCurrent - currentValue) > 0.001f) {
                state->ui->setParamValue(qIdx, state->qSmoothCurrent);
            } else if (std::fabs(state->qSmoothTarget - currentValue) > 0.0001f) {
                // Force final value when close enough to target
                state->ui->setParamValue(qIdx, state->qSmoothTarget);
            }
            
            // Invalidate canvas to redraw curve
            if (state->canvas) lv_obj_invalidate(state->canvas);
        } else if (state->canvas) {
            lv_obj_invalidate(state->canvas);
        }
    }

    static void begin_classic_q_gesture(State* state, int qIdx) {
        if (!state || qIdx < 0) return;
        if (state->qGesture.active && state->qGesture.qParam != qIdx) {
            finalize_classic_q_gesture(state, true);
        }
        if (beginClassicQGesture(state->qGesture, qIdx)) {
            state->ui->editParameter((uint32_t)qIdx, true);
        }
    }

    static void finalize_classic_q_gesture(State* state, bool commitTarget) {
        if (!state) return;
        const int qIdx = state->qGesture.qParam;
        if (qIdx >= 0 && commitTarget) {
            state->ui->setParamValue((uint32_t)qIdx, state->qSmoothTarget);
            state->ui->updateParamLabel((uint32_t)qIdx);
        }
        if (qIdx >= 0 && endClassicQGesture(state->qGesture, -1)) {
            state->ui->editParameter((uint32_t)qIdx, false);
        }
        for (int i = 0; i < 5; ++i) {
            if (((state->qGestureBands >> i) & 1u) == 0) continue;
            const int p = qParamForBand(state->baseParam, i);
            if (p >= 0) state->ui->editParameter((uint32_t)p, false);
        }
        state->qGestureBands = 0;
    }

    static void q_arc_finalize_cb(lv_event_t* e) {
        State* state = (State*)lv_event_get_user_data(e);
        if (!state) return;
        const lv_event_code_t code = lv_event_get_code(e);
        if (isArcReleaseEvent(code)) {
            finalize_classic_q_gesture(state, true);
        } else if (code == LV_EVENT_DELETE) {
            finalize_classic_q_gesture(state, false);
        }
    }

    static void arc_release_cb(lv_event_t* e) {
        State* state = (State*)lv_event_get_user_data(e);
        if (!state) return;
        const lv_event_code_t code = lv_event_get_code(e);
        lv_obj_t* target = (lv_obj_t*)lv_event_get_target(e);
        if ((code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) && state->selectedBand >= 0) {
            if (target == state->freqArc && state->freqGestureBands != 0) {
                for (int i = 0; i < 5; ++i)
                    if ((state->freqGestureBands >> i) & 1u)
                        state->ui->editParameter((uint32_t)(state->baseParam + i * 4), false);
                state->freqGestureBands = 0;
            }
            if (target == state->gainArc && state->gainGestureBands != 0) {
                for (int i = 0; i < 5; ++i)
                    if ((state->gainGestureBands >> i) & 1u)
                        state->ui->editParameter((uint32_t)(state->baseParam + i * 4 + 1), false);
                state->gainGestureBands = 0;
            }
            if (target == state->satArc && state->satGestureBands != 0) {
                for (int i = 0; i < 5; ++i) {
                    if (((state->satGestureBands >> i) & 1u) == 0) continue;
                    const int satP = satParamForBand(state->baseParam, i);
                    if (satP >= 0) state->ui->editParameter((uint32_t)satP, false);
                }
                state->satGestureBands = 0;
            }
        }
    }

    static void canvas_event_cb(lv_event_t* e) {
        lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
        State* state = (State*)lv_event_get_user_data(e);
        lv_event_code_t code = lv_event_get_code(e);

        if (code == LV_EVENT_DRAW_MAIN) {
            draw_main(e);
        }
        else if (code == LV_EVENT_SIZE_CHANGED) {
            layout_scale_labels(state);
        }
        else if (code == LV_EVENT_SCROLL) {
            // Prevent parent tab from scrolling while user interacts with EQ editor.
            lv_event_stop_bubbling(e);
        }
        else if (code == LV_EVENT_PRESSED) {
            lv_event_stop_bubbling(e);
            lv_indev_t* indev = lv_indev_get_act();
            lv_point_t p; lv_indev_get_point(indev, &p);
            // Indev points are raw window pixels; the canvas is scaled by the
            // ancestor fit transform, so map back into unscaled local coords.
            const lv_point_t lp = uiObjLocalPoint(obj, p);

            const int w = lv_obj_get_width(obj), h = lv_obj_get_height(obj);
            const int plotTop = 16;
            const int plotBottom = std::max(plotTop + 20, h - 24);
            int hit = -1;
            for (int i = 0; i < 5; ++i) {
                int px = 0, py = 0;
                nodeCenter(state, i, w, plotTop, plotBottom, px, py);
                if (std::abs(lp.x - px) < 25 && std::abs(lp.y - py) < 25) { hit = i; break; }
            }

            finalize_classic_q_gesture(state, true);
            finalize_cut_arc_gestures(state);
            state->marqueeActive = false;
            state->groupDragActive = false;

            if (hit != -1) {
                if (!isBandSelected(state, hit)) selectOnlyBand(state, hit);
                else                             state->selectedBand = hit;
                state->groupDragActive = true;
                state->groupDragStart = lp;
                {
                    const int qIdx = state->baseParam + hit * 4 + 2;
                    const float currentQ = state->ui->getParamValue(qIdx);
                    state->qSmoothTarget = currentQ;
                    state->qSmoothCurrent = currentQ;
                }
                for (int i = 0; i < 5; ++i) {
                    if (!isBandSelected(state, i)) continue;
                    state->groupStartFreq[i] = state->ui->getParamValue((uint32_t)(state->baseParam + i * 4));
                    state->groupStartGain[i] = state->ui->getParamValue((uint32_t)(state->baseParam + i * 4 + 1));
                    state->groupStartQ[i] = state->ui->getParamValue((uint32_t)(state->baseParam + i * 4 + 2));
                    state->ui->editParameter((uint32_t)(state->baseParam + i * 4), true);
                    const int tfp = filterTypeParamForBand(state->baseParam, i);
                    const float typeRaw = (tfp >= 0) ? state->ui->getParamValue(tfp) : 0.0f;
                    const int typeIdx = std::clamp((int)std::round(typeRaw), 0, 7);
                    if (typeIdx >= 4 && typeIdx <= 6)
                        state->ui->editParameter((uint32_t)(state->baseParam + i * 4 + 1), true);
                }
                lv_obj_remove_flag(state->settingsPanel, LV_OBJ_FLAG_HIDDEN);
            } else {
                state->marqueeActive = true;
                state->marqueeStart = lp;
                state->marqueeCurrent = lp;
                clearBandSelection(state);
                lv_obj_add_flag(state->settingsPanel, LV_OBJ_FLAG_HIDDEN);
            }
            syncBandButtons(state);
            lv_obj_invalidate(obj);
        }
        else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
            if (state->marqueeActive) {
                const int x1 = std::min(state->marqueeStart.x, state->marqueeCurrent.x);
                const int x2 = std::max(state->marqueeStart.x, state->marqueeCurrent.x);
                const int y1 = std::min(state->marqueeStart.y, state->marqueeCurrent.y);
                const int y2 = std::max(state->marqueeStart.y, state->marqueeCurrent.y);
                uint8_t mask = 0;
                int primary = -1;
                const int w = lv_obj_get_width(obj), h = lv_obj_get_height(obj);
                const int plotTop = 16;
                const int plotBottom = std::max(plotTop + 20, h - 24);
                for (int i = 0; i < 5; ++i) {
                    int px = 0, py = 0;
                    nodeCenter(state, i, w, plotTop, plotBottom, px, py);
                    if (px >= x1 && px <= x2 && py >= y1 && py <= y2) {
                        mask |= (uint8_t)(1u << i);
                        primary = i;
                    }
                }
                state->marqueeActive = false;
                state->selectedBands = mask;
                state->selectedBand = primary;
                if (primary >= 0) {
                    const float q = state->ui->getParamValue((uint32_t)(state->baseParam + primary * 4 + 2));
                    state->qSmoothTarget = q;
                    state->qSmoothCurrent = q;
                    lv_obj_remove_flag(state->settingsPanel, LV_OBJ_FLAG_HIDDEN);
                    update_settings_ui(state);
                    update_settings_width(state);
                } else {
                    lv_obj_add_flag(state->settingsPanel, LV_OBJ_FLAG_HIDDEN);
                }
                syncBandButtons(state);
                lv_obj_invalidate(obj);
                lv_event_stop_bubbling(e);
                return;
            }

            if (state->groupDragActive) {
                for (int i = 0; i < 5; ++i) {
                    if (!isBandSelected(state, i)) continue;
                    state->ui->editParameter((uint32_t)(state->baseParam + i * 4), false);
                    const int tfp = filterTypeParamForBand(state->baseParam, i);
                    const float typeRaw = (tfp >= 0) ? state->ui->getParamValue(tfp) : 0.0f;
                    const int typeIdx = std::clamp((int)std::round(typeRaw), 0, 7);
                    if (typeIdx >= 4 && typeIdx <= 6)
                        state->ui->editParameter((uint32_t)(state->baseParam + i * 4 + 1), false);
                }
                state->groupDragActive = false;
            }
            finalize_classic_q_gesture(state, true);
            lv_event_stop_bubbling(e);
        }
        else if (code == LV_EVENT_PRESSING) {
            // Everything below runs PER POINTER MOVE, so every arc write here
            // (and in update_settings_ui, which this calls) uses the INSTANT
            // setter. The animated one restarts a 200ms sweep on each move, so
            // dragging a band would make the freq/gain/Q knobs trail the
            // cursor and never settle. Animate only discrete jumps (preset
            // load, host sync) - never a streaming gesture.
            if (state->marqueeActive) {
                lv_event_stop_bubbling(e);
                lv_indev_t* indev = lv_indev_get_act();
                lv_point_t p; lv_indev_get_point(indev, &p);
                state->marqueeCurrent = uiObjLocalPoint(obj, p);
                lv_obj_invalidate(obj);
                return;
            }
            if (!state->groupDragActive || state->selectedBands == 0) return;

            lv_event_stop_bubbling(e);
            lv_indev_t* indev = lv_indev_get_act();
            lv_point_t p; lv_indev_get_point(indev, &p);
            const lv_point_t lp = uiObjLocalPoint(obj, p);
            const int w = lv_obj_get_width(obj), h = lv_obj_get_height(obj);
            if (w <= 0 || h <= 0) return;
            const int plotTop = 16;
            const int plotBottom = std::max(plotTop + 20, h - 24);
            const float plotH = (float)std::max(1, plotBottom - plotTop);
            const float dx = (float)(lp.x - state->groupDragStart.x) / (float)w;
            const float dy = (float)(state->groupDragStart.y - lp.y) / plotH;

            for (int i = 0; i < 5; ++i) {
                if (!isBandSelected(state, i)) continue;
                const int freqP = state->baseParam + i * 4;
                const float freq = std::clamp(state->groupStartFreq[i] + dx, 0.0f, 1.0f);
                state->ui->setParamValue((uint32_t)freqP, freq);
                state->ui->updateParamLabel((uint32_t)freqP);
                if (i == state->selectedBand && state->freqArc)
                    UIWidgets::setArcValueInstant(state->freqArc, (int)(freq * 1000.0f));

                const int tfp = filterTypeParamForBand(state->baseParam, i);
                const float typeRaw = (tfp >= 0) ? state->ui->getParamValue(tfp) : 0.0f;
                const int typeIdx = std::clamp((int)std::round(typeRaw), 0, 7);
                const bool gainRelevant = (typeIdx >= 4 && typeIdx <= 6);
                const bool qRelevant = (typeIdx <= 3 || typeIdx == 7);
                if (gainRelevant) {
                    const float gain = std::clamp(state->groupStartGain[i] + dy, 0.0f, 1.0f);
                    state->ui->setParamValue((uint32_t)(freqP + 1), gain);
                    state->ui->updateParamLabel((uint32_t)(freqP + 1));
                    if (i == state->selectedBand && state->gainArc)
                        UIWidgets::setArcValueInstant(state->gainArc, (int)(gain * 1000.0f));
                } else if (qRelevant) {
                    const float q = std::clamp(state->groupStartQ[i] + dy, 0.0f, 1.0f);
                    state->ui->setParamValue((uint32_t)(freqP + 2), q);
                    state->ui->updateParamLabel((uint32_t)(freqP + 2));
                    if (i == state->selectedBand) {
                        state->qSmoothTarget = q;
                        state->qSmoothCurrent = q;
                        if (state->qArc)
                            UIWidgets::setArcValueInstant(state->qArc, (int)(q * 1000.0f));
                    }
                }
            }

            // Keep every settings knob arc + value label in sync with the
            // dragged chart point (freq/gain/Q/sat values under the knobs).
            update_settings_ui(state);
            lv_obj_invalidate(obj);
        }
    }

    static void settings_changed_cb(lv_event_t* e) {
        State* state = (State*)lv_event_get_user_data(e);
        if (!state || state->selectedBand == -1) return;
        // parameterModulationChanged() calls syncParamWidget(base + mod),
        // which sends VALUE_CHANGED while fInSync is set. Writing that value
        // back here would move the base toward the modulated display value.
        if (state->ui && state->ui->isInSync()) return;

        lv_obj_t* target = (lv_obj_t*)lv_event_get_target(e);
        // Host-driven writes (automation/preset load) reach this callback via
        // syncParamWidget's synthetic VALUE_CHANGED; only a real pointer drag
        // holds LV_STATE_PRESSED. The gesture-begin paths below must not fire
        // on synthetic events: they open an editParameter() session that only a
        // real RELEASE would close (unbalanced begin/end for the host).
        const bool fromUserGesture = (lv_obj_get_state(target) & LV_STATE_PRESSED) != 0;
        const int sb = state->selectedBand;
        const int modelP = modelParamForBand(state->baseParam, sb);
        const int filterTypeP = filterTypeParamForBand(state->baseParam, sb);
        const int slopeP = slopeParamForBand(state->baseParam, sb);
        const int qP = qParamForBand(state->baseParam, sb);

        if (target == state->qArc && qP >= 0) {
            const float qNorm = (float)lv_arc_get_value(state->qArc) / 1000.0f;
            state->qSmoothTarget = qNorm;
            if (state->qValueLabel) {
                char buf[16];
                snprintf(buf, sizeof(buf), "Q %.2f", DSPUtils::qFromParam(qNorm));
                lv_label_set_text(state->qValueLabel, buf);
            }
            if (fromUserGesture && !state->qGesture.active) {
                for (int i = 0; i < 5; ++i) {
                    if (i == sb || !isBandSelected(state, i)) continue;
                    const int p = bandParamForKind(state->baseParam, i, kBandQ);
                    if (p >= 0) state->ui->editParameter((uint32_t)p, true);
                }
                state->qGestureBands = (uint8_t)(state->selectedBands & ~(1u << sb));
                begin_classic_q_gesture(state, qP);
            }
            writeSelectedBandParam(state, kBandQ, qNorm, fromUserGesture);
        } else if (target == state->freqArc) {
            const float freqNorm = (float)lv_arc_get_value(state->freqArc) / 1000.0f;
            if (fromUserGesture && state->freqGestureBands == 0) {
                for (int i = 0; i < 5; ++i) {
                    if (!isBandSelected(state, i)) continue;
                    const int p = bandParamForKind(state->baseParam, i, kBandFreq);
                    if (p >= 0) state->ui->editParameter((uint32_t)p, true);
                }
                state->freqGestureBands = state->selectedBands;
            }
            writeSelectedBandParam(state, kBandFreq, freqNorm, fromUserGesture);
            if (state->freqValueLabel) {
                const float freqHz = DSPUtils::normToHz(freqNorm);
                char buf[16];
                if (freqHz >= 1000.0f)
                    snprintf(buf, sizeof(buf), "%.2f kHz", freqHz / 1000.0f);
                else
                    snprintf(buf, sizeof(buf), "%.0f Hz", freqHz);
                lv_label_set_text(state->freqValueLabel, buf);
            }
        } else if (target == state->gainArc) {
            const float gainNorm = (float)lv_arc_get_value(state->gainArc) / 1000.0f;
            if (fromUserGesture && state->gainGestureBands == 0) {
                for (int i = 0; i < 5; ++i) {
                    if (!isBandSelected(state, i)) continue;
                    const int p = bandParamForKind(state->baseParam, i, kBandGain);
                    if (p >= 0) state->ui->editParameter((uint32_t)p, true);
                }
                state->gainGestureBands = state->selectedBands;
            }
            writeSelectedBandParam(state, kBandGain, gainNorm, fromUserGesture);
            if (state->gainValueLabel) {
                const float gainDb = (gainNorm - 0.5f) * 48.0f;
                char buf[16];
                snprintf(buf, sizeof(buf), "%+.1f dB", gainDb);
                lv_label_set_text(state->gainValueLabel, buf);
            }
        } else if (target == state->satArc) {
            const float satNorm = (float)lv_arc_get_value(state->satArc) / 1000.0f;
            if (fromUserGesture && state->satGestureBands == 0) {
                for (int i = 0; i < 5; ++i) {
                    if (!isBandSelected(state, i)) continue;
                    const int p = bandParamForKind(state->baseParam, i, kBandSat);
                    if (p >= 0) state->ui->editParameter((uint32_t)p, true);
                }
                state->satGestureBands = state->selectedBands;
            }
            writeSelectedBandParam(state, kBandSat, satNorm, fromUserGesture);
            if (state->satValueLabel) {
                char buf[16];
                snprintf(buf, sizeof(buf), "Sat %.0f%%", satNorm * 100.0f);
                lv_label_set_text(state->satValueLabel, buf);
            }
        } else if (target == state->typeArc && filterTypeP >= 0) {
            const int arcVal = lv_arc_get_value(state->typeArc);
            const int modelForType = (modelP >= 0) ? std::clamp((int)std::round(state->ui->getParamValue(modelP)), 0, (int)UnifiedFilter::Count - 1) : 0;
            const int snapped = arcValueToType(arcVal, modelForType);
            const int snappedArc = typeToArcValue(snapped, modelForType);
            if (snappedArc != arcVal)
                UIWidgets::setArcValueInstant(state->typeArc, snappedArc);

            for (int i = 0; i < 5; ++i) {
                if (!isBandSelected(state, i)) continue;
                if (i != sb && !fromUserGesture) continue;
                const int tp = filterTypeParamForBand(state->baseParam, i);
                if (tp < 0) continue;
                const int mp = modelParamForBand(state->baseParam, i);
                const int model = (mp >= 0)
                    ? std::clamp((int)std::round(state->ui->getParamValue(mp)), 0, (int)UnifiedFilter::Count - 1)
                    : 0;
                const int type = arcValueToType(arcVal, model);
                setParamOneShot(state, tp, (float)type);
                if (!(type >= 4 && type <= 6))
                    setParamOneShot(state, state->baseParam + i * 4 + 1, 0.5f);
            }
            update_settings_ui(state);
            if (state->canvas) lv_obj_invalidate(state->canvas);
        } else if (state->charDropdown && target == state->charDropdown && modelP >= 0) {
            const int newModel = modelDropdownToValue(lv_dropdown_get_selected(state->charDropdown));
            for (int i = 0; i < 5; ++i) {
                if (!isBandSelected(state, i)) continue;
                const int mp = modelParamForBand(state->baseParam, i);
                const int tp = filterTypeParamForBand(state->baseParam, i);
                if (mp < 0) continue;
                setParamOneShot(state, mp, (float)newModel);
                if (tp >= 0) {
                    const int curType = std::clamp((int)std::round(state->ui->getParamValue(tp)), 0, 7);
                    const int snapped = snapTypeToValid(curType, newModel);
                    if (snapped != curType)
                        setParamOneShot(state, tp, (float)snapped);
                }
            }
            update_settings_ui(state);
            if (state->canvas) lv_obj_invalidate(state->canvas);
        } else if (target == state->poleArc && slopeP >= 0 && state->poleArc) {
            const int poleMode = cutSlopeModeFromArcValue(lv_arc_get_value(state->poleArc));
            const int snappedArcValue = cutSlopeArcValueFromMode(poleMode);
            if (lv_arc_get_value(state->poleArc) != snappedArcValue) {
                UIWidgets::setArcValueInstant(state->poleArc, snappedArcValue);
            }
            state->ui->setParamValue(slopeP, (float)poleMode);
            state->ui->updateParamLabel(slopeP);
            if (state->poleLabel) lv_label_set_text(state->poleLabel, "Slope");
            update_cut_slope_label(state,
                std::max(0, (int)std::round(state->ui->getParamValue(modelP)) - 1), poleMode);
        } else if (target == state->filterTypeDropdown && filterTypeP >= 0 && state->filterTypeDropdown) {
            const int selection = (int)lv_dropdown_get_selected(state->filterTypeDropdown);
            for (int i = 0; i < 5; ++i) {
                if (!isBandSelected(state, i)) continue;
                const int tp = filterTypeParamForBand(state->baseParam, i);
                if (tp < 0) continue;
                const int mp = modelParamForBand(state->baseParam, i);
                const int model = (mp >= 0)
                    ? std::clamp((int)std::round(state->ui->getParamValue(mp)), 0, (int)UnifiedFilter::Count - 1)
                    : 0;
                const int type = snapTypeToValid(selection, model);
                setParamOneShot(state, tp, (float)type);
            }
            update_settings_ui(state);
            if (state->canvas) lv_obj_invalidate(state->canvas);
        } else if (target == state->waspDriveArc) {
            const int driveParam = advancedDriveParamForBand(state->baseParam, sb);
            if (driveParam >= 0 && state->waspDriveArc) {
                const float driveNorm = (float)lv_arc_get_value(state->waspDriveArc) / 1000.0f;
                state->ui->editParameter(driveParam, true);
                state->ui->setParamValue(driveParam, driveNorm);
                state->ui->editParameter(driveParam, false);
                state->ui->updateParamLabel(driveParam);
                update_wasp_drive_label(state, driveNorm);
            }
        } else if (target == state->waspCharacterArc) {
            const int characterParam = advancedCharacterParamForBand(state->baseParam, sb);
            if (characterParam >= 0 && state->waspCharacterArc) {
                const float characterNorm = (float)lv_arc_get_value(state->waspCharacterArc) / 1000.0f;
                state->ui->editParameter(characterParam, true);
                state->ui->setParamValue(characterParam, characterNorm);
                state->ui->editParameter(characterParam, false);
                state->ui->updateParamLabel(characterParam);
                update_wasp_character_label(state, characterNorm);
            }
        }
        if (state->canvas) lv_obj_invalidate(state->canvas);
    }

    static void update_settings_ui(State* state) {
        if (!state) return;

        // Hide old ripple/stopband containers (always hidden in unified 5-node system)
        if (state->rippleCont) lv_obj_add_flag(state->rippleCont, LV_OBJ_FLAG_HIDDEN);
        if (state->stopbandCont) lv_obj_add_flag(state->stopbandCont, LV_OBJ_FLAG_HIDDEN);
        if (state->charHelpLabel) {
            lv_label_set_text(state->charHelpLabel, "");
            lv_obj_add_flag(state->charHelpLabel, LV_OBJ_FLAG_HIDDEN);
        }

        // === ALL 5 NODES IDENTICAL: Model + Type + Q ===
        const int sb = state->selectedBand;
        const int modelParam = modelParamForBand(state->baseParam, sb);
        const int filterTypeP = filterTypeParamForBand(state->baseParam, sb);

        // When model is None: hide all filter controls, keep only model dropdown
        const bool noneModel = (modelParam >= 0) && ((int)std::round(state->ui->getParamValue(modelParam)) == 28);
        if (noneModel) {
            if (state->typeCont) lv_obj_add_flag(state->typeCont, LV_OBJ_FLAG_HIDDEN);
            if (state->poleCont) lv_obj_add_flag(state->poleCont, LV_OBJ_FLAG_HIDDEN);
            if (state->qCont) lv_obj_add_flag(state->qCont, LV_OBJ_FLAG_HIDDEN);
            if (state->gainCont) lv_obj_add_flag(state->gainCont, LV_OBJ_FLAG_HIDDEN);
            if (state->satCont) lv_obj_add_flag(state->satCont, LV_OBJ_FLAG_HIDDEN);
            if (state->freqCont) lv_obj_add_flag(state->freqCont, LV_OBJ_FLAG_HIDDEN);
            if (state->charCont) lv_obj_remove_flag(state->charCont, LV_OBJ_FLAG_HIDDEN);
            if (state->charDropdown) {
                lv_dropdown_set_options(state->charDropdown, eqBandModelDropdownOptions());
                lv_dropdown_set_selected(state->charDropdown, (uint16_t)modelValueToDropdown((int)UnifiedFilter::NoneModel));
            }
            return;
        }
        const int slopeParam = slopeParamForBand(state->baseParam, sb);
        const int qP = qParamForBand(state->baseParam, sb);
        const int modelSel = (modelParam >= 0)
            ? std::clamp((int)std::round(state->ui->getParamValue(modelParam)), 0, (int)UnifiedFilter::Count - 1) : 0;

        // Model dropdown visible for all nodes
        if (state->charCont) lv_obj_remove_flag(state->charCont, LV_OBJ_FLAG_HIDDEN);
        if (state->charDropdown) {
            lv_dropdown_set_options(state->charDropdown, eqBandModelDropdownOptions());
            lv_dropdown_set_selected(state->charDropdown, (uint16_t)modelValueToDropdown(modelSel));
        }

        // Type dropdown visible for all nodes
        if (state->typeCont) lv_obj_remove_flag(state->typeCont, LV_OBJ_FLAG_HIDDEN);
        // Slope arc visible only for types 0-3 (LP/BP/HP/Notch) on models whose
        // DSP actually uses slope (hidden for Formant/Bode/ZPlane and types 4-7)
        {
            const int currentType = (filterTypeP >= 0)
                ? std::clamp((int)std::round(state->ui->getParamValue(filterTypeP)), 0, 7) : 0;
            const bool slopeRelevant = UnifiedFilter::slopeIsRelevant(modelSel);
            if (state->poleCont) {
                if (currentType <= 3 && slopeRelevant) lv_obj_remove_flag(state->poleCont, LV_OBJ_FLAG_HIDDEN);
                else lv_obj_add_flag(state->poleCont, LV_OBJ_FLAG_HIDDEN);
            }
        }
        // filterTypeDropdown / stopbandCont is redundant — always hidden
        if (state->stopbandCont) lv_obj_add_flag(state->stopbandCont, LV_OBJ_FLAG_HIDDEN);
        // Q arc always visible
        if (state->qCont) lv_obj_remove_flag(state->qCont, LV_OBJ_FLAG_HIDDEN);
        // Freq arc always visible
        if (state->freqCont) lv_obj_remove_flag(state->freqCont, LV_OBJ_FLAG_HIDDEN);
        // Sat arc always visible
        if (state->satCont) lv_obj_remove_flag(state->satCont, LV_OBJ_FLAG_HIDDEN);

        // Sync type arc (maps only to valid types for current model)
        if (filterTypeP >= 0 && state->typeArc) {
            const int selType = std::clamp((int)std::round(state->ui->getParamValue(filterTypeP)), 0, 7);
            UIWidgets::setArcValueInstant(state->typeArc, typeToArcValue(selType, modelSel));
            lv_obj_invalidate(state->typeArc);
            if (state->typeValueLabel) {
                lv_label_set_text(state->typeValueLabel, UnifiedFilter::filterTypeNameForModel(modelSel, selType));
            }
        }

        // Sync slope arc (for model > 0 = UnifiedFilter)
        if (slopeParam >= 0 && state->poleArc) {
            if (state->poleLabel) lv_label_set_text(state->poleLabel, "Slope");
            const int slopeMode = std::clamp((int)std::round(state->ui->getParamValue(slopeParam)), 0, 3);
            UIWidgets::setArcValueInstant(state->poleArc, cutSlopeArcValueFromMode(slopeMode));
            lv_obj_invalidate(state->poleArc);
            update_cut_slope_label(state, modelSel - 1, slopeMode);
        }

        // Sync filter type dropdown (for model > 0 = UnifiedFilter)
        if (filterTypeP >= 0 && state->filterTypeDropdown) {
            const int ftSel = std::clamp((int)std::round(state->ui->getParamValue(filterTypeP)), 0, 7);
            lv_dropdown_set_selected(state->filterTypeDropdown, (uint16_t)ftSel);
        }

        // Sync Q arc (always visible)
        if (qP >= 0 && state->qArc) {
            const float qNorm = readQNorm(state, qP);
            UIWidgets::setArcValueInstant(state->qArc, (int)(qNorm * 1000.0f));
            lv_obj_invalidate(state->qArc);
            if (state->qValueLabel) {
                char buf[16];
                snprintf(buf, sizeof(buf), "Q %.2f", DSPUtils::qFromParam(qNorm));
                lv_label_set_text(state->qValueLabel, buf);
            }
        }

        // Sync freq arc value (always visible)
        {
            const int freqP = state->baseParam + sb * 4;
            if (freqP >= 0 && state->freqArc) {
                const float freqNorm = state->ui->getParamValue(freqP);
                UIWidgets::setArcValueInstant(state->freqArc, (int)(freqNorm * 1000.0f));
                lv_obj_invalidate(state->freqArc);
                if (state->freqValueLabel) {
                    const float freqHz = DSPUtils::normToHz( freqNorm);
                    char buf[16];
                    if (freqHz >= 1000.0f)
                        snprintf(buf, sizeof(buf), "%.2f kHz", freqHz / 1000.0f);
                    else
                        snprintf(buf, sizeof(buf), "%.0f Hz", freqHz);
                    lv_label_set_text(state->freqValueLabel, buf);
                }
            }
        }

        // Gain arc visible only for Bell/LowShelf/HighShelf (types 4-6)
        {
            const int currentType = (filterTypeP >= 0)
                ? std::clamp((int)std::round(state->ui->getParamValue(filterTypeP)), 0, 7) : 0;
            if (state->gainCont) {
                if (currentType >= 4 && currentType <= 6)
                    lv_obj_remove_flag(state->gainCont, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_add_flag(state->gainCont, LV_OBJ_FLAG_HIDDEN);
            }
        }
        // Sync gain arc value
        {
            const int gainP = state->baseParam + sb * 4 + 1;
            if (gainP >= 0 && state->gainArc) {
                const float gainNorm = state->ui->getParamValue(gainP);
                UIWidgets::setArcValueInstant(state->gainArc, (int)(gainNorm * 1000.0f));
                lv_obj_invalidate(state->gainArc);
                if (state->gainValueLabel) {
                    const float gainDb = (gainNorm - 0.5f) * 48.0f;
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%+.1f dB", gainDb);
                    lv_label_set_text(state->gainValueLabel, buf);
                }
            }
        }

        // Sync sat arc value
        {
            const int satP = satParamForBand(state->baseParam, sb);
            if (satP >= 0 && state->satArc) {
                const float satNorm = state->ui->getParamValue(satP);
                UIWidgets::setArcValueInstant(state->satArc, (int)(satNorm * 1000.0f));
                lv_obj_invalidate(state->satArc);
                if (state->satValueLabel) {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "Sat %.0f%%", satNorm * 100.0f);
                    lv_label_set_text(state->satValueLabel, buf);
                }
            }
        }

        // Wasp advanced controls are removed — all nodes use UnifiedFilter
        if (state->waspDriveCont) lv_obj_add_flag(state->waspDriveCont, LV_OBJ_FLAG_HIDDEN);
        if (state->waspCharacterCont) lv_obj_add_flag(state->waspCharacterCont, LV_OBJ_FLAG_HIDDEN);

        registerModArcs(state);
    }

    // Mod-target registration: the editor edits ONE selected band at a time, so
    // its freq/gain/q/sat arcs must (re)bind to the selected band's parameters.
    // Called at the end of update_settings_ui (the single band-switch funnel).
    // Only params inside kModTargetClasses are registered (Freq/Gain/Q/Sat);
    // Type/Pole/Model/Slope arcs are discrete and stay unregistered.
    static void registerModArcs(State* st) {
        if (!st || !st->widgets) return;
        for (int k = 0; k < 4; ++k) {
            if (st->modParams[k] && st->modParams[k] < PluginCymbals::kParameterCount)
                st->widgets[st->modParams[k]] = nullptr;
            st->modParams[k] = 0;
        }
        const int band = st->selectedBand;
        if (band < 0 || band > 4) return;
        struct { lv_obj_t* arc; int param; } m[4] = {
            { st->freqArc, st->baseParam + band * 4 + 0 },
            { st->gainArc, st->baseParam + band * 4 + 1 },
            { st->qArc,    st->baseParam + band * 4 + 2 },
            { st->satArc,  satParamForBand(st->baseParam, band) },
        };
        for (int k = 0; k < 4; ++k) {
            if (!m[k].arc || m[k].param < 0) continue;
            lv_obj_set_user_data(m[k].arc, (void*)(intptr_t)m[k].param);
            st->widgets[m[k].param] = m[k].arc;
            st->modParams[k] = (uint32_t)m[k].param;
        }
    }

    static void draw_main(lv_event_t* e) {
        lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
        State* state = (State*)lv_event_get_user_data(e);
        lv_layer_t* layer = lv_event_get_layer(e);
        lv_area_t coords; lv_obj_get_coords(obj, &coords);
        int w = lv_obj_get_width(obj), h = lv_obj_get_height(obj);
        if (w <= 1 || h <= 1) {
            return;
        }

        // Dark background overlay for stronger contrast against grid + curves
        lv_draw_rect_dsc_t bg_dsc; lv_draw_rect_dsc_init(&bg_dsc);
        bg_dsc.bg_color = COL_EQ_PLOT_BG;
        bg_dsc.bg_opa = LV_OPA_COVER;
        lv_draw_rect(layer, &bg_dsc, &coords);
        const int plotTop = 16;
        const int plotBottom = std::max(plotTop + 20, h - 24);  // trimmed with layout_scale_labels (see above)
        auto x_from_hz_local = [&](float hz) { return x_from_hz(hz, w); };
        auto fmt_hz = [](float hz, char* out, size_t n) {
            if (hz >= 1000.0f) {
                if (hz >= 9950.0f) snprintf(out, n, "%.0f kHz", hz / 1000.0f);
                else snprintf(out, n, "%.2f kHz", hz / 1000.0f);
            } else {
                snprintf(out, n, "%.0f Hz", hz);
            }
        };

        const float sampleRate = resolveSampleRate(state ? state->ui : nullptr);
        const float maxPlotHz = std::min(20000.0f, sampleRate * 0.49f);
        const float clampedMaxPlotHz = std::max(20.0f, maxPlotHz);
        const int previewSamples = std::max(48, std::min(160, w / 3));

        // Preview filters are cached per editor instance (allocated in create());
        // only their coefficients are re-derived here, so this stays allocation-free.
        UnifiedFilter* previewFilters = state->previewFilters.get();
        if (!previewFilters) return;
        for (int i = 0; i < 5; ++i) {
            const float f0 = DSPUtils::normToHz( UIEQEditor::uiLiveValue(state->ui, (uint32_t)(state->baseParam + i * 4)));
            const float gainDb = (UIEQEditor::uiLiveValue(state->ui, (uint32_t)(state->baseParam + i * 4 + 1)) - 0.5f) * 48.0f;
            const float Q = DSPUtils::qFromParam(UIEQEditor::uiLiveValue(state->ui, (uint32_t)(state->baseParam + i * 4 + 2)));
            const int mp = modelParamForBand(state->baseParam, i);
            const int fp = filterTypeParamForBand(state->baseParam, i);
            const int sp = slopeParamForBand(state->baseParam, i);
            const int model = (mp >= 0) ? std::clamp((int)std::round(state->ui->getParamValue(mp)), 0, (int)UnifiedFilter::Count - 1) : 0;
            const int type = (fp >= 0) ? std::clamp((int)std::round(state->ui->getParamValue(fp)), 0, 7) : 0;
            const int slope = (sp >= 0) ? std::clamp((int)std::round(state->ui->getParamValue(sp)), 0, 3) : 0;
            const float previewGain = (type >= 4 && type <= 6) ? gainDb : 0.0f;
            previewFilters[i].setSampleRate(sampleRate);
            previewFilters[i].setModel(model);
            previewFilters[i].setType(type);
            previewFilters[i].setSlope(slope);
            previewFilters[i].setCutoffHz(std::clamp(f0, 20.0f, std::max(20.0f, sampleRate * 0.49f)));
            previewFilters[i].setResonance(std::clamp(Q, 0.1f, 10.0f));
            previewFilters[i].setGainDb(previewGain);
            {
                const int satP = satParamForBand(state->baseParam, i);
                if (satP >= 0)
                    previewFilters[i].setSaturation(std::clamp(state->ui->getParamValue(satP), 0.0f, 1.0f));
            }
            previewFilters[i].prepare();
        }

        // Single-band contribution (dB) at normalized x — the per-band
        // response curves of a proper EQ display are drawn from these.
        // Saturation mirrors the DSP pre-filter drive (x *= 1 + sat*3, up
        // to +12 dB) so the saturated node's level is visible here too.
        auto band_db_at = [&](float x_norm, int band) {
            const float freqHz = 20.0f * std::pow(clampedMaxPlotHz / 20.0f, x_norm);
            const float safeFreqHz = std::clamp(freqHz, 20.0f, std::max(20.0f, sampleRate * 0.49f));

            float mag = previewFilters[band].getMagnitudeForFrequency(safeFreqHz);
            if (mag < 1.0e-6f) mag = 1.0e-6f;
            const int satP = satParamForBand(state->baseParam, band);
            if (satP >= 0 && previewFilters[band].model() != UnifiedFilter::NoneModel) {
                const float sat = std::clamp(state->ui->getParamValue(satP), 0.0f, 1.0f);
                if (sat > 0.0f) mag *= 1.0f + sat * 3.0f;
            }
            return 20.0f * std::log10(mag);
        };

        // Tilt shelf pair, coefficients derived ONCE per draw — the 1px-hot
        // response loop must not rebuild biquads (setParams does the whole
        // coefficient solve; doing it per pixel cost seconds of CPU per frame
        // at 1920-wide canvases).
        DSPUtils::Biquad tiltLowShelf;
        DSPUtils::Biquad tiltHighShelf;
        const bool tiltActive = (state->tiltAmountParam >= 0 && state->tiltPivotParam >= 0);
        if (tiltActive) {
            const float tiltAmt = std::clamp(state->ui->getParamValue(state->tiltAmountParam), 0.0f, 1.0f);
            const float tiltPivot = std::clamp(state->ui->getParamValue(state->tiltPivotParam), 0.0f, 1.0f);
            const float pivotHz = DSPUtils::normToHz( tiltPivot);
            const float tiltDb = DSPUtils::tiltDbFromParam(tiltAmt);
            tiltLowShelf.setParams(DSPUtils::Biquad::LOW_SHELF, pivotHz, -tiltDb, 0.707f, sampleRate);
            tiltHighShelf.setParams(DSPUtils::Biquad::HIGH_SHELF, pivotHz, tiltDb, 0.707f, sampleRate);
        }

        // Tilt contributes to the summed curve only (separate shelf pair,
        // not part of any single band's response).
        auto tilt_db_at = [&](float x_norm) {
            if (!tiltActive) return 0.0f;
            const float freqHz = 20.0f * std::pow(clampedMaxPlotHz / 20.0f, x_norm);
            const float lowShelfMag = previewBiquadMagnitude(tiltLowShelf, sampleRate, freqHz);
            const float highShelfMag = previewBiquadMagnitude(tiltHighShelf, sampleRate, freqHz);
            return 20.0f * std::log10(std::max(1.0e-6f, lowShelfMag * highShelfMag));
        };

        auto response_db_at = [&](float x_norm) {
            float totalDb = 0.0f;
            for (int i = 0; i < 5; ++i)
                totalDb += band_db_at(x_norm, i);
            totalDb += tilt_db_at(x_norm);
            return totalDb;
        };

        float minResponseDb = 0.0f;
        float maxResponseDb = 0.0f;
        for (int sampleIndex = 0; sampleIndex < previewSamples; ++sampleIndex) {
            const float xn = (previewSamples <= 1) ? 0.0f : (float)sampleIndex / (float)(previewSamples - 1);
            const float responseDb = response_db_at(xn);
            minResponseDb = std::min(minResponseDb, responseDb);
            maxResponseDb = std::max(maxResponseDb, responseDb);
        }

        const PlotScale plotScale = plotScaleForResponseExtremaWithHysteresis(minResponseDb, maxResponseDb, state->plotTopDb, state->plotBottomDb);
        // Lower threshold for contraction (0.1f) vs expansion - this lets y-axis shrink more responsively
        const float scaleThreshold = (plotScale.topDb < state->plotTopDb - 0.5f) ? 0.1f : 0.01f;
        if (std::fabs(state->plotTopDb - plotScale.topDb) > scaleThreshold || std::fabs(state->plotBottomDb - plotScale.bottomDb) > scaleThreshold) {
            state->plotTopDb = plotScale.topDb;
            state->plotBottomDb = plotScale.bottomDb;
            state->plotScaleDirty = true;
        }

        auto y_from_db = [&](float db) {
            const float t = std::clamp((plotScale.topDb - db) / (plotScale.topDb - plotScale.bottomDb), 0.0f, 1.0f);
            return plotTop + (int)(t * (plotBottom - plotTop));
        };
        const int yFloor = y_from_db(plotScale.bottomDb);

        // Background grid: log-frequency verticals and dynamic dB horizontals.
        lv_draw_line_dsc_t grid_dsc; lv_draw_line_dsc_init(&grid_dsc);
        grid_dsc.width = 1;
        grid_dsc.color = COL_EQ_GRID;
        grid_dsc.opa = 145;

        const float freqMarks[] = {20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f};
        for (float hz : freqMarks) {
            const int x = x_from_hz_local(hz);
            const bool major = (hz == 100.0f || hz == 1000.0f || hz == 10000.0f);
            grid_dsc.opa = major ? 160 : 100;
            grid_dsc.p1.x = coords.x1 + x; grid_dsc.p1.y = coords.y1;
            grid_dsc.p2.x = coords.x1 + x; grid_dsc.p2.y = coords.y1 + h - 1;
            lv_draw_line(layer, &grid_dsc);
        }

        for (int markIndex = 0; markIndex < 5; ++markIndex) {
            const float db = dbMarkForIndex(state, markIndex);
            const int y = y_from_db(db);
            const bool isZero = (std::fabs(db) < 0.1f);
            grid_dsc.color = isZero ? COL_EQ_GRID_ZERO : COL_EQ_GRID;
            grid_dsc.opa = isZero ? 255 : 120;
            grid_dsc.width = isZero ? 2 : 1;  // 0 dB datum reads as intentional headroom divider
            grid_dsc.p1.x = coords.x1; grid_dsc.p1.y = coords.y1 + y;
            grid_dsc.p2.x = coords.x1 + w - 1; grid_dsc.p2.y = coords.y1 + y;
            lv_draw_line(layer, &grid_dsc);
        }

        // Draw analyzer spectrum first so EQ curve stays on top.
        lv_draw_line_dsc_t spectrum_dsc; lv_draw_line_dsc_init(&spectrum_dsc);
        spectrum_dsc.color = COL_EQ_SPECTRUM;
        spectrum_dsc.width = 3;
        spectrum_dsc.opa = 255;
        lv_draw_line_dsc_t spectrum_fill_dsc; lv_draw_line_dsc_init(&spectrum_fill_dsc);
        spectrum_fill_dsc.color = COL_EQ_SPECTRUM;
        spectrum_fill_dsc.width = 1;
        spectrum_fill_dsc.opa = 60;
        lv_draw_line_dsc_t spectrum_fill_top_dsc; lv_draw_line_dsc_init(&spectrum_fill_top_dsc);
        spectrum_fill_top_dsc.color = COL_EQ_SPECTRUM_TOP;
        spectrum_fill_top_dsc.width = 1;
        spectrum_fill_top_dsc.opa = 120;
        const int bins = FFTAnalyzer::kNumBands;
        if (bins > 1) {
            float envPlot[FFTAnalyzer::kNumBands] = {0.0f};
            float envMax = 0.0f;

            for (int i = 0; i < bins; ++i) {
                float raw = std::clamp(state->ui->getAnalyzerBand(i), 0.0f, 1.0f);
                float env = state->analyzerEnv[i];
                env += (raw > env) ? (raw - env) * 0.72f : (raw - env) * 0.18f;
                state->analyzerEnv[i] = env;
                envPlot[i] = env;
                envMax = std::max(envMax, env);
            }

            float visTarget = (envMax > 0.003f) ? 1.0f : 0.0f;
            state->analyzerVis += (visTarget > state->analyzerVis)
                ? (visTarget - state->analyzerVis) * 0.45f
                : (visTarget - state->analyzerVis) * 0.08f;
            state->analyzerVis = std::clamp(state->analyzerVis, 0.0f, 1.0f);

            if (state->analyzerVis > 0.01f) {
                spectrum_dsc.opa = (lv_opa_t)(255.0f * state->analyzerVis);
                spectrum_fill_dsc.opa = (lv_opa_t)(60.0f * state->analyzerVis);
                spectrum_fill_top_dsc.opa = (lv_opa_t)(120.0f * state->analyzerVis);

                const float analyzerSpanDb = plotScale.topDb - plotScale.bottomDb;
                for (int i = 0; i < bins - 1; ++i) {
                    const int x0 = (i * (w - 1)) / (bins - 1);
                    const int x1 = ((i + 1) * (w - 1)) / (bins - 1);

                    float db0 = plotScale.bottomDb + std::pow(std::clamp(envPlot[i], 0.0f, 1.0f), 0.35f) * analyzerSpanDb * 0.68f;
                    float db1 = plotScale.bottomDb + std::pow(std::clamp(envPlot[i + 1], 0.0f, 1.0f), 0.35f) * analyzerSpanDb * 0.68f;
                    db0 = std::clamp(db0, plotScale.bottomDb, plotScale.topDb);
                    db1 = std::clamp(db1, plotScale.bottomDb, plotScale.topDb);

                    const int y0 = std::clamp(y_from_db(db0), plotTop, plotBottom);
                    const int y1 = std::clamp(y_from_db(db1), plotTop, plotBottom);
                    const int yMid0 = yFloor - (int)((yFloor - y0) * 0.55f);

                    spectrum_fill_dsc.p1.x = coords.x1 + x0; spectrum_fill_dsc.p1.y = coords.y1 + yFloor;
                    spectrum_fill_dsc.p2.x = coords.x1 + x0; spectrum_fill_dsc.p2.y = coords.y1 + yMid0;
                    lv_draw_line(layer, &spectrum_fill_dsc);

                    spectrum_fill_top_dsc.p1.x = coords.x1 + x0; spectrum_fill_top_dsc.p1.y = coords.y1 + yMid0;
                    spectrum_fill_top_dsc.p2.x = coords.x1 + x0; spectrum_fill_top_dsc.p2.y = coords.y1 + y0;
                    lv_draw_line(layer, &spectrum_fill_top_dsc);

                    spectrum_dsc.p1.x = coords.x1 + x0; spectrum_dsc.p1.y = coords.y1 + y0;
                    spectrum_dsc.p2.x = coords.x1 + x1; spectrum_dsc.p2.y = coords.y1 + y1;
                    lv_draw_line(layer, &spectrum_dsc);
                }
            }
        }

        lv_draw_line_dsc_t line_dsc; lv_draw_line_dsc_init(&line_dsc);
        line_dsc.color = COL_EQ_LINE; line_dsc.width = 5; line_dsc.opa = 255;
        // Soft area fill between the response curve and the 0 dB datum — the
        // graph reads as an instrument display at rest, not a bare polyline.
        lv_draw_line_dsc_t fill_dsc; lv_draw_line_dsc_init(&fill_dsc);
        fill_dsc.color = COL_EQ_LINE; fill_dsc.width = 3; fill_dsc.opa = 90;

        // Per-band response curves — the "proper EQ display" pattern: each
        // band's own shape, drawn in the SAME colour as its node dot so the
        // curve→handle mapping is instant. Strokes stay thin (2px) and
        // mid-contrast; a band at unity (off/flat) draws nothing at all, so
        // no stroke ever lands exactly on the 0 dB grid as noise. The summed
        // white curve stays the dominant read on top.
        const lv_color_t bandColor[5] = { COL_EQ_HPF_BG, COL_EQ_LOW, COL_EQ_MID, COL_EQ_HIGH, COL_EQ_LPF_BG };
        bool bandOff[5];
        for (int i = 0; i < 5; ++i) bandOff[i] = bandDisabled(state, i);
        lv_draw_line_dsc_t bandFillDsc[5];
        lv_draw_line_dsc_t bandLineDsc[5];
        for (int i = 0; i < 5; ++i) {
            lv_draw_line_dsc_init(&bandFillDsc[i]);
            bandFillDsc[i].color = bandOff[i] ? COL_TEXT_DIM : bandColor[i];
            bandFillDsc[i].width = 2;
            bandFillDsc[i].opa = bandOff[i] ? 0 : 50;   // disabled: no fill
            lv_draw_line_dsc_init(&bandLineDsc[i]);
            bandLineDsc[i].color = bandOff[i] ? COL_TEXT_DIM : bandColor[i];
            bandLineDsc[i].width = 2;
            bandLineDsc[i].opa = bandOff[i] ? 70 : 170; // disabled: faint grey
        }

        if (w > 1) {
            const int yZeroFill = std::clamp(y_from_db(0.0f), 0, h - 1);
            // 1px sampling (was 2px): steep shelf edges and narrow resonances
            // stop aliasing into stair steps — that jaggedness was the legibility
            // complaint. One pass computes every band at each x, so the cost is
            // the old per-pixel filter work, once per pixel instead of per 2px.
            int prevX = 0;
            int prevBandY[5] = {0, 0, 0, 0, 0};
            int prevSumY = 0;
            bool havePrev = false;
            // Magnitude evaluation is trig-heavy (tan/cos/exp/pow) and several
            // models loop internally. Sampling the response on at most 384
            // segments per band keeps a redraw bounded instead of scaling
            // linearly with canvas width (CPU spikes at large windows/extreme
            // settings).
            const int sampleStep = std::max(1, w / 384);
            // `w` is not necessarily a multiple of the step: pin the final
            // segment to w-1 or the right edge of every curve is left blank
            // (reads as a response bug, not as sampling).
            for (int x = 0;; x += sampleStep) {
                const bool last = (x + sampleStep >= w);
                const int xEnd = last ? (w - 1) : x;
                const float xn = (float)xEnd / (float)(w - 1);
                float bandDb[5];
                float totalDb = 0.0f;
                for (int i = 0; i < 5; ++i) {
                    bandDb[i] = bandOff[i] ? 0.0f : band_db_at(xn, i);
                    totalDb += bandDb[i];
                }
                totalDb += tilt_db_at(xn);

                const int ySum = std::clamp(y_from_db(totalDb), 0, h - 1);

                for (int i = 0; i < 5; ++i) {
                    const int yB = std::clamp(y_from_db(bandDb[i]), 0, h - 1);
                    if (bandFillDsc[i].opa > 0 && yB != yZeroFill) {
                        bandFillDsc[i].p1.x = coords.x1 + xEnd; bandFillDsc[i].p1.y = coords.y1 + yB;
                        bandFillDsc[i].p2.x = coords.x1 + xEnd; bandFillDsc[i].p2.y = coords.y1 + yZeroFill;
                        lv_draw_line(layer, &bandFillDsc[i]);
                    }
                    if (havePrev && (yB != yZeroFill || prevBandY[i] != yZeroFill)) {
                        bandLineDsc[i].p1.x = coords.x1 + prevX;    bandLineDsc[i].p1.y = coords.y1 + prevBandY[i];
                        bandLineDsc[i].p2.x = coords.x1 + xEnd;     bandLineDsc[i].p2.y = coords.y1 + yB;
                        lv_draw_line(layer, &bandLineDsc[i]);
                    }
                    prevBandY[i] = yB;
                }

                if (fill_dsc.opa > 0 && ySum != yZeroFill) {
                    fill_dsc.p1.x = coords.x1 + xEnd; fill_dsc.p1.y = coords.y1 + ySum;
                    fill_dsc.p2.x = coords.x1 + xEnd; fill_dsc.p2.y = coords.y1 + yZeroFill;
                    lv_draw_line(layer, &fill_dsc);
                }
                if (havePrev) {
                    line_dsc.p1.x = coords.x1 + prevX;  line_dsc.p1.y = coords.y1 + prevSumY;
                    line_dsc.p2.x = coords.x1 + xEnd;   line_dsc.p2.y = coords.y1 + ySum;
                    lv_draw_line(layer, &line_dsc);
                }

                prevX = xEnd;
                prevSumY = ySum;
                havePrev = true;
                if (last) break;
            }
        }

        // Draw Dots — all 5 nodes uniformly
        for(int i=0; i<5; ++i) {
            int px = 0, py = 0;
            nodeCenter(state, i, w, plotTop, plotBottom, px, py);
            const float fx = (w > 0) ? (float)px / (float)w : 0.0f;
            const int tfp = filterTypeParamForBand(state->baseParam, i);
            float typeRaw = (tfp >= 0) ? state->ui->getParamValue(tfp) : 0.0f;
            int typeIdx = std::clamp((int)std::round(typeRaw), 0, 7);
            bool gainRelevant = (typeIdx >= 4 && typeIdx <= 6);
            bool qRelevant = (typeIdx <= 3 || typeIdx == 7);

            const bool off = bandOff[i];
            lv_draw_rect_dsc_t rect_dsc; lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.radius = LV_RADIUS_CIRCLE;
            rect_dsc.bg_color = off ? COL_BORDER : bandColor[i];
            rect_dsc.bg_opa = off ? LV_OPA_60
                                  : (isBandSelected(state, i) ? LV_OPA_COVER : LV_OPA_90);
            // Thin/dim rim: the white TYPE GLYPH is the icon now, so a fat
            // white ring competed with it (the glyph read as a smudge).
            rect_dsc.border_width = 1;
            rect_dsc.border_color = off ? COL_TEXT_DIM : lv_color_white();
            rect_dsc.border_opa = off ? LV_OPA_50 : LV_OPA_60;
            // Strong glow under the node dot (lit handle); a
            // disabled band has no glow, matching its flat grey response.
            rect_dsc.shadow_color = bandColor[i];
            rect_dsc.shadow_width = off ? 0 : 14;
            rect_dsc.shadow_opa = off ? 0 : 110;
            
            const int satP = satParamForBand(state->baseParam, i);
            const float satVal = (satP >= 0) ? std::clamp(state->ui->getParamValue(satP), 0.0f, 1.0f) : 0.0f;
            int nodeRadius = (int)(std::max(7, std::min(20, h / 25)) * (1.0f + satVal * 0.6f));
            lv_area_t dot_area = {coords.x1 + px - nodeRadius, coords.y1 + py - nodeRadius, coords.x1 + px + nodeRadius, coords.y1 + py + nodeRadius};
            lv_draw_rect(layer, &rect_dsc, &dot_area);

            if (isBandSelected(state, i)) {
                lv_draw_rect_dsc_t outline;
                lv_draw_rect_dsc_init(&outline);
                outline.bg_opa = LV_OPA_TRANSP;
                outline.border_color = COL_HIGHLIGHT;
                outline.border_width = 2;
                outline.border_opa = LV_OPA_COVER;
                outline.radius = LV_RADIUS_CIRCLE;
                lv_area_t ring_area = {dot_area.x1 - 3, dot_area.y1 - 3,
                                       dot_area.x2 + 3, dot_area.y2 + 3};
                lv_draw_rect(layer, &outline, &ring_area);
            }

            // Filter-TYPE glyph inside the node. The handle used to be a
            // generic diamond; the node now shows what the band actually does,
            // so the graph reads without selecting the band:
            //   LP \_  HP _/  BP/Pell /\  Notch \/  LS _\  HS _/  AP ~
            // Points are normalised (-1..1) and drawn as a polyline; y is
            // screen-down, so a boost (positive dB) is NEGATIVE y.
            struct GlyphPoint { float x, y; };
            GlyphPoint glyph[5];
            int glyphN = 0;
            switch (typeIdx) {
                case 0: // Low-pass: flat, then fall
                    glyph[0] = {-0.80f, -0.15f}; glyph[1] = {0.05f, -0.15f}; glyph[2] = {0.80f, 0.55f}; glyphN = 3; break;
                case 2: // High-pass: rise, then flat
                    glyph[0] = {-0.80f, 0.55f}; glyph[1] = {-0.05f, -0.15f}; glyph[2] = {0.80f, -0.15f}; glyphN = 3; break;
                case 1: // Band-pass: peak
                    glyph[0] = {-0.80f, 0.50f}; glyph[1] = {0.0f, -0.55f}; glyph[2] = {0.80f, 0.50f}; glyphN = 3; break;
                case 3: // Notch: dip
                    glyph[0] = {-0.80f, -0.45f}; glyph[1] = {0.0f, 0.55f}; glyph[2] = {0.80f, -0.45f}; glyphN = 3; break;
                case 4: // Bell: symmetric peak with shoulders
                    glyph[0] = {-0.80f, 0.10f}; glyph[1] = {-0.30f, 0.10f}; glyph[2] = {0.0f, -0.55f};
                    glyph[3] = {0.30f, 0.10f}; glyph[4] = {0.80f, 0.10f}; glyphN = 5; break;
                case 5: // Low shelf: boost low, unity high
                    glyph[0] = {-0.80f, -0.50f}; glyph[1] = {-0.10f, -0.50f}; glyph[2] = {0.80f, 0.35f}; glyphN = 3; break;
                case 6: // High shelf: unity low, boost high
                    glyph[0] = {-0.80f, 0.35f}; glyph[1] = {0.10f, 0.35f}; glyph[2] = {0.80f, -0.50f}; glyphN = 3; break;
                default: // All-pass: phase twist
                    glyph[0] = {-0.80f, 0.0f}; glyph[1] = {-0.25f, 0.0f}; glyph[2] = {0.0f, -0.40f};
                    glyph[3] = {0.25f, 0.40f}; glyph[4] = {0.80f, 0.0f}; glyphN = 5; break;
            }
            lv_draw_line_dsc_t glyphLine; lv_draw_line_dsc_init(&glyphLine);
            glyphLine.color = lv_color_white();
            glyphLine.opa = off ? LV_OPA_50 : LV_OPA_COVER;
            glyphLine.width = (lv_coord_t)std::max(2, nodeRadius / 5);
            glyphLine.round_start = 1;
            glyphLine.round_end = 1;
            // Keep the icon legible even on the smallest node: a 7px dot
            // would otherwise show a 6px smudge.
            const float gScale = std::max(7.5f, (float)nodeRadius * 0.95f);
            for (int k = 1; k < glyphN; ++k) {
                glyphLine.p1.x = (lv_coord_t)std::lround(coords.x1 + px + glyph[k - 1].x * gScale);
                glyphLine.p1.y = (lv_coord_t)std::lround(coords.y1 + py + glyph[k - 1].y * gScale);
                glyphLine.p2.x = (lv_coord_t)std::lround(coords.x1 + px + glyph[k].x * gScale);
                glyphLine.p2.y = (lv_coord_t)std::lround(coords.y1 + py + glyph[k].y * gScale);
                lv_draw_line(layer, &glyphLine);
            }

            // Frequency readout directly under the dot
            char hzBuf[16], valBuf[16];
            const float hz = DSPUtils::normToHz( std::clamp(fx, 0.0f, 1.0f));
            fmt_hz(hz, hzBuf, sizeof(hzBuf));
            if (gainRelevant) {
                const float gainNorm = UIEQEditor::uiLiveValue(state->ui, (uint32_t)(state->baseParam + i*4 + 1));
                const float gainDb = (gainNorm - 0.5f) * 48.0f;
                snprintf(valBuf, sizeof(valBuf), "%+.1f dB", gainDb);
            } else if (qRelevant) {
                const float qNorm = UIEQEditor::uiLiveValue(state->ui, (uint32_t)(state->baseParam + i*4 + 2));
                const float qVal = DSPUtils::qFromParam(qNorm);
                snprintf(valBuf, sizeof(valBuf), "Q %.2f", qVal);
            } else {
                valBuf[0] = '\0';
            }

            lv_draw_label_dsc_t lbl; lv_draw_label_dsc_init(&lbl);
            lbl.color = COL_EQ_LABEL;
            lbl.opa = LV_OPA_COVER;
            lbl.font = uiMicroFont();

            // Hz label below dot — dark chip so it reads over the grid.
            // text_local=1: the draw task COPIES the text. Stack buffers die
            // when this callback returns; LVGL9 may dispatch the task later,
            // and a dangling text pointer spins the renderer forever
            // (text_length=0xFFFFFFFF garbage — the intermittent freeze).
            lbl.text = hzBuf;
            lbl.text_local = 1; lbl.text_static = 0;
            lv_point_t sz{};
            lv_txt_get_size(&sz, hzBuf, lbl.font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
            {
                const int tx = std::clamp(coords.x1 + px - sz.x / 2, coords.x1, coords.x1 + w - sz.x);
                const int ty = std::clamp(coords.y1 + py + 14, coords.y1 + 2, coords.y1 + h - 16);
                lv_area_t ta = {tx - 3, ty - 2, tx + sz.x + 3, ty + sz.y + 2};
                lv_draw_rect_dsc_t chip; lv_draw_rect_dsc_init(&chip);
                chip.bg_color = COL_PANEL_DARK; chip.bg_opa = LV_OPA_90;
                chip.radius = 3;
                chip.border_color = COL_ACCENT; chip.border_width = 1; chip.border_opa = LV_OPA_70;
                lv_draw_rect(layer, &chip, &ta);
                lv_area_t ta_lbl = {tx, ty, tx + sz.x + 2, ty + sz.y + 2};
                lv_draw_label(layer, &lbl, &ta_lbl);
            }

            // Value label above dot (only when selected)
            if (state->selectedBand == i && (gainRelevant || qRelevant)) {
                lv_point_t sz2{};
                lv_txt_get_size(&sz2, valBuf, lbl.font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
                lbl.text = valBuf;
                lbl.text_local = 1; lbl.text_static = 0;
                const int tx2 = std::clamp(coords.x1 + px - sz2.x / 2, coords.x1, coords.x1 + w - sz2.x);
                const int ty2 = std::clamp(coords.y1 + py - 24, coords.y1 + 2, coords.y1 + h - 16);
                lv_area_t ta2 = {tx2 - 3, ty2 - 2, tx2 + sz2.x + 3, ty2 + sz2.y + 2};
                lv_draw_rect_dsc_t chip2; lv_draw_rect_dsc_init(&chip2);
                chip2.bg_color = COL_PANEL_DARK; chip2.bg_opa = LV_OPA_90;
                chip2.radius = 3;
                chip2.border_color = COL_ACCENT; chip2.border_width = 1; chip2.border_opa = LV_OPA_70;
                lv_draw_rect(layer, &chip2, &ta2);
                lv_area_t ta2_lbl = {tx2, ty2, tx2 + sz2.x + 2, ty2 + sz2.y + 2};
                lv_draw_label(layer, &lbl, &ta2_lbl);
            }
        }

        if (state->marqueeActive) {
            const int x1 = coords.x1 + std::min(state->marqueeStart.x, state->marqueeCurrent.x);
            const int x2 = coords.x1 + std::max(state->marqueeStart.x, state->marqueeCurrent.x);
            const int y1 = coords.y1 + std::min(state->marqueeStart.y, state->marqueeCurrent.y);
            const int y2 = coords.y1 + std::max(state->marqueeStart.y, state->marqueeCurrent.y);
            if (x2 > x1 && y2 > y1) {
                lv_draw_rect_dsc_t marquee;
                lv_draw_rect_dsc_init(&marquee);
                marquee.bg_color = COL_HIGHLIGHT;
                marquee.bg_opa = LV_OPA_20;
                marquee.border_color = COL_HIGHLIGHT;
                marquee.border_width = 1;
                marquee.border_opa = LV_OPA_80;
                lv_area_t box = {x1, y1, x2, y2};
                lv_draw_rect(layer, &marquee, &box);
            }
        }
    }
};

END_NAMESPACE_DISTRHO

#endif


