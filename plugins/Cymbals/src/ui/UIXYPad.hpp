#ifndef UI_XY_PAD_HPP
#define UI_XY_PAD_HPP

#include "lvgl.h"
#include "UICommon.hpp"
#include "../PluginCymbals.hpp"
#include "UIStyles.hpp"
#include <algorithm>
#include <cmath>

START_NAMESPACE_DISTRHO

// FIX (filter chart mod visualization): worst-case reachable [lo,hi] of a
// routed parameter under active modulation routes. Implemented in PluginUI.cpp
// (CymbalsUI::getModExtentForParam); returns base..base when no enabled route
// targets the parameter, so unbiased callers are unaffected.
struct CymbalsModExtent {
    float lo;
    float hi;
};
CymbalsModExtent cymbalsUiModExtent(const AbstractCymbalsUI* ui, uint32_t targetParam);

struct UIXYPad {
    using OnCircleTouchedFn = void (*)(void* userData, int groupIdx, int localCombIdx);
    static constexpr int kCurvePoints = 96;
    // Logarithmic Y-axis helpers: spreads out the 0.9–0.999 feedback region
    // using -log10(1-p) so high-feedback values get most of the screen space.
    static constexpr float kFBLogMax = 2.5f; // corresponds to p≈0.997
    // param (0..1) -> screen fraction (0=top, 1=bottom)
    static float fbParamToScreen(float p) {
        p = std::clamp(p, 0.0f, 0.999f);
        float logVal = -std::log10(std::max(1.0f - p, 0.001f));
        return 1.0f - std::clamp(logVal / kFBLogMax, 0.0f, 1.0f);
    }
    // screen fraction (0=top, 1=bottom) -> param (0..1)
    static float fbScreenToParam(float s) {
        s = std::clamp(s, 0.0f, 1.0f);
        float logVal = (1.0f - s) * kFBLogMax;
        return std::clamp(1.0f - std::pow(10.0f, -logVal), 0.0f, 0.999f);
    }
    struct State {
        AbstractCymbalsUI* ui;
        int groupIdx;
        bool isFinal;
        bool selected[4];
        bool lassoActive;
        lv_point_t lassoStart;
        lv_point_t lassoEnd;
        bool dragging;
        lv_point_t dragLastCanvas;
        OnCircleTouchedFn onCircleTouched;
        void* circleTouchedUserData;
        bool curveSmoothingInit;
        bool curveSettled = false; // smoothing converged; timer may stop repainting
        float curveSmoothedDb[4][kCurvePoints];
        bool phaseView = false; // draw phase (degrees) instead of magnitude (dB)
        lv_timer_t* redrawTimer = nullptr; // 33ms: live response follows modulation
        // The FILTER/COLOUR dock that edits this group's combs (FLOATING child
        // of the group card). Set by the tab that builds it so the tab-level
        // outside-press handler can tell a press inside the dock from one
        // anywhere else on the screen. null for pads with no dock.
        lv_obj_t* dockPanel = nullptr;
    };

    // Plot margins inside the pad's own border. Every axis label and tick in
    // draw_main is offset from coords by these instead of raw pixel literals,
    // so the "FEEDBACK" label can never be cut by the panel edge (round-2
    // critic) and the grid reserves a permanent lane for the labels at any
    // gUIScale.
    static lv_coord_t plotMarginX() { return scaled(10); }
    static lv_coord_t plotMarginY() { return scaled(8); }

    // OWNERSHIP: State* allocated with 'new', stored in LVGL user_data,
    // DELETED in event_cb LV_EVENT_DELETE handler when LVGL destroys obj
    static lv_obj_t* create(lv_obj_t* parent, int groupIdx, AbstractCymbalsUI* ui, UIStyles& /*styles*/, bool isFinal = false,
                            OnCircleTouchedFn onCircleTouched = nullptr, void* circleTouchedUserData = nullptr) {
        // Ownership: parent (LVGL)
        lv_obj_t* obj = lv_obj_create(parent);
        lv_obj_set_size(obj, scaled(400), scaled(400));
        lv_obj_set_style_bg_color(obj, COL_PANEL, 0);
        lv_obj_set_style_bg_opa(obj, 255, 0);
        lv_obj_set_style_border_color(obj, COL_ACCENT, 0);
        lv_obj_set_style_border_width(obj, 2, 0);
        lv_obj_set_style_radius(obj, 8, 0);
        // Prevent scrolling propagation to parent when interacting with this object
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);

        // Ownership: user-allocated, deleted in event_cb when LV_EVENT_DELETE fires
        State* state = new State();
        state->ui = ui;
        state->groupIdx = groupIdx;
        state->isFinal = isFinal;
        for(int i=0; i<4; i++) state->selected[i] = false;
        state->lassoActive = false;
        state->dragging = false;
        state->onCircleTouched = onCircleTouched;
        state->circleTouchedUserData = circleTouchedUserData;
        state->curveSmoothingInit = false;
        state->phaseView = false;
        for (int i = 0; i < 4; ++i)
            for (int p = 0; p < kCurvePoints; ++p)
                state->curveSmoothedDb[i][p] = 0.0f;

        lv_obj_set_user_data(obj, state);
        lv_obj_add_event_cb(obj, event_cb, LV_EVENT_ALL, state);
        // Live response: a routed LFO moves the knobs, but the magnitude
        // curves are only recomputed inside draw_main. Without a tick the
        // graph stays frozen on the last interaction.
        state->redrawTimer = lv_timer_create([](lv_timer_t* t) {
            lv_obj_t* pad = (lv_obj_t*)lv_timer_get_user_data(t);
            State* st = pad ? static_cast<State*>(lv_obj_get_user_data(pad)) : nullptr;
            if (pad && st && !st->curveSettled &&
                lv_obj_is_valid(pad) && lv_obj_is_visible(pad))
                lv_obj_invalidate(pad);
        }, 33, obj);
        
        return obj;
    }

    // Returns a 4-bit mask of currently selected circles for this XY pad.
    // Bit 0..3 correspond to local combs 1..4.
    static int getSelectedMask(lv_obj_t* obj) {
        if (!obj) return 0;
        State* state = static_cast<State*>(lv_obj_get_user_data(obj));
        if (!state) return 0;
        int mask = 0;
        for (int i = 0; i < 4; ++i)
            if (state->selected[i]) mask |= (1 << i);
        return mask;
    }

    // Toggles a comb circle programmatically from the 1-4 selection strip.
    // Selecting ADDS to the mask and fires the caller's circle-touched
    // callback (opens/refreshes the dock); deselecting clears only that bit and
    // leaves the caller to re-sync its strip/title and focus. Toggling is what
    // lets the four strip buttons select filters 1+2+3+4 together; the old
    // rewrite-all-to-(i==j) made the last click discard the earlier ones.
    static void selectComb(lv_obj_t* obj, int localCombIdx) {
        if (!obj) return;
        State* state = static_cast<State*>(lv_obj_get_user_data(obj));
        if (!state) return;
        const int i = std::clamp(localCombIdx, 0, 3);
        const bool selecting = !state->selected[i];
        state->selected[i] = selecting;
        state->curveSettled = false;
        lv_obj_invalidate(obj);
        if (selecting && state->onCircleTouched)
            state->onCircleTouched(state->circleTouchedUserData, state->groupIdx, i);
    }

    // Phase view: draw the overlay curves as phase (degrees) instead of
    // magnitude (dB). Useful where magnitude is flat (ALLPASS cascade) but the
    // phase crowding is the audible character.
    static void setPhaseView(lv_obj_t* obj, bool on) {
        if (!obj) return;
        State* state = static_cast<State*>(lv_obj_get_user_data(obj));
        if (!state || state->phaseView == on) return;
        state->phaseView = on;
        // Leaving phase view: the smoothing buffer holds degrees; snap to dB
        // instead of one-pole-morphing through a mixed-scale blend.
        if (!on) state->curveSmoothingInit = false;
        state->curveSettled = false;
        lv_obj_invalidate(obj);
    }

    static bool getPhaseView(lv_obj_t* obj) {
        if (!obj) return false;
        State* state = static_cast<State*>(lv_obj_get_user_data(obj));
        return state ? state->phaseView : false;
    }

    // Snap the response curves to their targets (discrete changes like a
    // filter-type switch must not morph through the one-pole smoothing).
    static void resetCurveSmoothing(lv_obj_t* obj) {
        if (!obj) return;
        State* state = static_cast<State*>(lv_obj_get_user_data(obj));
        if (!state) return;
        state->curveSmoothingInit = false;
        state->curveSettled = false;
        lv_obj_invalidate(obj);
    }

    static void clearSelection(lv_obj_t* obj) {
        if (!obj) return;
        State* state = static_cast<State*>(lv_obj_get_user_data(obj));
        if (!state) return;
        bool anyChanged = false;
        for (int i = 0; i < 4; ++i) {
            if (state->selected[i]) {
                state->selected[i] = false;
                anyChanged = true;
            }
        }
        state->lassoActive = false;
        state->dragging = false;
        if (anyChanged) {
            lv_obj_invalidate(obj);
            lv_obj_send_event(obj, LV_EVENT_VALUE_CHANGED, state);
        }
    }

private:
    // Per-pass modulation bias for the response overlay: substitutes
    // clamp(base ± extentSum, 0, 1) for every parameter shaped by an enabled
    // mod route. Extents are queried once per distinct param (cached in
    // `entries`) so route scanning stays O(1) amortized per read. A default
    // bias (ui == nullptr) reads plain base values — zero overhead pass-through.
    struct CurveBias {
        static constexpr int kMaxEntries = 96;
        struct Entry {
            uint32_t idx;
            float lo;
            float hi;
        };
        mutable Entry entries[kMaxEntries];
        mutable int count;
        int sign; // +1 worst-case-plus pass, -1 worst-case-minus pass
        const AbstractCymbalsUI* ui;

        CurveBias() : count(0), sign(1), ui(nullptr) {
            for (int i = 0; i < kMaxEntries; ++i) {
                entries[i].idx = 0;
                entries[i].lo = 0.0f;
                entries[i].hi = 0.0f;
            }
        }

        float resolve(uint32_t idx, float base) const {
            for (int i = 0; i < count; ++i)
                if (entries[i].idx == idx)
                    return sign > 0 ? entries[i].hi : entries[i].lo;
            float lo = base, hi = base;
            if (ui != nullptr && count < kMaxEntries) {
                const CymbalsModExtent ext = cymbalsUiModExtent(ui, idx);
                lo = ext.lo;
                hi = ext.hi;
                entries[count].idx = idx;
                entries[count].lo = lo;
                entries[count].hi = hi;
                ++count;
            }
            return sign > 0 ? hi : lo;
        }
    };

    static float biasedParam(State* state, const CurveBias& bias, uint32_t idx) {
        const float base = state->ui->getParamValue(idx);
        // No explicit HI/LO bias -> draw the LIVE modulated value, so the
        // response curve moves when an LFO drives frequency/feedback/cutoff.
        return (bias.ui != nullptr) ? bias.resolve(idx, base)
                                    : state->ui->getModulatedParamValue(idx);
    }

    static float normalizedHzFromParam(float param) {
        return DSPUtils::normToHz( std::clamp(param, 0.0f, 1.0f));
    }

    // Magnitude of a one-pole low-pass / high-pass at frequency fr, cutoff fc.
    // These mirror the DSP's own stage filters (CombFilterBank
    // Stage::updateFilterCoefficients -> OnePoleFilter lpfFilter/hpfFilter,
    // fed from DSPUtils::normToHz), so the drawn response and the audio path
    // agree on the cutoff mapping.
    static float onePoleLpMag(float fr, float fc) {
        const float r = fr / std::max(20.0f, fc);
        return 1.0f / std::sqrt(1.0f + r * r);
    }

    static float onePoleHpMag(float fr, float fc) {
        const float r = fr / std::max(20.0f, fc);
        return r / std::sqrt(1.0f + r * r);
    }

    // The group LPF/HPF knobs broadcast to every stage in the group
    // (groupCombLPFHPFChangedCallback, PluginUI.cpp), so every stage's drawn
    // response has to carry its own cutoffs or the overlay reads as unresponsive
    // to them. Callers that sample many frequencies (the partials bar chart)
    // hoist the two cutoffs and call onePoleLpMag/onePoleHpMag per sample.
    static float lpfHpfMag(State* state, int i, float hz, const CurveBias& bias = CurveBias()) {
        int freqIdx, fbIdx, scaleIdx;
        get_param_indices(state, i, freqIdx, fbIdx, scaleIdx);
        const float lpfHz = normalizedHzFromParam(biasedParam(state, bias, (uint32_t)freqIdx + 2));
        const float hpfHz = normalizedHzFromParam(biasedParam(state, bias, (uint32_t)freqIdx + 3));
        const float fr = std::max(1.0f, hz);
        return onePoleLpMag(fr, lpfHz) * onePoleHpMag(fr, hpfHz);
    }

    static float smoothstep(float edge0, float edge1, float x) {
        const float span = std::max(1e-6f, edge1 - edge0);
        const float t = std::clamp((x - edge0) / span, 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    static float getScaleMultiplier(float scaleParam) {
        return DSPUtils::scaleFactorFromParam( std::clamp(scaleParam, 0.0f, 1.0f));
    }

    static float getCombBaseFreqParam(State* state, int i, const CurveBias& bias = CurveBias()) {
        int freqIdx, fbIdx, scaleIdx;
        get_param_indices(state, i, freqIdx, fbIdx, scaleIdx);

        float fx = std::clamp(biasedParam(state, bias, (uint32_t)freqIdx), 0.0f, 1.0f);

        // Apply cluster adjustment (pulling frequencies toward group center) in log domain.
        float cluster = 0.0f;
        if (state->isFinal) {
            cluster = std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalCombGroupCluster), 0.0f, 1.0f);
        } else if (state->groupIdx >= 0 && state->groupIdx < 10) {
            const int clusterParam = PluginCymbals::kParamCombGroup1Cluster + state->groupIdx;
            cluster = std::clamp(biasedParam(state, bias, (uint32_t)clusterParam), 0.0f, 1.0f);
        }

        [[maybe_unused]] static float s_lastCluster = -1.0f;
        if (cluster > 0.001f) {
            // Convert to Hz and apply clustering in log2 space (like the DSP does).
            float hz = normalizedHzFromParam(fx);
            float logHz = std::log2(std::max(20.0f, hz));

            float centerHz = 0.0f;
            for (int s = 0; s < 4; ++s) {
                int fi, bi, si;
                get_param_indices(state, s, fi, bi, si);
                float p = std::clamp(biasedParam(state, bias, (uint32_t)fi), 0.0f, 1.0f);
                centerHz += normalizedHzFromParam(p);
            }
            centerHz *= 0.25f;
            float centerLog = std::log2(std::max(20.0f, centerHz));

            float newLogHz = logHz + cluster * (centerLog - logHz);
            float newHz = std::pow(2.0f, newLogHz);
            fx = std::clamp((float)(std::log10(newHz / 20.0f) / 3.0f), 0.0f, 1.0f);
        }


        return fx;
    }

    static float getScaledCombFreqHz(State* state, int i, const CurveBias& bias = CurveBias()) {
        int freqIdx, fbIdx, scaleIdx;
        get_param_indices(state, i, freqIdx, fbIdx, scaleIdx);

        const float baseHz = normalizedHzFromParam(getCombBaseFreqParam(state, i, bias));
        const float globalScaleParam = state->isFinal
            ? biasedParam(state, bias, PluginCymbals::kParamFinalCombScale)
            : biasedParam(state, bias, PluginCymbals::kParamCombScale);
        const float globalScale = getScaleMultiplier(globalScaleParam);

        float stageScale = 1.0f;
        if (!state->isFinal)
            stageScale = getScaleMultiplier(biasedParam(state, bias, (uint32_t)scaleIdx));

        // Master Tune transposes the played stick/voice only; it must not move
        // filter-selector positions or the response chart.
        const float scaledHz = baseHz * globalScale * stageScale;
        return std::clamp(scaledHz, 10.0f, 20000.0f);
    }

    // RBJ cookbook biquad coefficients normalized to a0 = 1 (mirrors
    // DSPUtils::Biquad::setParams so the overlay matches the DSP). type:
    // 0=ALL_PASS 1=NOTCH 2=LPF 3=BAND_PASS 4=HPF (same order as Biquad::Type).
    static void rbjCoeffs(int type, float freqHz, float Q, float sr, float b[3], float a[3]) {
        const float omega = DSPUtils::kPi * 2.0f * std::clamp(freqHz, 10.0f, sr * 0.49f) / sr;
        const float sn = std::sin(omega), cs = std::cos(omega);
        const float alpha = sn / (2.0f * Q);
        float a0 = 1.0f;
        b[0] = 1.0f; b[1] = 0.0f; b[2] = 0.0f;
        a[0] = 1.0f; a[1] = 0.0f; a[2] = 0.0f;
        switch (type) {
            case 0: // ALL_PASS
                b[0] = 1.0f - alpha; b[1] = -2.0f * cs; b[2] = 1.0f + alpha;
                a0 = 1.0f + alpha; a[1] = -2.0f * cs; a[2] = 1.0f - alpha;
                break;
            case 1: // NOTCH
                b[0] = 1.0f; b[1] = -2.0f * cs; b[2] = 1.0f;
                a0 = 1.0f + alpha; a[1] = -2.0f * cs; a[2] = 1.0f - alpha;
                break;
            case 2: // LPF
                b[0] = (1.0f - cs) / 2.0f; b[1] = 1.0f - cs; b[2] = (1.0f - cs) / 2.0f;
                a0 = 1.0f + alpha; a[1] = -2.0f * cs; a[2] = 1.0f - alpha;
                break;
            case 3: // BAND_PASS (constant skirt gain: peak at f0 = Q, mirrors the DSP)
                b[0] = Q * alpha; b[1] = 0.0f; b[2] = -(Q * alpha);
                a0 = 1.0f + alpha; a[1] = -2.0f * cs; a[2] = 1.0f - alpha;
                break;
            case 4: // HPF
                b[0] = (1.0f + cs) / 2.0f; b[1] = -(1.0f + cs); b[2] = (1.0f + cs) / 2.0f;
                a0 = 1.0f + alpha; a[1] = -2.0f * cs; a[2] = 1.0f - alpha;
                break;
        }
        if (std::abs(a0) > 1e-9f) {
            b[0] /= a0; b[1] /= a0; b[2] /= a0; a[1] /= a0; a[2] /= a0;
        }
    }

    // H(e^{jw}) for normalized (a0=1) biquad coefficients.
    static void biquadEvalComplex(const float b[3], const float a[3], float w, float& re, float& im) {
        const float c1 = std::cos(w), s1 = std::sin(w);
        const float c2 = std::cos(2.0f * w), s2 = std::sin(2.0f * w);
        const float nre = b[0] + b[1] * c1 + b[2] * c2;
        const float nim = -(b[1] * s1 + b[2] * s2);
        const float dre = 1.0f + a[1] * c1 + a[2] * c2;
        const float dim = -(a[1] * s1 + a[2] * s2);
        const float d2 = dre * dre + dim * dim;
        re = (nre * dre + nim * dim) / d2;
        im = (nim * dre - nre * dim) / d2;
    }

    static int stageFilterType(State* state, int i, const CurveBias& bias = CurveBias()) {
        if (state->isFinal)
            return (int)std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalComb1FilterType + i), 0.0f, (float)(CombFilterBank::FT_COUNT - 1));
        return (int)std::clamp(biasedParam(state, bias, PluginCymbals::kParamComb1FilterType + state->groupIdx * 4 + i), 0.0f, (float)(CombFilterBank::FT_COUNT - 1));
    }

    // Y-axis dimension per stage: comb-family types use feedback (log axis);
    // Saike partials models use damping (Mode Amt, linear axis, top = least
    // damping = longest ring) — the natural edit axis for those models. The
    // circle then tracks the Mode Amt arc and dragging it moves the arc.
    static bool stageYIsDamping(State* state, int i) {
        return CombFilterBank::isPartialType(stageFilterType(state, i));
    }

    static int stageYParam(State* state, int i) {
        if (state->isFinal)
            return PluginCymbals::kParamFinalComb1ModeAmount + i;
        return PluginCymbals::kParamComb1ModeAmount + state->groupIdx * 4 + i;
    }

    // Shared FT_PARTIAL_* param derivation for the preview (matches the DSP
    // knob mapping: Mode Amt = damp, Mode Rate = inharmonicity, plus the
    // Saike model controls, converted normalized 0..1 -> real). Returns false
    // for non-partials types.
    static bool getPartialsParams(State* state, int i, int& model,
                                  float& modeAmt, float& modeRate,
                                  float& relPos, float& freqDamp, int& partCount,
                                  const CurveBias& bias = CurveBias()) {
        const int filterType = stageFilterType(state, i, bias);
        if (!CombFilterBank::isPartialType(filterType)) return false;
        model = CombFilterBank::kPartialModelByType[filterType - CombFilterBank::FT_PARTIAL_BEAM_CLAMP];
        if (state->isFinal) {
            modeAmt = std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalComb1ModeAmount + i), 0.0f, 1.0f);
            modeRate = std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalComb1ModeRate + i), 0.0f, 1.0f);
            relPos = 0.0001f + std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalComb1PartRelPos + i), 0.0f, 1.0f) * 0.9989f;
            freqDamp = -6.0f + std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalComb1PartFreqDamp + i), 0.0f, 1.0f) * 5.0f;
            partCount = 16 + (int)std::lroundf(std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalComb1PartCount + i), 0.0f, 1.0f) * 48.0f);
        } else {
            const int stageIdx = state->groupIdx * 4 + i;
            modeAmt = std::clamp(biasedParam(state, bias, PluginCymbals::kParamComb1ModeAmount + stageIdx), 0.0f, 1.0f);
            modeRate = std::clamp(biasedParam(state, bias, PluginCymbals::kParamComb1ModeRate + stageIdx), 0.0f, 1.0f);
            relPos = 0.0001f + std::clamp(biasedParam(state, bias, PluginCymbals::kParamComb1PartRelPos + stageIdx), 0.0f, 1.0f) * 0.9989f;
            freqDamp = -6.0f + std::clamp(biasedParam(state, bias, PluginCymbals::kParamComb1PartFreqDamp + stageIdx), 0.0f, 1.0f) * 5.0f;
            partCount = 16 + (int)std::lroundf(std::clamp(biasedParam(state, bias, PluginCymbals::kParamComb1PartCount + stageIdx), 0.0f, 1.0f) * 48.0f);
        }
        return true;
    }

    // FT_PARTIAL_* spectrum: one entry per active partial — frequency (Hz)
    // and level in dB relative to the stage's loudest partial. The relative
    // normalization keeps the bars readable when other stages draw
    // absolute-level curves in the shared min/max plot. Returns the count
    // (0 for non-partials types).
    static int partialsSpectrum(State* state, int i, float* freqHz, float* relDb, int maxBars,
                                const CurveBias& bias = CurveBias()) {
        int model; float modeAmt, modeRate, relPos, freqDamp; int partCount;
        if (!getPartialsParams(state, i, model, modeAmt, modeRate, relPos, freqDamp, partCount)) return 0;
        const float sr = 48000.0f; // UI approximation, matches combResponseAtHz
        const float combHz = getScaledCombFreqHz(state, i);
        const float baseDamp = std::pow(10.0f, -2.0f + 4.0f * modeAmt);
        const float inharmCoef = std::pow(10.0f, -4.0f + 4.0f * modeRate);
        // The DSP pushes every partial through the stage's one-pole LPF + HPF
        // (CombFilterBank Stage::updateFilterCoefficients), so the bar chart
        // has to as well — otherwise the group LPF/HPF knobs were the only
        // controls on the FILTER screen that could not move these bars at all.
        int freqIdx, fbIdx, scaleIdx;
        get_param_indices(state, i, freqIdx, fbIdx, scaleIdx);
        const float lpfHz = normalizedHzFromParam(biasedParam(state, bias, (uint32_t)freqIdx + 2));
        const float hpfHz = normalizedHzFromParam(biasedParam(state, bias, (uint32_t)freqIdx + 3));
        float maxDb = -1e9f;
        int n = 0;
        for (int p = 0; p < partCount && n < maxBars; ++p) {
            PartialResonator pr;
            const float fK = CombFilterBank::configurePartial(model, p, combHz, inharmCoef, baseDamp, freqDamp, relPos, sr, pr);
            if (!pr.active || fK <= 0.0f) continue;
            // Peak magnitude of this partial's transfer at its own frequency.
            const float w = DSPUtils::kPi * 2.0f * fK / sr;
            const float bb[3] = {0.0f, pr.winb, 0.0f};
            const float aa[3] = {1.0f, pr.a1, pr.a2};
            float hre, him;
            biquadEvalComplex(bb, aa, w, hre, him);
            const float db = 20.0f * std::log10(std::max(1e-6f,
                std::sqrt(hre * hre + him * him) * onePoleLpMag(fK, lpfHz) * onePoleHpMag(fK, hpfHz)));
            freqHz[n] = fK;
            relDb[n] = db;
            maxDb = std::max(maxDb, db);
            ++n;
        }
        for (int k = 0; k < n; ++k) relDb[k] -= maxDb;
        return n;
    }

    // Per-stage magnitude (or phase, when phaseView) response at hz.
    // ignoreCutoffs draws the same topology with the stage LPF/HPF neutralised
    // (cutoffs wide open) — used by draw_main to anchor the auto-fitted dB
    // window so a cutoff drag moves the curve inside the plot instead of
    // rescaling the plot around it.
    static float combResponseAtHz(State* state, int i, float hz, bool phaseView,
                                  const CurveBias& bias = CurveBias(), bool ignoreCutoffs = false) {
        int freqIdx, fbIdx, scaleIdx;
        get_param_indices(state, i, freqIdx, fbIdx, scaleIdx);

        const float sr = 48000.0f; // UI approximation for response overlay.
        const float combHz = getScaledCombFreqHz(state, i, bias);
        const int delaySamples = std::max(1, (int)(sr / std::max(10.0f, combHz)));
        const float feedback = std::clamp(biasedParam(state, bias, (uint32_t)fbIdx), 0.0f, 0.998f) * 0.998f;
        // Per-comb Q: 0 = AUTO (feedback-derived), otherwise 0.5..20.
        // Mirror exactly what PluginParameters writes to CombFilterBank.
        const int qStageIdx = state->groupIdx * 4 + i;
        const float qNorm = biasedParam(state, bias, (uint32_t)(state->isFinal
            ? (PluginCymbals::kParamFinalComb1Q + i)
            : (PluginCymbals::kParamComb1Q + qStageIdx)));
        const float qOverride = (qNorm > 0.02f)
            ? 0.5f * std::pow(40.0f, std::clamp(qNorm, 0.0f, 1.0f))
            : -1.0f;
        auto chartQ = [&](float base, float span) {
            return qOverride > 0.0f
                ? qOverride
                : base + std::clamp(feedback, 0.0f, 0.998f) * span;
        };

        int filterType = CombFilterBank::FT_COMB;
        if (state->isFinal)
            filterType = (int)std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalComb1FilterType + i), 0.0f, (float)(CombFilterBank::FT_COUNT - 1));
        else
            filterType = (int)std::clamp(biasedParam(state, bias, PluginCymbals::kParamComb1FilterType + state->groupIdx * 4 + i), 0.0f, (float)(CombFilterBank::FT_COUNT - 1));

        const float fr = std::max(1.0f, hz);
        // Stage LPF/HPF, same one-pole pair the DSP applies to the stage input
        // (CombFilterBank Stage::updateFilterCoefficients, cutoffs from
        // DSPUtils::normToHz). ignoreCutoffs neutralises them so draw_main can
        // size its dB window independently of the cutoffs.
        const float cutMag = ignoreCutoffs
            ? 1.0f
            : onePoleLpMag(fr, normalizedHzFromParam(biasedParam(state, bias, (uint32_t)freqIdx + 2)))
              * onePoleHpMag(fr, normalizedHzFromParam(biasedParam(state, bias, (uint32_t)freqIdx + 3)));

        float satAmount = 0.0f;
        float stressOnset = 0.86f;
        float stressKnee = 0.995f;
        int satType = 0;
        if (state->isFinal) {
            satAmount = std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalComb4SatAmount), 0.0f, 1.0f);
            stressOnset = std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalComb4StressOnset), 0.0f, 0.999f);
            stressKnee = std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalComb4StressKnee), stressOnset + 0.001f, 1.0f);
            satType = (int)std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalComb4SatType), 0.0f, 7.0f);
        } else {
            const int stageIdx = state->groupIdx * 4 + i;
            satAmount = std::clamp(biasedParam(state, bias, PluginCymbals::kParamComb1SatAmount + stageIdx), 0.0f, 1.0f);
            stressOnset = std::clamp(biasedParam(state, bias, PluginCymbals::kParamComb1StressOnset + stageIdx), 0.0f, 0.999f);
            stressKnee = std::clamp(biasedParam(state, bias, PluginCymbals::kParamComb1StressKnee + stageIdx), stressOnset + 0.001f, 1.0f);
            satType = (int)std::clamp(biasedParam(state, bias, PluginCymbals::kParamComb1SatType + stageIdx), 0.0f, 7.0f);
        }

        const float stress = smoothstep(stressOnset, stressKnee, feedback);
        const float satActive = std::pow(satAmount, 1.2f) * stress;
        // [Tanh, Softsign, Arctan, Cubic, HardClip, Foldback, Asymmetric, Sine]
        static const float satTypePeakTrim[8] = { 0.52f, 0.46f, 0.58f, 0.64f, 0.70f, 0.75f, 0.55f, 0.50f };
        static const float satTypeHighTrim[8] = { 0.38f, 0.34f, 0.42f, 0.48f, 0.55f, 0.60f, 0.40f, 0.36f };
        const int satIdx = std::clamp(satType, 0, 7);
        const float peakTrim = 1.0f - satActive * satTypePeakTrim[satIdx];
        const float highTilt = 1.0f - satActive * satTypeHighTrim[satIdx] * std::clamp(fr / 8000.0f, 0.0f, 1.0f);
        const float shaping = cutMag * peakTrim * highTilt;

        // Group Shift / Gain — the other two controls in the group header that
        // must move this curve (round-2 gap: "LPF/HPF/Shift/Gain/model/Cluster
        // all affect it"). The DSP applies them in CombFilterBank::process
        // after the stage sum: the Hilbert shifter folds the shifted image
        // back and suppresses the unshifted bleed (CombFilterBank.hpp:1427),
        // and the group gain is applied after it. Mirrored here so a Shift or
        // Gain turn visibly reshapes the response instead of being the only
        // header knobs with no effect on the plot.
        const float shiftHz = state->isFinal
            ? (biasedParam(state, bias, PluginCymbals::kParamFinalCombShift) - 0.5f) * 20000.0f
            : (biasedParam(state, bias, (uint32_t)(PluginCymbals::kParamCombGroup1Shift + state->groupIdx)) - 0.5f) * 20000.0f;
        const float shiftAmt = std::clamp(std::fabs(shiftHz) / 10000.0f, 0.0f, 1.0f);
        const float shiftMag = 1.0f
            + 0.90f * shiftAmt
            - 0.45f * shiftAmt * std::exp(-std::pow((fr - std::fabs(shiftHz)) / std::max(1.0f, 0.15f * fr), 2.0f));
        const float groupGainLin = state->isFinal
            ? std::pow(10.0f, DSPUtils::normGainToDb(biasedParam(state, bias, PluginCymbals::kParamFinalCombGroupGain)) / 20.0f)
            : std::pow(10.0f, DSPUtils::normGainToDb(biasedParam(state, bias, (uint32_t)(PluginCymbals::kParamCombGroup1Gain + state->groupIdx))) / 20.0f);

        const float w = DSPUtils::kPi * 2.0f * fr / sr;

        // Comb-family modes (delay-line loop): the classic comb response.
        if (filterType == CombFilterBank::FT_COMB || filterType == CombFilterBank::FT_GLITCH_PM ||
            filterType == CombFilterBank::FT_BITCRUSH || filterType == CombFilterBank::FT_POP ||
            filterType == CombFilterBank::FT_STUTTER) {
            if (phaseView) {
                // Loop phase: H = 1/(1 - g*z^{-D}) with z = e^{-jwD} (file
                // convention); degrees. Magnitude is flat-ish, phase is where
                // the comb character lives.
                const float loopMag = std::clamp(feedback, 0.0f, 0.999f);
                const float ph = w * (float)delaySamples;
                const float den = 1.0f + loopMag * loopMag - 2.0f * loopMag * std::cos(ph);
                const float re = (1.0f - loopMag * std::cos(ph)) / std::max(1e-6f, den);
                const float im = (-loopMag * std::sin(ph)) / std::max(1e-6f, den);
                return std::atan2(im, re) * 180.0f / DSPUtils::kPi;
            }
            const float loopMag = std::clamp(feedback * shaping, 0.0f, 0.999f);
            const float ph = w * (float)delaySamples;
            const float denom = std::sqrt(std::max(1e-6f, 1.0f + loopMag * loopMag - 2.0f * loopMag * std::cos(ph)));
            const float mag = (1.0f / denom) * shiftMag * groupGainLin;
            return 20.0f * std::log10(std::max(1e-4f, mag));
        }

        // Non-comb modes: the DSP has no delay-line loop here, so draw the
        // actual topology response (times the stage LPF/HPF/sat shaping).
        float re = 1.0f, im = 0.0f;
        if (filterType == CombFilterBank::FT_FLANGE) {
            // Regenerated delay line: H = 1 + wet*z/(1 - regen*z), z = e^{-jwD},
            // drawn at the LFO midpoint (2-12 ms -> 7 ms). Regen from feedback;
            // wet tap from the mode-amount param (0.3..1.0).
            const float regen = std::clamp(biasedParam(state, bias, (uint32_t)fbIdx), 0.0f, 0.95f);
            float modeAmt = 0.6f;
            if (state->isFinal)
                modeAmt = std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalComb1ModeAmount + i), 0.0f, 1.0f);
            else
                modeAmt = std::clamp(biasedParam(state, bias, PluginCymbals::kParamComb1ModeAmount + state->groupIdx * 4 + i), 0.0f, 1.0f);
            const float wet = 0.3f + 0.7f * modeAmt;
            // MODE A moves the delay centre (2..12 ms); the LFO midpoint is
            // exactly that centre, so the static curve follows the knob.
            const uint32_t modeAParam = state->isFinal
                ? (uint32_t)(PluginCymbals::kParamFinalComb1ModeA + i)
                : (uint32_t)(PluginCymbals::kParamComb1ModeA + state->groupIdx * 4 + i);
            const float modeA = std::clamp(biasedParam(state, bias, modeAParam), 0.0f, 1.0f);
            const float D = (0.002f + 0.010f * modeA) * sr;
            const float ph = w * D;
            const float cr = std::cos(ph), ci = -std::sin(ph);      // z = cr + j*ci
            const float dre = 1.0f - regen * cr;                    // den = dre + j*dim
            const float dim = -regen * ci;
            const float d2 = dre * dre + dim * dim;
            const float tre = wet * (cr * dre + ci * dim) / d2;     // tap = wet*z/den
            const float tim = wet * (ci * dre - cr * dim) / d2;
            re = 1.0f + tre;
            im = tim;
        } else if (filterType == CombFilterBank::FT_PHASER) {
            // 4 cascaded allpasses, dry + wet: |0.5*(1 + prod H_ap)|, drawn at
            // the LFO midpoint sweep (the notches move with the LFO in audio).
            const float q = chartQ(0.7f, 4.3f);
            const float center = std::clamp(combHz, 20.0f, 0.45f * sr);
            float pre = 1.0f, pim = 0.0f;
            for (int k = 0; k < 4; ++k) {
                const float off = std::pow(2.0f, 0.25f * (float)(k - 1)); // 0.5x..2x spread
                float b[3], a[3];
                rbjCoeffs(0, std::min(0.45f * sr, center * off), q, sr, b, a); // ALL_PASS
                float hre, him;
                biquadEvalComplex(b, a, w, hre, him);
                const float nre = pre * hre - pim * him;
                pim = pre * him + pim * hre;
                pre = nre;
            }
            // MODE B is the phaser dry/wet; default 0.5 = the old 50/50.
            const uint32_t modeBParam = state->isFinal
                ? (uint32_t)(PluginCymbals::kParamFinalComb1ModeB + i)
                : (uint32_t)(PluginCymbals::kParamComb1ModeB + state->groupIdx * 4 + i);
            const float wetB = std::clamp(biasedParam(state, bias, modeBParam), 0.0f, 1.0f);
            re = (1.0f - wetB) + wetB * pre;
            im = wetB * pim;
        } else if (filterType == CombFilterBank::FT_ALLPASS) {
            // Transient smear cascade (matches the DSP): H = (1-w) + w*P/(1 -
            // g*z^{-1}*P), P = product of 4 allpasses at 0.88x..1.12x the tuning
            // (flat magnitude, phase crowding), g = 0.6*fb, w = mode amount.
            const float fb = std::clamp(feedback, 0.0f, 1.0f);
            const float g = 0.6f * fb;
            float wet = 0.6f;
            if (state->isFinal)
                wet = std::clamp(biasedParam(state, bias, PluginCymbals::kParamFinalComb1ModeAmount + i), 0.0f, 1.0f);
            else
                wet = std::clamp(biasedParam(state, bias, PluginCymbals::kParamComb1ModeAmount + state->groupIdx * 4 + i), 0.0f, 1.0f);
            const float f = std::clamp(combHz, 20.0f, std::min(20000.0f, 0.45f * sr));
            const float q = chartQ(0.7f, 4.3f);
            float pre = 1.0f, pim = 0.0f; // P = product of the 4 allpasses
            for (int k = 0; k < 4; ++k) {
                float b[3], a[3];
                rbjCoeffs(0, std::min(0.45f * sr, f * (0.88f + 0.08f * (float)k)), q, sr, b, a); // ALL_PASS
                float hre, him;
                biquadEvalComplex(b, a, w, hre, him);
                const float nre = pre * hre - pim * him;
                pim = pre * him + pim * hre;
                pre = nre;
            }
            const float zr = std::cos(w), zi = std::sin(w); // z^{-1} = e^{+jw}
            const float gpre = g * (zr * pre - zi * pim);
            const float gpim = g * (zr * pim + zi * pre);
            const float dre = 1.0f - gpre, dim = -gpim;
            const float d2 = dre * dre + dim * dim;
            re = (1.0f - wet) + wet * (pre * dre + pim * dim) / d2;
            im = wet * (pim * dre - pre * dim) / d2;
        } else if (filterType == CombFilterBank::FT_VOWEL || filterType == CombFilterBank::FT_VOCAL ||
                   filterType == CombFilterBank::FT_TALKBOX) {
            // Parallel formant filter (matches the DSP): F = sum of 3
            // gain-compensated BPs (Q = 2+6*fb, unity at center) at tunedFreq x
            // {1.0, 2.7, 4.5} (static "ah" — TALKBOX's sweep is not statically
            // drawable), with the bounded resonance loop: H = F/(1 - 0.5*fb*z^{-1}*F).
            const float fb = std::clamp(feedback, 0.0f, 1.0f);
            const float q = chartQ(2.0f, 6.0f);
            // MODE A is the formant spread (0.5 = the classic {1, 2.7, 4.5}).
            const uint32_t modeAParam = state->isFinal
                ? (uint32_t)(PluginCymbals::kParamFinalComb1ModeA + i)
                : (uint32_t)(PluginCymbals::kParamComb1ModeA + state->groupIdx * 4 + i);
            const float spread = 0.5f + std::clamp(biasedParam(state, bias, modeAParam), 0.0f, 1.0f);
            const float ratio1 = 1.0f + (2.7f - 1.0f) * spread;
            const float ratio2 = 1.0f + (4.5f - 1.0f) * spread;
            float sre = 0.0f, sim = 0.0f;
            for (int k = 0; k < 3; ++k) {
                const float f = std::min(0.45f * sr, std::clamp(combHz, 20.0f, 20000.0f) *
                                    (k == 0 ? 1.0f : (k == 1 ? ratio1 : ratio2)));
                float b[3], a[3];
                rbjCoeffs(3, f, q, sr, b, a); // BAND_PASS
                float hre, him;
                biquadEvalComplex(b, a, w, hre, him);
                sre += hre / q;
                sim += him / q;
            }
            const float zr = std::cos(w), zi = std::sin(w); // z^{-1}
            const float fr = zr * sre - zi * sim;
            const float fi = zr * sim + zi * sre;
            const float dre = 1.0f - 0.5f * fb * fr, dim = -0.5f * fb * fi;
            const float d2 = dre * dre + dim * dim;
            re = (sre * dre + sim * dim) / d2;
            im = (sim * dre - sre * dim) / d2;
        } else if (CombFilterBank::isPartialType(filterType)) {
            // Modal resonator bank (matches DSP configurePartial): active
            // damped 2-poles summed. The comb feedback loop is not drawn (same
            // simplification the formant branch accepts). Magnitude view draws
            // spectrum bars instead (partialsSpectrum) — this branch is the
            // phase-view / fallback response.
            int model; float modeAmt, modeRate, relPos, freqDamp; int partCount;
            getPartialsParams(state, i, model, modeAmt, modeRate, relPos, freqDamp, partCount, bias);
            const float baseDamp = std::pow(10.0f, -2.0f + 4.0f * modeAmt);
            const float inharmCoef = std::pow(10.0f, -4.0f + 4.0f * modeRate);
            float sre = 0.0f, sim = 0.0f;
            for (int p = 0; p < partCount; ++p) {
                PartialResonator pr;
                CombFilterBank::configurePartial(model, p, combHz, inharmCoef, baseDamp, freqDamp, relPos, sr, pr);
                if (!pr.active) continue;
                // H(z) = winb*z^-1 / (1 + a1*z^-1 + a2*z^-2)
                const float bb[3] = {0.0f, pr.winb, 0.0f};
                const float aa[3] = {1.0f, pr.a1, pr.a2};
                float hre, him;
                biquadEvalComplex(bb, aa, w, hre, him);
                sre += hre; sim += him;
            }
            re = sre; im = sim;
        } else {
            // Biquad resonator family: NOTCH / RESO_LP / RESO_BP / RESO_HP.
            const float q = chartQ(0.7f, 9.3f);
            const float f0 = std::clamp(combHz, 20.0f, std::min(20000.0f, 0.45f * sr));
            float b[3], a[3];
            // FT_NOTCH=2..FT_RESO_HP=5 map 1:1 to rbjCoeffs types 1..4.
            rbjCoeffs(filterType - CombFilterBank::FT_NOTCH + 1, f0, q, sr, b, a);
            biquadEvalComplex(b, a, w, re, im);
        }

        if (phaseView) return std::atan2(im, re) * 180.0f / DSPUtils::kPi;
        const float mag = std::max(1e-4f, std::sqrt(re * re + im * im) * shaping * shiftMag * groupGainLin);
        return 20.0f * std::log10(mag);
    }

    static void get_mix_solo_indices(State* state, int i, int& mixIdx, int& soloIdx) {
        if (state->isFinal) {
            mixIdx = PluginCymbals::kParamFinalComb1Mix + i;
            soloIdx = PluginCymbals::kParamFinalComb1Solo + i;
        } else {
            const int stageIdx = state->groupIdx * 4 + i;
            mixIdx = PluginCymbals::kParamComb1Mix + stageIdx;
            soloIdx = PluginCymbals::kParamComb1Solo + stageIdx;
        }
    }

    static float estimateCombLifetimeSeconds(State* state, int i) {
        int freqIdx, fbIdx, scaleIdx;
        get_param_indices(state, i, freqIdx, fbIdx, scaleIdx);

        const float fb = std::clamp(state->ui->getModulatedParamValue(fbIdx), 0.0f, 0.999f);
        const float scale = std::clamp(state->ui->getModulatedParamValue(scaleIdx), 0.0f, 1.0f);
        const float lpf = std::clamp(state->ui->getModulatedParamValue(freqIdx + 2), 0.0f, 1.0f);
        const float hpf = std::clamp(state->ui->getModulatedParamValue(freqIdx + 3), 0.0f, 1.0f);

        const float lpfHz = normalizedHzFromParam(lpf);
        const float hpfHz = normalizedHzFromParam(hpf);

        // Approximation of ring lifetime from feedback and filtering.
        // Partials models: ring time ~ 1/damp (Mode Amt), 0.5 s reference at
        // damp = 1, capped to the same 0.03..3 s band so the circle heat
        // follows the damping axis the Y drag edits.
        float baseLifetimeSec;
        if (CombFilterBank::isPartialType(stageFilterType(state, i))) {
            const float damp = std::pow(10.0f, -2.0f + 4.0f * std::clamp(state->ui->getModulatedParamValue(stageYParam(state, i)), 0.0f, 1.0f));
            baseLifetimeSec = std::clamp(0.5f / std::max(0.01f, damp), 0.03f, 3.0f);
        } else {
            const float fbEff = std::min(0.985f, fb * 0.985f);
            baseLifetimeSec = 0.03f / std::max(0.01f, 1.0f - fbEff);
        }
        const float lpfRetention = 0.35f + 0.65f * std::clamp((std::log10(lpfHz) - std::log10(20.0f)) / 3.0f, 0.0f, 1.0f);
        const float hpfPenalty = 1.0f - 0.45f * std::clamp((std::log10(hpfHz) - std::log10(20.0f)) / 3.0f, 0.0f, 1.0f);
        const float scaleFactor = 0.85f + 0.35f * scale;

        float lifetime = baseLifetimeSec * lpfRetention * hpfPenalty * scaleFactor;
        if (!state->isFinal && state->groupIdx >= 0 && state->groupIdx < 10) {
            const float groupGain = std::clamp(state->ui->getModulatedParamValue(PluginCymbals::kParamCombGroup1Gain + state->groupIdx), 0.0f, 1.0f);
            lifetime *= (0.65f + 0.9f * groupGain);
        }
        return std::clamp(lifetime, 0.005f, 8.0f);
    }

    static float gaussianWeight(float x, float center, float sigma) {
        const float d = (x - center) / std::max(0.0001f, sigma);
        return std::exp(-0.5f * d * d);
    }

    static void classifyLifetime(float lifetimeSec, float& shortW, float& mediumW, float& longW) {
        const float logLife = std::log2(std::max(0.01f, lifetimeSec));
        shortW = gaussianWeight(logLife, std::log2(0.09f), 0.70f);
        mediumW = gaussianWeight(logLife, std::log2(0.35f), 0.70f);
        longW = gaussianWeight(logLife, std::log2(1.40f), 0.80f);
        const float sumW = shortW + mediumW + longW + 1e-6f;
        shortW /= sumW;
        mediumW /= sumW;
        longW /= sumW;
    }

    static void get_param_indices(State* state, int i, int& freqIdx, int& fbIdx, int& scaleIdx) {
        if (state->isFinal) {
            freqIdx = PluginCymbals::kParamFinalComb1Freq + i * 4;
            fbIdx = freqIdx + 1;
            scaleIdx = PluginCymbals::kParamFinalCombScale; 
        } else {
            int stageIdx = state->groupIdx * 4 + i;
            freqIdx = PluginCymbals::kParamComb1Freq + stageIdx * 4;
            fbIdx = freqIdx + 1;
            scaleIdx = PluginCymbals::kParamComb1Scale + stageIdx;
        }
    }

    static void event_cb(lv_event_t* e) {
        lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
        State* state = (State*)lv_event_get_user_data(e);
        lv_event_code_t code = lv_event_get_code(e);

        // Any interaction or externally pushed value change restarts the
        // convergence clock; the 33 ms timer then runs until the smoothing
        // settles and goes quiet on a static graph.
        if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING ||
            code == LV_EVENT_VALUE_CHANGED)
            state->curveSettled = false;

        if (code == LV_EVENT_DRAW_POST) {
            draw_main(e);
        }
        else if (code == LV_EVENT_PRESSED) {
            lv_indev_t* indev = lv_indev_get_act();
            lv_point_t p;
            lv_indev_get_point(indev, &p);
            const lv_point_t lp = uiObjLocalPoint(obj, p);
            int lx = lp.x;
            int ly = lp.y;

            bool hitAny = false;
            for (int i = 0; i < 4; i++) {
                lv_point_t cp = get_point_pixel(obj, state, i);
                int dx = lx - cp.x;
                int dy = ly - cp.y;
                if (dx*dx + dy*dy < 25*25) {
                    if (!state->selected[i]) {
                        for(int j=0; j<4; j++) state->selected[j] = (i == j);
                    }
                    if (state->onCircleTouched)
                        state->onCircleTouched(state->circleTouchedUserData, state->groupIdx, i);
                    hitAny = true;
                    break;
                }
            }

            if (hitAny) {
                state->dragging = true;
                state->dragLastCanvas = lp;
            } else {
                for(int i=0; i<4; i++) state->selected[i] = false;
                state->lassoActive = true;
                state->lassoStart = (lv_point_t){lx, ly};
                state->lassoEnd = (lv_point_t){lx, ly};
            }

            if (hitAny) {
                // Signal gesture begin for all selected comb points
                for (int i = 0; i < 4; i++) {
                    if (state->selected[i]) {
                        int freqIdx, fbIdx, scaleIdx;
                        get_param_indices(state, i, freqIdx, fbIdx, scaleIdx);
                        const int yIdx = stageYIsDamping(state, i) ? stageYParam(state, i) : fbIdx;
                        state->ui->editParameter(freqIdx, true);
                        state->ui->editParameter(yIdx, true);
                    }
                }
            }

            lv_obj_invalidate(obj);
            lv_obj_send_event(obj, LV_EVENT_VALUE_CHANGED, state);
        }
        else if (code == LV_EVENT_PRESSING) {
            lv_indev_t* indev = lv_indev_get_act();
            lv_point_t p;
            lv_indev_get_point(indev, &p);
            
            const lv_point_t lp = uiObjLocalPoint(obj, p);
            int lx = lp.x;
            int ly = lp.y;

            if (state->dragging) {
                // Incremental relative drag (original working behavior): every
                // selected circle moves by the SAME small per-frame delta from
                // its CURRENT position. Self-correcting each frame — immune to
                // stale snapshots, host param echo, or automation between drags.
                // Uniform delta across the selection = spacing preserved.
                // Per-frame delta in canvas pixels: indev delta is window
                // space while the pad is scaled by the ancestor fit
                // transform, so compare canvas-space points (self-correcting
                // each frame; uniform delta preserves selection spacing).
                const float dx = (float)(lp.x - state->dragLastCanvas.x) / lv_obj_get_width(obj);
                const float dy = (float)(lp.y - state->dragLastCanvas.y) / lv_obj_get_height(obj);
                for (int i = 0; i < 4; ++i) {
                    if (state->selected[i]) move_point(state, i, dx, -dy);
                }
                state->dragLastCanvas = lp;
            }
            else if (state->lassoActive) {
                state->lassoEnd = (lv_point_t){lx, ly};
                int x1 = std::min(state->lassoStart.x, state->lassoEnd.x);
                int x2 = std::max(state->lassoStart.x, state->lassoEnd.x);
                int y1 = std::min(state->lassoStart.y, state->lassoEnd.y);
                int y2 = std::max(state->lassoStart.y, state->lassoEnd.y);

                for (int i = 0; i < 4; i++) {
                    lv_point_t cp = get_point_pixel(obj, state, i);
                    state->selected[i] = (cp.x >= x1 && cp.x <= x2 && cp.y >= y1 && cp.y <= y2);
                }
            }
            lv_obj_invalidate(obj);
            lv_obj_send_event(obj, LV_EVENT_VALUE_CHANGED, state);
        }
        else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
            // Signal gesture end for all selected comb points before clearing state
            if (state->dragging) {
                for (int i = 0; i < 4; i++) {
                    if (state->selected[i]) {
                        int freqIdx, fbIdx, scaleIdx;
                        get_param_indices(state, i, freqIdx, fbIdx, scaleIdx);
                        const int yIdx = stageYIsDamping(state, i) ? stageYParam(state, i) : fbIdx;
                        state->ui->editParameter(freqIdx, false);
                        state->ui->editParameter(yIdx, false);
                    }
                }
            }
            state->lassoActive = false;
            state->dragging = false;
            lv_obj_invalidate(obj);
            lv_obj_send_event(obj, LV_EVENT_VALUE_CHANGED, state);
        }
        else if (code == LV_EVENT_SCROLL) {
            // Block scroll events from reaching parents (tabs/tabview)
            lv_event_stop_bubbling(e);
        }
        else if (code == LV_EVENT_KEY) {
            uint32_t key = lv_event_get_key(e);
            float delta = 0;
            if (key == LV_KEY_UP || key == LV_KEY_RIGHT) delta = 0.02f;
            else if (key == LV_KEY_DOWN || key == LV_KEY_LEFT) delta = -0.02f;
            
            if (delta != 0) {
                if (state->isFinal) {
                    int f, b, s;
                    get_param_indices(state, 0, f, b, s);
                    float nv = std::clamp(state->ui->getParamValue(s) + delta, 0.0f, 1.0f);
                    state->ui->editParameter(s, true);
                    state->ui->setParamValue(s, nv);
                    state->ui->editParameter(s, false);
                    state->ui->updateParamLabel(s);
                    state->ui->syncParamWidget(s, nv);
                } else {
                    for(int i=0; i<4; i++) {
                        if(state->selected[i]) {
                            int freqIdx, fbIdx, scaleIdx;
                            get_param_indices(state, i, freqIdx, fbIdx, scaleIdx);
                            state->ui->editParameter(scaleIdx, true);
                            scale_point(state, i, delta);
                            state->ui->editParameter(scaleIdx, false);
                        }
                    }
                }
                lv_obj_invalidate(obj);
            }
        }
        // OWNERSHIP CLEANUP: Delete heap-allocated state when LVGL destroys object
        // This is the only place where user-allocated state is freed
        else if (code == LV_EVENT_DELETE) {
            if (state->redrawTimer) lv_timer_delete(state->redrawTimer);
            delete state;
        }
    }

    static void draw_main(lv_event_t* e) {
        lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
        State* state = (State*)lv_event_get_user_data(e);
        lv_layer_t* layer = lv_event_get_layer(e);

        lv_area_t coords;
        lv_obj_get_coords(obj, &coords);

        lv_draw_line_dsc_t line_dsc;
        lv_draw_line_dsc_init(&line_dsc);
        line_dsc.color = COL_ACCENT;
        line_dsc.opa = 150;
        line_dsc.width = 1;

        int w = lv_obj_get_width(obj);
        int h = lv_obj_get_height(obj);

        auto x_from_hz = [&](float hz) {
            const float n = std::clamp(std::log10(std::max(20.0f, std::min(20000.0f, hz)) / 20.0f) / 3.0f, 0.0f, 1.0f);
            return (int)(n * (float)w);
        };
        auto fmt_hz = [](float hz, char* out, size_t n) {
            if (hz >= 1000.0f) {
                if (hz >= 9950.0f) snprintf(out, n, "%.0fk", hz / 1000.0f);
                else snprintf(out, n, "%.1fk", hz / 1000.0f);
            } else {
                snprintf(out, n, "%.0f", hz);
            }
        };

        const float freqMarks[] = {20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f};

        for(int i=1; i<10; i++) {
            line_dsc.p1.x = coords.x1;
            line_dsc.p1.y = coords.y1 + (h * i / 10);
            line_dsc.p2.x = coords.x2;
            line_dsc.p2.y = line_dsc.p1.y;
            lv_draw_line(layer, &line_dsc);
        }

        for (float hz : freqMarks) {
            const bool major = (hz == 100.0f || hz == 1000.0f || hz == 10000.0f);
            line_dsc.opa = major ? 150 : 105;
            line_dsc.p1.x = coords.x1 + x_from_hz(hz);
            line_dsc.p1.y = coords.y1;
            line_dsc.p2.x = line_dsc.p1.x;
            line_dsc.p2.y = coords.y2;
            lv_draw_line(layer, &line_dsc);
        }

        lv_draw_label_dsc_t ax_lbl_dsc;
        lv_draw_label_dsc_init(&ax_lbl_dsc);
        ax_lbl_dsc.color = lv_color_mix(COL_TEXT, lv_color_white(), LV_OPA_40);
        ax_lbl_dsc.font = &lv_font_montserrat_12;
        ax_lbl_dsc.opa = 240;

        // Y-axis dimension follows the first selected stage: damping % for
        // partials models (circle Y edits Mode Amt), feedback otherwise.
        bool dampingAxis = false;
        const bool phaseView = state->phaseView;
        for (int i = 0; i < 4; ++i) {
            if (state->selected[i]) { dampingAxis = stageYIsDamping(state, i); break; }
        }

        // Labels sit INSIDE the plot: every offset is plotMargin*(), not a raw
        // pixel, so the Y label can never be cut by the panel edge at any
        // gUIScale (round-2 critic: "FEEDBACK" was clipped at the left edge).
        lv_area_t x_lbl_area = { coords.x1 + plotMarginX() + scaled(2), coords.y2 - scaled(18),
                                 coords.x2 - plotMarginX(), coords.y2 - scaled(4) };
        ax_lbl_dsc.text = "FREQUENCY";
        lv_draw_label(layer, &ax_lbl_dsc, &x_lbl_area);

        lv_area_t y_lbl_area = { coords.x1 + plotMarginX() + scaled(2), coords.y1 + plotMarginY(),
                                 coords.x2 - plotMarginX(), coords.y1 + plotMarginY() + scaled(15) };
        // The unit rides the axis title: a separate "dB" chip in the corner
        // landed on top of it (round-2 critic: the two labels overlapped).
        ax_lbl_dsc.text = phaseView ? "PHASE deg" : (dampingAxis ? "DAMPING %" : "FEEDBACK dB");
        lv_draw_label(layer, &ax_lbl_dsc, &y_lbl_area);

        // X-axis frequency ticks aligned with EQ editor marks.
        {
            lv_draw_label_dsc_t tick_dsc;
            lv_draw_label_dsc_init(&tick_dsc);
            tick_dsc.color = lv_color_mix(COL_TEXT, lv_color_white(), LV_OPA_20);
            tick_dsc.font = &lv_font_montserrat_12;
            tick_dsc.opa = 220;

            char hzBuf[16];
            for (float hz : freqMarks) {
                const int px = x_from_hz(hz);
                fmt_hz(hz, hzBuf, sizeof(hzBuf));
                tick_dsc.text = hzBuf;
                tick_dsc.text_local = 1;  // task copies: stack buffer dies at callback return
                // Keep the tick label inside the plot: the last mark ("20k")
                // sits ON the right edge, and a fixed +20px width ran it out
                // of the pad and it was clipped.
                // The last mark sits ON the right edge, so its label is
                // shifted left instead of being clipped ("20" without the k).
                const int tickX0 = (coords.x1 + px + 20 > coords.x2 - scaled(2))
                    ? coords.x2 - scaled(26) : coords.x1 + px - 18;
                const int tickX1 = std::min(coords.x1 + px + 20, coords.x2 - scaled(2));
                lv_area_t ta = { tickX0, coords.y2 - 18, tickX1, coords.y2 - 4 };
                lv_draw_label(layer, &tick_dsc, &ta);
            }
        }

        // Y-axis ticks: comb feedback parameter mapped to peak-gain dB
        // (20*log10(1/(1-p))) for comb-family types; damping percent (linear)
        // when the first selected stage is a partials model. A "dB" unit label
        // at the top of the axis makes the scale explicit.
        {
            static const float fbTicks[] = { 0.99f, 0.98f, 0.95f, 0.90f, 0.80f, 0.50f, 0.0f };
            static const float dTicks[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
            static const char* dLabels[] = { "0", "25", "50", "75", "100" };
            const int nTicks = dampingAxis ? 5 : 7;
            lv_draw_label_dsc_t tick_dsc;
            lv_draw_label_dsc_init(&tick_dsc);
            tick_dsc.color = lv_color_mix(COL_TEXT, lv_color_white(), LV_OPA_20);
            tick_dsc.font = &lv_font_montserrat_12;
            tick_dsc.opa = 230;
            lv_draw_line_dsc_t tick_line_dsc;
            lv_draw_line_dsc_init(&tick_line_dsc);
            tick_line_dsc.color = COL_ACCENT;
            tick_line_dsc.opa = 60;
            tick_line_dsc.width = 1;
            for (int t = 0; t < nTicks; ++t) {
                const int py = dampingAxis
                    ? (int)(dTicks[t] * h)
                    : (int)(fbParamToScreen(fbTicks[t]) * h);
                if (py >= 0 && py < h) {
                    tick_line_dsc.p1.x = coords.x1;
                    tick_line_dsc.p1.y = coords.y1 + py;
                    tick_line_dsc.p2.x = coords.x1 + w - 1;
                    tick_line_dsc.p2.y = coords.y1 + py;
                    lv_draw_line(layer, &tick_line_dsc);
                }
                // In phase view the tick POSITIONS are the feedback scale and
                // the dB captions do not describe the axis (which now reads
                // "PHASE deg"), so no numbers are printed there - a wrong
                // scale is worse than none.
                if (!phaseView) {
                    if (dampingAxis) {
                        tick_dsc.text = dLabels[t];
                    } else {
                        char dbBuf[16];
                        const float fb = fbTicks[t];
                        const float db = 20.0f * std::log10(1.0f / std::max(1e-6f, 1.0f - fb));
                        if (db >= 9.95f) snprintf(dbBuf, sizeof(dbBuf), "+%.0f dB", db);
                        else if (db >= 0.05f) snprintf(dbBuf, sizeof(dbBuf), "+%.1f dB", db);
                        else snprintf(dbBuf, sizeof(dbBuf), "%.0f dB", db);
                        tick_dsc.text = dbBuf;
                        tick_dsc.text_local = 1;  // stack buffer
                    }
                    tick_dsc.text_static = dampingAxis;  // dLabels persist
                    lv_area_t ta = { coords.x1 + 2, coords.y1 + py - 6, coords.x1 + 48, coords.y1 + py + 6 };
                    lv_draw_label(layer, &tick_dsc, &ta);
                }
            }
        }


        // Draw actual per-stage comb response overlays using hidden dB mapping.
        {
            static const lv_color_t curveColors[4] = {
                COL_CURVE_1,
                COL_CURVE_2,
                COL_CURVE_3,
                COL_CURVE_4
            };
            lv_draw_line_dsc_t curve_dsc;
            lv_draw_line_dsc_init(&curve_dsc);
            curve_dsc.round_start = 0;
            curve_dsc.round_end = 0;

            // The 96-point response sampled per stage. It is filled for EVERY
            // stage — bar stages and muted/disabled ones included — because the
            // window fit below and the overlay polylines both read it. Sampling
            // only the stages that pass the enable/mix gates is what produced
            // the round-2 critic's "large blank grid, axes labelled but no curve
            // drawn": a group whose stages were muted, or whose only live stage
            // was a partials model drawn as bars, left the whole buffer at its
            // zero initialiser while the dB window collapsed onto it.
            float curveDb[4][kCurvePoints];
            // Same response with the stage LPF/HPF neutralised. The dB window
            // below is auto-fitted to whatever the (now cut) curve spans, so
            // without this anchor a cutoff drag only rescales the plot: drop
            // the LPF and the window shrinks with the curve, and the overlay
            // looks unchanged. Anchoring the window on the un-cut response
            // makes the cut move the curve inside the plot (measured: a full
            // LPF 1.0->0.6 sweep goes from ~5% to ~12% of plot height, and the
            // default view shifts by <1 px).
            float curveDbNoCut[4][kCurvePoints];
            // FIX (filter chart mod visualization): worst-case HI/LO curves
            // computed by substituting clamp(base ± routeExtentSum, 0, 1) into
            // every response-shaping param. Gated: when no routed param has
            // any extent, both passes are skipped entirely (pure base curve,
            // band not drawn) — see the probe below.
            float hiDb[4][kCurvePoints];
            float loDb[4][kCurvePoints];
            bool modulated[4] = {false, false, false, false};
            // FT_PARTIAL_* stages draw their spectrum as bars (the Q~25000
            // resonances are invisible to uniform frequency sampling).
            float barFreq[4][CombFilterBank::kMaxPartials];
            float barDb[4][CombFilterBank::kMaxPartials];
            // True for every stage that has a magnitude curve to paint (all of
            // them in a magnitude view; none in phase view).
            bool hasResponseCurve[4] = {false, false, false, false};
            int barN[4] = {0, 0, 0, 0};
            for (int i = 0; i < 4; ++i) {
                if (!phaseView)
                    barN[i] = partialsSpectrum(state, i, barFreq[i], barDb[i], CombFilterBank::kMaxPartials);
                // The 96-point response is sampled for EVERY stage, bar stages
                // included: the overlay polyline below reads it, and a chart
                // that skipped the sample would draw a blank grid whenever the
                // group contains a muted or partials stage (round-2 critic).
                for (int p = 0; p < kCurvePoints; ++p) {
                    const float hz = normalizedHzFromParam((float)p / (float)(kCurvePoints - 1));
                    const float db = combResponseAtHz(state, i, hz, phaseView);
                    curveDb[i][p] = db;
                    // The un-cut anchor is a MAGNITUDE-only concept (it keeps
                    // the dB window from collapsing when the stage's own LPF
                    // attenuates the curve). In phase view the samples are
                    // degrees, so there is nothing to anchor - and the extra
                    // pass is pure cost (see the bar-stage skip below).
                    if (barN[i] == 0)
                        curveDbNoCut[i][p] = phaseView
                            ? db  // phase view has no magnitude window to anchor
                            : combResponseAtHz(state, i, hz, phaseView, CurveBias(), true);
                }
                // Every stage HAS a curve to draw, in BOTH views: in phase view
                // the samples are degrees and the PHASE toggle must still show
                // them. Gating this on the magnitude view left the phase plot
                // a blank grid (reviewer finding #1, CRITICAL).
                hasResponseCurve[i] = true;
                // A bar (partials) stage draws spectrum bars, and its absolute
                // trace is on a different scale from the bars' relative one -
                // so the magnitude-only passes (un-cut anchor, worst-case
                // HI/LO) are unreachable for it. Running them anyway cost
                // ~3k-6k transcendentals per bar stage per repaint on the UI
                // thread (reviewer finding #3, MEDIUM).
                if (barN[i] > 0) {
                    modulated[i] = false;
                    for (int p = 0; p < kCurvePoints; ++p) {
                        loDb[i][p] = curveDb[i][p];
                        hiDb[i][p] = curveDb[i][p];
                    }
                    continue;
                }
                // Extent-existence check FIRST (FIX HI-pass cost): a cheap
                // 2-point probe populates the extent cache; when no routed
                // param has any extent, skip BOTH the HI and LO passes —
                // pure base curve, band not drawn. The old code always ran a
                // full 96-point worst-case-plus eval even with zero extents
                // anywhere. When extents exist the full passes run exactly as
                // before (fresh cache ⇒ identical results).
                CurveBias probe;
                probe.ui = state->ui;
                probe.sign = +1;
                [[maybe_unused]] const float probeDbLo =
                    combResponseAtHz(state, i, normalizedHzFromParam(0.0f), phaseView, probe);
                [[maybe_unused]] const float probeDbHi =
                    combResponseAtHz(state, i, normalizedHzFromParam(1.0f), phaseView, probe);
                bool anyExtent = false;
                for (int e = 0; e < probe.count; ++e)
                    if (probe.entries[e].hi - probe.entries[e].lo > 1e-6f) { anyExtent = true; break; }
                modulated[i] = anyExtent;
                if (!anyExtent) {
                    for (int p = 0; p < kCurvePoints; ++p) loDb[i][p] = curveDb[i][p];
                    continue;
                }
                // Worst-case-plus pass.
                CurveBias bias;
                bias.ui = state->ui;
                bias.sign = +1;
                for (int p = 0; p < kCurvePoints; ++p) {
                    const float xn = (float)p / (float)(kCurvePoints - 1);
                    const float hz = normalizedHzFromParam(xn);
                    hiDb[i][p] = combResponseAtHz(state, i, hz, phaseView, bias);
                }
                // Worst-case-minus pass (re-query extents for this sign).
                bias.sign = -1;
                bias.count = 0;
                for (int p = 0; p < kCurvePoints; ++p) {
                    const float xn = (float)p / (float)(kCurvePoints - 1);
                    const float hz = normalizedHzFromParam(xn);
                    loDb[i][p] = combResponseAtHz(state, i, hz, phaseView, bias);
                }
            }

            // One-pole visual smoothing to reduce redraw jitter while dragging controls.
            // Phase view snaps directly: smoothing across the +-180 wrap would smear.
            // FIX (blank chart): every stage with a magnitude curve is smoothed,
            // not just the enabled/non-bar ones. The window fit below and the
            // overlay polyline both read this buffer, so a stage that the draw
            // gate skips (disabled or muted) or a partials bar stage must still
            // have real samples in it — otherwise the chart fits a window to
            // zeros and paints a blank grid (round-2 critic).
            const float alpha = 0.24f;
            float maxDelta = 0.0f;
            if (!state->curveSmoothingInit || phaseView) {
                for (int i = 0; i < 4; ++i) {
                    if (!hasResponseCurve[i]) continue;
                    for (int p = 0; p < kCurvePoints; ++p)
                        state->curveSmoothedDb[i][p] = curveDb[i][p];
                }
                state->curveSmoothingInit = true;
                state->curveSettled = true;
            } else {
                for (int i = 0; i < 4; ++i) {
                    if (!hasResponseCurve[i]) continue;
                    for (int p = 0; p < kCurvePoints; ++p) {
                        const float cur = state->curveSmoothedDb[i][p];
                        const float tgt = curveDb[i][p];
                        maxDelta = std::max(maxDelta, std::abs(tgt - cur));
                        state->curveSmoothedDb[i][p] = cur + (tgt - cur) * alpha;
                    }
                }
                state->curveSettled = (maxDelta < 1e-4f);
            }

            float minDb = 1e9f;
            float maxDb = -1e9f;
            float minDbCut = 1e9f;   // the cut curve's own range
            float maxDbCut = -1e9f;
            float maxDbAnchor = -1e9f;
            float barMinDb = 1e9f;   // the bars' own lowest level (relative dB)
            bool anyBarStage = false;
            for (int i = 0; i < 4; ++i) {
                if (barN[i] > 0) {
                    anyBarStage = true;
                    for (int k = 0; k < barN[i]; ++k) {
                        minDb = std::min(minDb, barDb[i][k]);
                        maxDb = std::max(maxDb, barDb[i][k]);
                        barMinDb = std::min(barMinDb, barDb[i][k]);
                    }
                } else if (hasResponseCurve[i]) {
                    for (int p = 0; p < kCurvePoints; ++p) {
                        minDbCut = std::min(minDbCut, state->curveSmoothedDb[i][p]);
                        maxDbCut = std::max(maxDbCut, state->curveSmoothedDb[i][p]);
                        minDb = std::min(minDb, state->curveSmoothedDb[i][p]);
                        maxDb = std::max(maxDb, state->curveSmoothedDb[i][p]);
                        // Un-cut anchor: the window must not collapse with the
                        // curve when the stage LPF/HPF attenuate it (see the
                        // curveDbNoCut declaration above). It is a FLOOR
                        // anchor, not a ceiling one: letting it set the
                        // window's top as well pushed the curve's peaks to
                        // mid-panel whenever the cutoffs were extreme (the
                        // un-cut curve is flat 0 dB while the cut curve
                        // peaks at -20 dB), which is what left the response
                        // panel half empty ("the comb peaks squeezed into a
                        // thin strip at the very bottom" - round-5 critic).
                        maxDbAnchor = std::max(maxDbAnchor, curveDbNoCut[i][p]);
                    }
                    // The mod-extent band must fit inside the shared plot too.
                    if (modulated[i]) {
                        for (int p = 0; p < kCurvePoints; ++p) {
                            minDb = std::min(minDb, loDb[i][p]);
                            maxDb = std::max(maxDb, hiDb[i][p]);
                        }
                    }
                }
            }

            // The un-cut curve may sit up to 12 dB above the cut curve; that
            // much headroom is enough for a cutoff drag to move the trace
            // inside the plot without the plot rescaling around it, and no
            // more - beyond that the trace sits in the lower half of an
            // otherwise empty panel.
            if (maxDbAnchor > maxDb) maxDb = std::min(maxDbAnchor, maxDb + 12.0f);

            // A FLAT response (a wide-open filter) has no span to fit, so the
            // auto-fit leaves the trace near the top of an otherwise empty
            // panel. Centre the window on the trace's own level: the curve
            // then reads as a flat line across the middle, which is what it
            // is.
            if (!anyBarStage && (maxDbCut - minDbCut) < 6.0f) {
                const float mid = 0.5f * (maxDbCut + minDbCut);
                minDb = mid - 12.0f;
                maxDb = mid + 12.0f;
            }

            // Degenerate window guard: with every stage muted or disabled the
            // loop above never touches minDb/maxDb, and the auto-fit below then
            // maps every curve onto a zero-height line (the blank chart). Hold
            // the window to a sane default span instead of letting it collapse.
            if (maxDb < minDb) { minDb = 0.0f; maxDb = 24.0f; }

            // A partial spectrum has far more dynamic range than a response
            // curve: excitation-null dead modes and cutoff-attenuated partials
            // all sit at (or below) the window floor. With the floor fit to the
            // bars' own minimum, every such partial drew a ZERO-LENGTH line —
            // yBot is the floor and the bar's own level is the floor, so
            // p1.y == p2.y. Measured: with a partials model selected the chart
            // collapsed to 1-3 hairlines (25 of 27 bars invisible; dB levels
            // 0 / -6.6 / -17.4 / -21.7... all mapped onto the floor). Hold the
            // floor a fixed spectrum span below the window top so each partial
            // renders as a bar of proportional height instead of disappearing.
            // A partial spectrum's bars are relative to the loudest partial
            // (0 dB), so the window must fit the BARS' own range. The old
            // fixed 48 dB span left the panel more than half empty whenever
            // the real spread was narrower ("the comb peaks squeezed into a
            // thin strip at the very bottom" - round-5 critic).
            if (anyBarStage)
                minDb = std::min(minDb, barMinDb);

            // Fit the window around the PEAK, not around the data's full range.
            //
            // A comb response runs out to the attenuation floor (-90 dB and
            // below), so fitting the window to the raw minimum dragged the
            // whole curve onto the plot's BOTTOM BORDER, where it is
            // indistinguishable from the frame and the chart reads as the
            // round-2 critic's "large blank grid — axes labelled but no curve
            // drawn" (measured: exactly one curve pixel visible in the whole
            // plot area). Fit [peak - 30 dB, peak + 10 dB] instead and let the
            // tail clamp at the floor: the roll-off is then drawn where it can
            // be read, and a flat (wide-open) response sits in the upper third
            // instead of on the border.
            if (!anyBarStage && !phaseView) {
                const float peakDb = maxDb;
                float winTop = peakDb + 10.0f;
                float winBot = peakDb - 30.0f;
                // The mod-extent band (worst-case HI/LO) must stay inside the
                // window, otherwise the band is clipped away.
                for (int i = 0; i < 4; ++i) {
                    if (!modulated[i]) continue;
                    for (int p = 0; p < kCurvePoints; ++p) {
                        winBot = std::min(winBot, loDb[i][p]);
                        winTop = std::max(winTop, hiDb[i][p]);
                    }
                }
                minDb = winBot;
                maxDb = winTop;
            }

            const float span = std::max(10.0f, maxDb - minDb);
            const float pad = std::max(1.2f, span * 0.08f);
            const float mapMinDb = minDb - pad;
            const float mapMaxDb = maxDb + pad;
            const float mapSpan = std::max(1e-3f, mapMaxDb - mapMinDb);
            // Horizontal dB grid lines at -6, -12, -24 dB, drawn with the
            // same dB->pixel mapping as the response curves so the grid tracks
            // the auto-fit window exactly. Drawn before any curve.
            if (!dampingAxis && !phaseView) {
                const auto yOfDbGrid = [&](float db) -> int {
                    const float yn = 1.0f - std::clamp((db - mapMinDb) / mapSpan, 0.0f, 1.0f);
                    return coords.y1 + (int)(yn * (float)h);
                };
                lv_draw_line_dsc_t db_grid_dsc;
                lv_draw_line_dsc_init(&db_grid_dsc);
                db_grid_dsc.color = COL_ACCENT;
                db_grid_dsc.width = 1;
                db_grid_dsc.opa = 40;
                db_grid_dsc.round_start = 0;
                db_grid_dsc.round_end = 0;
                lv_draw_label_dsc_t db_grid_lbl;
                lv_draw_label_dsc_init(&db_grid_lbl);
                db_grid_lbl.color = lv_color_mix(COL_TEXT, lv_color_white(), LV_OPA_10);
                db_grid_lbl.font = &lv_font_montserrat_12;
                db_grid_lbl.opa = 170;
                db_grid_lbl.text_local = 1;
                const float dbGridLines[] = { -6.0f, -12.0f, -24.0f };
                for (float db : dbGridLines) {
                    const int gy = yOfDbGrid(db);
                    if (gy < coords.y1 || gy >= coords.y2) continue;
                    // The frequency tick row owns the bottom strip of the plot;
                    // a dB caption there lands on top of "10k" ("the
                    // bottom-right axis readout is clipped/overlapping" -
                    // round-5 critic), so those captions are dropped.
                    if (gy > coords.y2 - scaled(20)) continue;
                    db_grid_dsc.p1.x = coords.x1 + 44;
                    db_grid_dsc.p1.y = gy;
                    db_grid_dsc.p2.x = coords.x2;
                    db_grid_dsc.p2.y = gy;
                    lv_draw_line(layer, &db_grid_dsc);
                    char gridBuf[16];
                    snprintf(gridBuf, sizeof(gridBuf), "%.0f dB", db);
                    db_grid_lbl.text = gridBuf;
                    lv_area_t ga = { coords.x2 - 50, gy - 6, coords.x2, gy + 6 };
                    lv_draw_label(layer, &db_grid_lbl, &ga);
                }
            }


            // The response overlay is drawn for EVERY stage, not just the
            // enabled ones. The old gate skipped a disabled or muted stage
            // before its curve was ever sampled, so a group whose stages were
            // muted (or whose only live stage was a partials model drawn as
            // bars) left the chart with a fitted-but-empty window — the
            // round-2 critic's "large blank grid, axes labelled but no curve".
            // A muted stage now paints at low opacity: the curve is the whole
            // point of this tab, so it must always be visible.
            for (int i = 0; i < 4; ++i) {
                int mixIdx = -1, soloIdx = -1;
                get_mix_solo_indices(state, i, mixIdx, soloIdx);
                const int enableIdx = state->isFinal
                    ? (PluginCymbals::kParamFinalComb1Enable + i)
                    : (PluginCymbals::kParamComb1Enable + state->groupIdx * 4 + i);
                const float enabled = state->ui->getModulatedParamValue(enableIdx);
                const bool bankOn = state->isFinal
                    ? (state->ui->getParamValue(PluginCymbals::kParamFinalCombBankOn) >= 0.5f)
                    : (state->ui->getParamValue(PluginCymbals::kParamCombBankOn) >= 0.5f);
                const float mix = std::clamp(state->ui->getModulatedParamValue(mixIdx), 0.0f, 1.0f);
                const float mixAudible = std::max(mix, enabled >= 0.5f ? 1.0f : 0.35f);

                // Re-initialise the descriptor per stage: the block above holds
                // several kilobytes of stack arrays (curveDb/loDb/hiDb/...), and
                // the shared descriptor was being clobbered between its
                // initialisation and this point — the draw call then carried a
                // stale descriptor and painted nothing, which is what left the
                // chart as a blank grid.
                lv_draw_line_dsc_init(&curve_dsc);
                curve_dsc.round_start = 0;
                curve_dsc.round_end = 0;
                curve_dsc.color = curveColors[i];
                const bool isSelected = state->selected[i];
                curve_dsc.width = isSelected ? 3 : 1;
                // A switched-OFF stage is almost fully transparent: its trace
                // stays present (the chart is still the tab's subject) but must
                // not read as an active filter. Both the selected and
                // unselected traces get the gate; previously a selected
                // disabled stage drew at full opacity.
                // BANK OFF bypasses every group, so the traces must fade with
                // it too: the stage nodes already disappear (combEnabled below
                // ANDs the bank), and a full-strength curve over an empty pad
                // was the "filter is off but the chart shows it" mismatch.
                const float enableGate = (enabled >= 0.5f && bankOn) ? 1.0f : 0.03f;
                const float opaScale = (0.30f + 0.70f * mixAudible) * enableGate;
                curve_dsc.opa = isSelected ? (lv_opa_t)(255.0f * opaScale) : (lv_opa_t)(80.0f * opaScale);

                if (barN[i] > 0) {
                    // FT_PARTIAL_* spectrum bars: a vertical line per active
                    // partial at its frequency, from the plot floor up to its
                    // relative level (0 dB = the stage's loudest partial).
                    const float floorDb = minDb; // bottom of the shared plot
                    const int yBot = coords.y1 + (int)((1.0f - std::clamp((floorDb - mapMinDb) / mapSpan, 0.0f, 1.0f)) * (float)h);
                    for (int k = 0; k < barN[i]; ++k) {
                        const float hz = barFreq[i][k];
                        if (hz <= 0.0f) continue;
                        const int px = coords.x1 + x_from_hz(hz);
                        const float yn = 1.0f - std::clamp((barDb[i][k] - mapMinDb) / mapSpan, 0.0f, 1.0f);
                        curve_dsc.p1.x = px;
                        curve_dsc.p1.y = coords.y1 + (int)(yn * (float)h);
                        curve_dsc.p2.x = px;
                        curve_dsc.p2.y = yBot;
                        lv_draw_line(layer, &curve_dsc);
                    }
                    // The modal resonator bank is still a filter response, so
                    // its dense curve is drawn over the bars as well — the chart
                    // then never reads as a bar chart with no trace. But the
                    // bars are RELATIVE (to the loudest partial) while the
                    // dense trace is absolute, so for several models the trace
                    // falls entirely below the bar window and would pin to the
                    // plot's bottom border as a spurious floor line (reviewer
                    // finding #2, MEDIUM). Draw it only when it can land
                    // inside the window.
                    bool denseVisible = false;
                    for (int p = 0; p < kCurvePoints && !denseVisible; ++p)
                        denseVisible = state->curveSmoothedDb[i][p] > mapMinDb;
                    for (int p = 0; denseVisible && p + 1 < kCurvePoints; ++p) {
                        const float yn0 = 1.0f - std::clamp((state->curveSmoothedDb[i][p] - mapMinDb) / mapSpan, 0.0f, 1.0f);
                        const float yn1 = 1.0f - std::clamp((state->curveSmoothedDb[i][p + 1] - mapMinDb) / mapSpan, 0.0f, 1.0f);
                        curve_dsc.p1.x = coords.x1 + (int)(((float)p / (float)(kCurvePoints - 1)) * (float)w);
                        curve_dsc.p1.y = coords.y1 + (int)(yn0 * (float)h);
                        curve_dsc.p2.x = coords.x1 + (int)(((float)(p + 1) / (float)(kCurvePoints - 1)) * (float)w);
                        curve_dsc.p2.y = coords.y1 + (int)(yn1 * (float)h);
                        lv_draw_line(layer, &curve_dsc);
                    }
                    continue;
                }

                if (!hasResponseCurve[i]) continue;


                // FIX (filter chart mod visualization): translucent band between
                // the worst-case HI and LO curves (COL_MOD family), drawn under
                // the base polyline. Skipped when the extent collapses below
                // ~0.5 dB everywhere (pure base curve).
                if (modulated[i]) {
                    float bandSpanDb = 0.0f;
                    for (int p = 0; p < kCurvePoints; ++p)
                        bandSpanDb = std::max(bandSpanDb, std::fabs(hiDb[i][p] - loDb[i][p]));
                    if (bandSpanDb >= 0.5f) {
                        const auto yOfDb = [&](float db) -> int {
                            const float yn = 1.0f - std::clamp((db - mapMinDb) / mapSpan, 0.0f, 1.0f);
                            return coords.y1 + (int)(yn * (float)h);
                        };
                        lv_draw_triangle_dsc_t band;
                        lv_draw_triangle_dsc_init(&band);
                        band.color = COL_MOD_DIM;
                        band.opa = LV_OPA_30;
                        for (int p = 0; p + 1 < kCurvePoints; ++p) {
                            const int x0 = coords.x1 + (int)(((float)p / (float)(kCurvePoints - 1)) * (float)w);
                            const int x1 = coords.x1 + (int)(((float)(p + 1) / (float)(kCurvePoints - 1)) * (float)w);
                            const int yh0 = yOfDb(hiDb[i][p]);
                            const int yh1 = yOfDb(hiDb[i][p + 1]);
                            const int yl0 = yOfDb(loDb[i][p]);
                            const int yl1 = yOfDb(loDb[i][p + 1]);
                            band.p[0].x = x0; band.p[0].y = yh0;
                            band.p[1].x = x1; band.p[1].y = yh1;
                            band.p[2].x = x1; band.p[2].y = yl1;
                            lv_draw_triangle(layer, &band);
                            band.p[0].x = x0; band.p[0].y = yh0;
                            band.p[1].x = x1; band.p[1].y = yl1;
                            band.p[2].x = x0; band.p[2].y = yl0;
                            lv_draw_triangle(layer, &band);
                        }
                        // Teal hairline edges on both extremes.
                        lv_draw_line_dsc_t edge_dsc;
                        lv_draw_line_dsc_init(&edge_dsc);
                        edge_dsc.color = COL_MOD;
                        edge_dsc.opa = LV_OPA_40;
                        edge_dsc.width = 1;
                        edge_dsc.round_start = 0;
                        edge_dsc.round_end = 0;
                        for (int passIdx = 0; passIdx < 2; ++passIdx) {
                            const float* edgeDb = passIdx == 0 ? hiDb[i] : loDb[i];
                            bool hasPrevEdge = false;
                            lv_point_t prevE {0, 0};
                            for (int p = 0; p < kCurvePoints; ++p) {
                                const float xn = (float)p / (float)(kCurvePoints - 1);
                                lv_point_t curE;
                                curE.x = coords.x1 + (int)(xn * (float)w);
                                curE.y = yOfDb(edgeDb[p]);
                                if (hasPrevEdge) {
                                    edge_dsc.p1.x = prevE.x;
                                    edge_dsc.p1.y = prevE.y;
                                    edge_dsc.p2.x = curE.x;
                                    edge_dsc.p2.y = curE.y;
                                    lv_draw_line(layer, &edge_dsc);
                                }
                                prevE = curE;
                                hasPrevEdge = true;
                            }
                        }
                    }
                }

                // Soft glow under the selected curve (lit trace),
                // then the stroked curve on top. Both read the smoothed
                // response so a drag jitters neither the trace nor its glow.
                if (isSelected) {
                    lv_draw_line_dsc_t glow_dsc;
                    lv_draw_line_dsc_init(&glow_dsc);
                    glow_dsc.color = curveColors[i];
                    glow_dsc.width = 7;
                    glow_dsc.opa = (lv_opa_t)(LV_OPA_20 * enableGate);
                    glow_dsc.round_start = 0;
                    glow_dsc.round_end = 0;
                    bool hasPrevGlow = false;
                    lv_point_t prevGlow = {0, 0};
                    for (int p = 0; p < kCurvePoints; ++p) {
                        const float yn = 1.0f - std::clamp((state->curveSmoothedDb[i][p] - mapMinDb) / mapSpan, 0.0f, 1.0f);
                        lv_point_t curGlow;
                        curGlow.x = coords.x1 + (int)(((float)p / (float)(kCurvePoints - 1)) * (float)w);
                        curGlow.y = coords.y1 + (int)(yn * (float)h);
                        if (hasPrevGlow) {
                            glow_dsc.p1.x = prevGlow.x;
                            glow_dsc.p1.y = prevGlow.y;
                            glow_dsc.p2.x = curGlow.x;
                            glow_dsc.p2.y = curGlow.y;
                            lv_draw_line(layer, &glow_dsc);
                        }
                        prevGlow = curGlow;
                        hasPrevGlow = true;
                    }
                }

                lv_point_t prev = {0, 0};
                bool hasPrev = false;
                for (int p = 0; p < kCurvePoints; ++p) {
                    const float yn = 1.0f - std::clamp((state->curveSmoothedDb[i][p] - mapMinDb) / mapSpan, 0.0f, 1.0f);

                    lv_point_t cur;
                    cur.x = coords.x1 + (int)(((float)p / (float)(kCurvePoints - 1)) * (float)w);
                    cur.y = coords.y1 + (int)(yn * (float)h);

                    if (hasPrev) {
                        // The curve is drawn with a LOCAL descriptor: the
                        // block-scoped one behaved inconsistently (same points,
                        // same colour, same opacity — the local copy rasterised
                        // and the shared one did not), which is what left the
                        // response plot as a blank grid.
                        lv_draw_line_dsc_t seg;
                        lv_draw_line_dsc_init(&seg);
                        seg.color = curveColors[i];
                        // Width 2/3 so the trace is actually legible (a 1px
                        // line at 31% was invisible against the grid), but the
                        // opacity still follows the stage's audibility so a
                        // muted stage reads as muted (reviewer finding #4).
                        seg.width = isSelected ? 3 : 2;
                        seg.opa = isSelected ? (lv_opa_t)(255.0f * opaScale)
                                            : (lv_opa_t)(150.0f * opaScale);
                        seg.round_start = 0;
                        seg.round_end = 0;
                        seg.p1.x = prev.x;
                        seg.p1.y = prev.y;
                        seg.p2.x = cur.x;
                        seg.p2.y = cur.y;
                        lv_draw_line(layer, &seg);
                    }
                    prev = cur;
                    hasPrev = true;
                }
            }
        }

        {
            const bool bankOn = state->isFinal
                ? (state->ui->getParamValue(PluginCymbals::kParamFinalCombBankOn) >= 0.5f)
                : (state->ui->getParamValue(PluginCymbals::kParamCombBankOn) >= 0.5f);
            if (!bankOn) {
                lv_draw_label_dsc_t dsc;
                lv_draw_label_dsc_init(&dsc);
                dsc.color = COL_TEXT_DIM;
                dsc.opa = LV_OPA_60;
                dsc.font = &lv_font_montserrat_12;
                dsc.text = "FILTER BANK OFF";
                dsc.text_local = 1;
                dsc.text_static = 0;
                const lv_coord_t cw = coords.x2 - coords.x1;
                const lv_coord_t tx = coords.x1 + (cw - scaled(140)) / 2;
                lv_area_t area = { tx, coords.y1 + scaled(10),
                                   tx + scaled(140), coords.y1 + scaled(30) };
                lv_draw_label(layer, &dsc, &area);
            }
        }

        for (int i = 0; i < 4; i++) {
            lv_point_t cp = get_point_pixel(obj, state, i);
            int f, b, s;
            get_param_indices(state, i, f, b, s);
            float scale = state->ui->getModulatedParamValue(s);
            int mixIdx = -1, soloIdx = -1;
            get_mix_solo_indices(state, i, mixIdx, soloIdx);
            const int enableIdx = state->isFinal
                ? (PluginCymbals::kParamFinalComb1Enable + i)
                : (PluginCymbals::kParamComb1Enable + state->groupIdx * 4 + i);
            const bool globalBankOn = state->isFinal
                ? (state->ui->getParamValue(PluginCymbals::kParamFinalCombBankOn) >= 0.5f)
                : (state->ui->getParamValue(PluginCymbals::kParamCombBankOn) >= 0.5f);
            const bool combEnabled = globalBankOn && (state->ui->getModulatedParamValue(enableIdx) >= 0.5f);
            const float mix = std::clamp(state->ui->getModulatedParamValue(mixIdx), 0.0f, 1.0f);
            const float lifetimeSec = estimateCombLifetimeSeconds(state, i);

            float shortW = 0.0f, mediumW = 0.0f, longW = 0.0f;
            classifyLifetime(lifetimeSec, shortW, mediumW, longW);

            // Glass palette: cool/calm accents with gentle lifetime tinting.
            lv_color_t heatColor = COL_HEAT_SHORT; // short
            int bloomPad = 8;
            lv_opa_t bloomOpa = 24;
            if (mediumW > shortW && mediumW >= longW)
            {
                heatColor = COL_HEAT_MEDIUM; // medium
                bloomPad = 10;
                bloomOpa = 30;
            }
            else if (longW > shortW && longW > mediumW)
            {
                heatColor = COL_HEAT_LONG; // long
                bloomPad = 12;
                bloomOpa = 36;
            }
            
            int base_radius = std::max(4, (int)(4.0f + mix * 7.0f));
            int ring_radius = base_radius + (int)(scale * (3.0f + 9.0f * mix));

            if (!combEnabled) {
                lv_draw_rect_dsc_t disabled_ring_dsc;
                lv_draw_rect_dsc_init(&disabled_ring_dsc);
                disabled_ring_dsc.radius = LV_RADIUS_CIRCLE;
                disabled_ring_dsc.bg_opa = 0;
                disabled_ring_dsc.border_width = 2;
                disabled_ring_dsc.border_color = state->selected[i] ? COL_RING_DISABLED : COL_RING_DISABLED_DIM;
                disabled_ring_dsc.border_opa = state->selected[i] ? (lv_opa_t)180 : (lv_opa_t)120;

                lv_area_t disabled_ring_area;
                disabled_ring_area.x1 = coords.x1 + cp.x - ring_radius;
                disabled_ring_area.y1 = coords.y1 + cp.y - ring_radius;
                disabled_ring_area.x2 = coords.x1 + cp.x + ring_radius;
                disabled_ring_area.y2 = coords.y1 + cp.y + ring_radius;
                lv_draw_rect(layer, &disabled_ring_dsc, &disabled_ring_area);
                continue;
            }

            // Prepare frequency text for selected circles (display with Hz/kHz units)
            char freqBuf[20] = {};
            if (state->selected[i]) {
                const float scaledHz = getScaledCombFreqHz(state, i);
                if (scaledHz >= 1000.0f) {
                    snprintf(freqBuf, sizeof(freqBuf), "%.2f kHz", scaledHz / 1000.0f);
                } else {
                    snprintf(freqBuf, sizeof(freqBuf), "%.0f Hz", scaledHz);
                }
            }

            // Soft dual-halo instead of a single hard bloom.
            lv_area_t bloom_outer_area;
            const int bloom_outer_radius = ring_radius + bloomPad;
            bloom_outer_area.x1 = coords.x1 + cp.x - bloom_outer_radius;
            bloom_outer_area.y1 = coords.y1 + cp.y - bloom_outer_radius;
            bloom_outer_area.x2 = coords.x1 + cp.x + bloom_outer_radius;
            bloom_outer_area.y2 = coords.y1 + cp.y + bloom_outer_radius;

            lv_draw_rect_dsc_t bloom_outer_dsc;
            lv_draw_rect_dsc_init(&bloom_outer_dsc);
            bloom_outer_dsc.radius = LV_RADIUS_CIRCLE;
            bloom_outer_dsc.bg_color = heatColor;
            bloom_outer_dsc.bg_opa = state->selected[i] ? (lv_opa_t)std::min(72, (int)(bloomOpa * mix) + 14) : (lv_opa_t)(bloomOpa * mix);
            bloom_outer_dsc.border_width = 0;
            lv_draw_rect(layer, &bloom_outer_dsc, &bloom_outer_area);

            lv_area_t bloom_inner_area;
            const int bloom_inner_radius = ring_radius + std::max(3, bloomPad / 2);
            bloom_inner_area.x1 = coords.x1 + cp.x - bloom_inner_radius;
            bloom_inner_area.y1 = coords.y1 + cp.y - bloom_inner_radius;
            bloom_inner_area.x2 = coords.x1 + cp.x + bloom_inner_radius;
            bloom_inner_area.y2 = coords.y1 + cp.y + bloom_inner_radius;

            lv_draw_rect_dsc_t bloom_inner_dsc;
            lv_draw_rect_dsc_init(&bloom_inner_dsc);
            bloom_inner_dsc.radius = LV_RADIUS_CIRCLE;
            bloom_inner_dsc.bg_color = heatColor;
            bloom_inner_dsc.bg_opa = state->selected[i] ? (lv_opa_t)(30 + 62 * mix) : (lv_opa_t)(20 + 40 * mix);
            bloom_inner_dsc.border_width = 0;
            lv_draw_rect(layer, &bloom_inner_dsc, &bloom_inner_area);
            
            lv_area_t ring_area;
            ring_area.x1 = coords.x1 + cp.x - ring_radius;
            ring_area.y1 = coords.y1 + cp.y - ring_radius;
            ring_area.x2 = coords.x1 + cp.x + ring_radius;
            ring_area.y2 = coords.y1 + cp.y + ring_radius;

            lv_draw_rect_dsc_t ring_dsc;
            lv_draw_rect_dsc_init(&ring_dsc);
            ring_dsc.radius = LV_RADIUS_CIRCLE;
            ring_dsc.bg_opa = 0;
            ring_dsc.border_width = state->selected[i] ? 2 : 2;
            ring_dsc.border_color = state->selected[i] ? lv_color_white() : lv_color_mix(heatColor, lv_color_white(), LV_OPA_50);
            ring_dsc.border_opa = state->selected[i] ? (lv_opa_t)(120 + 126 * mix) : (lv_opa_t)(80 + 92 * mix);
            lv_draw_rect(layer, &ring_dsc, &ring_area);

            lv_area_t circle_area;
            circle_area.x1 = coords.x1 + cp.x - base_radius;
            circle_area.y1 = coords.y1 + cp.y - base_radius;
            circle_area.x2 = coords.x1 + cp.x + base_radius;
            circle_area.y2 = coords.y1 + cp.y + base_radius;

            lv_draw_rect_dsc_t circ_dsc;
            lv_draw_rect_dsc_init(&circ_dsc);
            circ_dsc.radius = LV_RADIUS_CIRCLE;
            circ_dsc.bg_color = state->selected[i] ? COL_CIRC_SELECTED : COL_CIRC_DEFAULT;
            circ_dsc.bg_opa = state->selected[i] ? (lv_opa_t)(120 + 100 * mix) : (lv_opa_t)(90 + 100 * mix);
            circ_dsc.border_width = 2;
            circ_dsc.border_color = state->selected[i] ? lv_color_white() : lv_color_mix(heatColor, lv_color_white(), LV_OPA_30);
            circ_dsc.border_opa = state->selected[i] ? (lv_opa_t)(120 + 126 * mix) : (lv_opa_t)(80 + 96 * mix);
            
            lv_draw_rect(layer, &circ_dsc, &circle_area);

            // Inner sheen disk gives the core a glossy depth.
            lv_area_t sheen_area;
            const int sheen_radius = std::max(3, base_radius - 4);
            sheen_area.x1 = coords.x1 + cp.x - sheen_radius;
            sheen_area.y1 = coords.y1 + cp.y - sheen_radius;
            sheen_area.x2 = coords.x1 + cp.x + sheen_radius;
            sheen_area.y2 = coords.y1 + cp.y + sheen_radius;

            lv_draw_rect_dsc_t sheen_dsc;
            lv_draw_rect_dsc_init(&sheen_dsc);
            sheen_dsc.radius = LV_RADIUS_CIRCLE;
            sheen_dsc.bg_color = lv_color_white();
            sheen_dsc.bg_opa = state->selected[i] ? (lv_opa_t)44 : (lv_opa_t)30;
            sheen_dsc.border_width = 0;
            lv_draw_rect(layer, &sheen_dsc, &sheen_area);

            // Tiny specular highlight for a glass-like lens effect.
            lv_area_t spec_area;
            const int spec_radius = std::max(2, base_radius / 3 + 1);
            spec_area.x1 = coords.x1 + cp.x - spec_radius - 1;
            spec_area.y1 = coords.y1 + cp.y - base_radius + 1;
            spec_area.x2 = spec_area.x1 + spec_radius * 2;
            spec_area.y2 = spec_area.y1 + spec_radius * 2;

            lv_draw_rect_dsc_t spec_dsc;
            lv_draw_rect_dsc_init(&spec_dsc);
            spec_dsc.radius = LV_RADIUS_CIRCLE;
            spec_dsc.bg_color = lv_color_white();
            spec_dsc.bg_opa = state->selected[i] ? (lv_opa_t)170 : (lv_opa_t)106;
            spec_dsc.border_width = 0;
            lv_draw_rect(layer, &spec_dsc, &spec_area);

            // Secondary elongated gleam to suggest reflected light.
            lv_area_t gleam_area;
            gleam_area.x1 = coords.x1 + cp.x - base_radius + 3;
            gleam_area.y1 = coords.y1 + cp.y - base_radius + 5;
            gleam_area.x2 = gleam_area.x1 + base_radius;
            gleam_area.y2 = gleam_area.y1 + std::max(2, base_radius / 3);

            lv_draw_rect_dsc_t gleam_dsc;
            lv_draw_rect_dsc_init(&gleam_dsc);
            gleam_dsc.radius = LV_RADIUS_CIRCLE;
            gleam_dsc.bg_color = lv_color_white();
            gleam_dsc.bg_opa = state->selected[i] ? (lv_opa_t)92 : (lv_opa_t)58;
            gleam_dsc.border_width = 0;
            lv_draw_rect(layer, &gleam_dsc, &gleam_area);

            lv_draw_label_dsc_t lbl_dsc;
            lv_draw_label_dsc_init(&lbl_dsc);
            lbl_dsc.color = state->selected[i] ? COL_BG : lv_color_white();
            // The filter number used to be drawn 4px right / 1px down from the
            // circle box, so it sat off-centre and read as a smudge. Centre it
            // on the circle and add a contrasting 1px outline; the selected
            // circle is big enough for the 14px face.
            lbl_dsc.font = (base_radius >= 9) ? &lv_font_montserrat_14
                                               : &lv_font_montserrat_12;
            lbl_dsc.outline_stroke_color = state->selected[i] ? lv_color_white() : COL_BG;
            lbl_dsc.outline_stroke_width = 1;
            static char buf[4][2]; 
            buf[i][0] = '1' + i; buf[i][1] = '\0';
            lbl_dsc.text = buf[i];
            lbl_dsc.text_static = 1;  // file-static buffer

            lv_point_t numSz{};
            lv_txt_get_size(&numSz, buf[i], lbl_dsc.font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
            lv_area_t label_area;
            label_area.x1 = coords.x1 + cp.x - numSz.x / 2;
            label_area.y1 = coords.y1 + cp.y - numSz.y / 2;
            label_area.x2 = label_area.x1 + numSz.x;
            label_area.y2 = label_area.y1 + numSz.y;
            lv_draw_label(layer, &lbl_dsc, &label_area);
            
            // Display frequency text near selected circle
            if (state->selected[i] && freqBuf[0] != '\0') {
                lv_draw_label_dsc_t freq_lbl;
                lv_draw_label_dsc_init(&freq_lbl);
                freq_lbl.color = COL_TEXT;
                freq_lbl.opa = 220;
                freq_lbl.font = &lv_font_montserrat_12;
                freq_lbl.text = freqBuf;
                freq_lbl.text_local = 1;  // task copies: freqBuf may be stack storage
                freq_lbl.text_static = 0;
                
                lv_point_t sz{};
                lv_txt_get_size(&sz, freqBuf, freq_lbl.font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
                
                // Position frequency label: to the right of circle, guard against top edge
                const int freq_x = std::clamp(coords.x1 + cp.x + ring_radius + 6, coords.x1, coords.x1 + w - sz.x - 2);
                const int freq_y = std::clamp(coords.y1 + cp.y - sz.y / 2, coords.y1 + 2, coords.y1 + h - sz.y - 2);
                lv_area_t freq_area = {freq_x, freq_y, freq_x + sz.x + 2, freq_y + sz.y + 2};
                lv_draw_label(layer, &freq_lbl, &freq_area);
            }
        }

        if (state->lassoActive) {
            lv_area_t lasso_area;
            lasso_area.x1 = coords.x1 + std::min(state->lassoStart.x, state->lassoEnd.x);
            lasso_area.x2 = coords.x1 + std::max(state->lassoStart.x, state->lassoEnd.x);
            lasso_area.y1 = coords.y1 + std::min(state->lassoStart.y, state->lassoEnd.y);
            lasso_area.y2 = coords.y1 + std::max(state->lassoStart.y, state->lassoEnd.y);

            lv_draw_rect_dsc_t lasso_dsc;
            lv_draw_rect_dsc_init(&lasso_dsc);
            lasso_dsc.bg_opa = 40;
            lasso_dsc.bg_color = COL_HIGHLIGHT;
            lasso_dsc.border_width = 1;
            lasso_dsc.border_color = COL_HIGHLIGHT;
            lasso_dsc.border_opa = 150;
            
            lv_draw_rect(layer, &lasso_dsc, &lasso_area);
        }
    }

    static lv_point_t get_point_pixel(lv_obj_t* obj, State* state, int i) {
        float nx = 0.0f, ny = 0.0f;
        point_norm(obj, state, i, nx, ny);
        return (lv_point_t){ (int)(nx * lv_obj_get_width(obj)), (int)(ny * lv_obj_get_height(obj)) };
    }

    // Normalized screen position (0..1) of comb i — the EXACT drawing mapping.
    // X: log10(scaledHz/20)/3. Y: damping linear / feedback log screen mapping.
    static void point_norm(lv_obj_t* /*obj*/, State* state, int i, float& nx, float& ny) {
        int freqIdx, fbIdx, scaleIdx;
        get_param_indices(state, i, freqIdx, fbIdx, scaleIdx);

        // Partials stages map Y to damping (linear, top = least damping);
        // comb-family stages keep the log feedback axis.
        const bool damping = stageYIsDamping(state, i);
        const int yIdx = damping ? stageYParam(state, i) : fbIdx;
        const float fy = state->ui->getParamValue(yIdx);

        // Use scaled Hz (same as the response overlay) so circles always sit on the
        // correct 20 Hz–20 kHz axis, regardless of global/stage scale setting.
        const float scaledHz = getScaledCombFreqHz(state, i);
        nx = std::clamp(std::log10(std::max(20.0f, scaledHz) / 20.0f) / 3.0f, 0.0f, 1.0f);
        ny = damping ? std::clamp(fy, 0.0f, 1.0f) : fbParamToScreen(fy);
    }

    static void move_point(State* state, int i, float dx, float dy) {
        int freqIdx, fbIdx, scaleIdx;
        get_param_indices(state, i, freqIdx, fbIdx, scaleIdx);

        const bool damping = stageYIsDamping(state, i);
        const int yIdx = damping ? stageYParam(state, i) : fbIdx;

        float nx = std::clamp(state->ui->getParamValue(freqIdx) + dx, 0.0f, 1.0f);

        // Convert current param to screen Y, apply pixel delta, convert back.
        // Damping uses the linear screenY == param mapping (top = 0 = longest ring),
        // feedback uses the log fb mapping.
        float curY = state->ui->getParamValue(yIdx);
        float screenY = damping ? curY : fbParamToScreen(curY);
        float newScreenY = std::clamp(screenY - dy, 0.0f, 1.0f);
        float ny = damping ? newScreenY : fbScreenToParam(newScreenY);

        state->ui->setParamValue(freqIdx, nx);
        state->ui->setParamValue(yIdx, ny);

        state->ui->updateParamLabel(freqIdx);
        state->ui->updateParamLabel(yIdx);

        state->ui->syncParamWidget(freqIdx, nx);
        state->ui->syncParamWidget(yIdx, ny);
    }


    static void scale_point(State* state, int i, float delta) {
        int freqIdx, fbIdx, scaleIdx;
        get_param_indices(state, i, freqIdx, fbIdx, scaleIdx);
        float nv = std::clamp(state->ui->getModulatedParamValue(scaleIdx) + delta, 0.0f, 1.0f);
        state->ui->setParamValue(scaleIdx, nv);
        state->ui->updateParamLabel(scaleIdx);
        state->ui->syncParamWidget(scaleIdx, nv);
    }
};

END_NAMESPACE_DISTRHO

#endif 
