#ifndef STAGE1_TAB_HPP
#define STAGE1_TAB_HPP

#include "../UIWidgets.hpp"
#include "../UIMsegEditor.hpp"
#include "../UIEQSection.hpp"
#include "../AdsrCurvePreview.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_map>

START_NAMESPACE_DISTRHO

struct Stage1Tab {
    // Per-SOUND character controls in the IMPULSES strip. The map is keyed by
    // the SOUND dropdown so host/preset sync (which sets the dropdown without
    // firing VALUE_CHANGED) can still reveal the right control.
    struct StickHeaderCtl {
        lv_obj_t* cols[13] = {};
    };
    static std::unordered_map<lv_obj_t*, StickHeaderCtl*>& stickHeaderCtlMap() {
        static std::unordered_map<lv_obj_t*, StickHeaderCtl*> m;
        return m;
    }
    static void syncStickHeader(lv_obj_t* dd) {
        if (!dd) return;
        auto it = stickHeaderCtlMap().find(dd);
        if (it == stickHeaderCtlMap().end() || it->second == nullptr) return;
        StickHeaderCtl* c = it->second;
        // Control order: 0 pulse width, 1 noise color, 2 metal spread,
        // 3 noise/metal blend, 4 hat tone, 5 square width, 6 saw curve,
        // 7 triangle skew, 8 sine drive, 9 phase, 10 grit, 11 detune,
        // 12 unison (dropdown).
        static const int kTypeControls[10][5] = {
            {8, 9, 10, 11, 12},   // SINE: DRIVE, PHASE, GRIT, DETUNE, UNISON
            {7, 9, 10, 11, 12},   // TRIANGLE: SKEW, PHASE, GRIT, DETUNE, UNISON
            {6, 9, 10, 11, 12},   // SAW: CURVE, PHASE, GRIT, DETUNE, UNISON
            {5, 9, 10, 11, 12},   // SQUARE: WIDTH, PHASE, GRIT, DETUNE, UNISON
            {0, 9, 10, 11, 12},   // PULSE: WIDTH, PHASE, GRIT, DETUNE, UNISON
            {1, -1, -1, -1, -1},  // NOISE: COLOR
            {2, -1, -1, -1, -1},  // METALLIC: SPREAD
            {3, -1, -1, -1, -1},  // NOISE+METAL: BLEND
            {4, -1, -1, -1, -1},  // HI-HAT: TONE
            {-1, -1, -1, -1, -1}, // NONE: external input only, no character controls
        };
        const int type = std::clamp((int)lv_dropdown_get_selected(dd), 0, 9);
        for (int i = 0; i < 13; ++i) {
            if (c->cols[i]) lv_obj_add_flag(c->cols[i], LV_OBJ_FLAG_HIDDEN);
        }
        for (int j = 0; j < 5; ++j) {
            const int idx = kTypeControls[type][j];
            if (idx >= 0 && idx < 13 && c->cols[idx])
                lv_obj_clear_flag(c->cols[idx], LV_OBJ_FLAG_HIDDEN);
        }
    }
    static void stickHeaderVisCb(lv_event_t* e) {
        syncStickHeader(static_cast<lv_obj_t*>(lv_event_get_target(e)));
    }
    static void stickHeaderDeleteCb(lv_event_t* e) {
        lv_obj_t* dd = static_cast<lv_obj_t*>(lv_event_get_target(e));
        auto it = stickHeaderCtlMap().find(dd);
        if (it != stickHeaderCtlMap().end()) {
            delete it->second;
            stickHeaderCtlMap().erase(it);
        }
    }
    // EXCITATION's Hardness/Contact/Ring are the STICK PITCH ADSR (attack/
    // decay/release). The pitch MSEG REPLACES that envelope when enabled
    // (StickSoundBank::process uses msegPitch when useMsegPitch is true), and
    // the MSEG is ON by default — so those three knobs are bypassed and the
    // module looked dead. Dim them while the MSEG owns the pitch and say why.
    static constexpr intptr_t kExciteGateTag = 0xE1;

    static void refreshExcitationGate(lv_obj_t* stage, const float* fParams) {
        if (stage == nullptr || fParams == nullptr) return;
        lv_obj_t* gate = nullptr;
        struct Find {
            static lv_obj_t* walk(lv_obj_t* o) {
                if (o == nullptr) return nullptr;
                if ((intptr_t)lv_obj_get_user_data(o) == kExciteGateTag) return o;
                for (uint32_t i = 0; i < lv_obj_get_child_count(o); ++i)
                    if (lv_obj_t* hit = walk(lv_obj_get_child(o, i))) return hit;
                return nullptr;
            }
        };
        gate = Find::walk(stage);
        if (gate == nullptr) return;
        const bool msegOwnsPitch = fParams[PluginCymbals::kParamStickPitchMsegOn] > 0.5f;
        for (uint32_t i = 0; i < lv_obj_get_child_count(gate); ++i) {
            lv_obj_t* col = lv_obj_get_child(gate, i);
            if (col != nullptr)
                lv_obj_set_style_opa(col, msegOwnsPitch ? LV_OPA_40 : LV_OPA_COVER, 0);
        }
    }

    // PITCH MSEG and EXCITATION are ONE module, not two cards: a single well
    // below the pitch wells with a single header row. The header's toggle is
    // the mode switch AND the module's heading (its caption names the active
    // view), so there is no second title to repeat and no wide empty bar; the
    // right-hand strike readout belongs to the excitation view and hides with
    // it. Only the body swaps (MSEG editor when ON, strike controls when OFF).
    // Keeping the header in both states is what makes the switch reachable in
    // either mode.
    struct RailLayout {
        lv_obj_t* stage = nullptr;
        lv_obj_t* pane = nullptr;
        lv_obj_t* well = nullptr;        // shared module well (header + body)
        lv_obj_t* msegBody = nullptr;    // MSEG editor: the ON body
        lv_obj_t* exciteBody = nullptr;  // strike controls: the OFF body
        lv_obj_t* desc = nullptr;        // strike readout (OFF only)
        lv_obj_t* scope = nullptr;       // impact forecast band (OFF, room only)
        lv_coord_t wellY = 0;
        bool on = true;
    };
    static std::unordered_map<lv_obj_t*, RailLayout*>& railLayoutMap() {
        static std::unordered_map<lv_obj_t*, RailLayout*> m;
        return m;
    }
    static void setRailBodyVisible(lv_obj_t* obj, bool visible) {
        if (obj == nullptr || !lv_obj_is_valid(obj)) return;
        if (visible) lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
        else         lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
    // The well always runs from its rail slot to the pane bottom; the active
    // body owns that whole space. Only visibility changes with the mode, so
    // there is no pair of stacked heights to keep in sync. Hidden children are
    // dropped from the well's flex flow, so the surviving body grows into the
    // full body box.
    static void applyRailLayout(RailLayout* rl) {
        if (rl == nullptr) return;
        // Mode first: visibility must follow the toggle even before the pane
        // has a measured size (the pane is 0 tall for as long as the build is
        // running, and the host can flip the mode inside that window).
        setRailBodyVisible(rl->msegBody, rl->on);
        setRailBodyVisible(rl->exciteBody, !rl->on);
        setRailBodyVisible(rl->desc, !rl->on);
        if (rl->pane == nullptr || !lv_obj_is_valid(rl->pane)) return;
        if (rl->well == nullptr || !lv_obj_is_valid(rl->well)) return;
        const lv_coord_t avail = lv_obj_get_height(rl->pane);
        if (avail <= 0) return;
        lv_coord_t h = avail - rl->wellY - scaled(2);
        // Header 26 + pads 6 + border 2 = 34, so the header never clips even
        // in a squeezed pane.
        if (h < scaled(48)) h = scaled(48);
        lv_obj_set_pos(rl->well, scaled(1), rl->wellY);
        lv_obj_set_height(rl->well, h);
        // Height-responsive excitation body: the knob band is a fixed 76px
        // and the forecast band takes the rest. Computed from the well height
        // (pad_top 4 + header 26 + gap 6 + pad_bottom 2 + border 2 = 40), NOT
        // from lv_obj_get_height(exciteBody) — a child's layout still holds
        // the previous pass' values at this point. Drop the forecast only when
        // the body is too short to give it a usable strip (the 1280x720 body
        // leaves ~23px; the taller 1.5x/2x bodies leave 75px+).
        const lv_coord_t bodyH = h - scaled(40);
        setRailBodyVisible(rl->scope, bodyH >= scaled(90));
    }
    static void refreshMsegLayout(lv_obj_t* stage, const float* fParams) {
        if (stage == nullptr || fParams == nullptr) return;
        auto it = railLayoutMap().find(stage);
        if (it == railLayoutMap().end() || it->second == nullptr) return;
        it->second->on = fParams[PluginCymbals::kParamStickPitchMsegOn] > 0.5f;
        applyRailLayout(it->second);
    }

    // Deferred rail-knob pack (called once from the footer tick, post-build):
    // packs the 10 rail knobs with the rail spec constants. Runs strictly
    // after construction — never inline: packing mid-build silently misfires
    // on the half-built tree, and mid-build layouts risk re-entrancy.
    static void packRailWidgets(lv_obj_t** widgets) {
        static const int kPs[] = {
            PluginCymbals::kParamStickDirect, PluginCymbals::kParamStickBus, PluginCymbals::kParamVolume,
            PluginCymbals::kParamStickAmpAttack, PluginCymbals::kParamStickAmpDecay,
            PluginCymbals::kParamStickAmpSustain, PluginCymbals::kParamStickAmpRelease,
            PluginCymbals::kParamStickPitchAmount, PluginCymbals::kParamStickPitchTimer,
            PluginCymbals::kParamStickPitchMsegDuration,
            PluginCymbals::kParamMasterTuning,
        };
        for (int p : kPs) {
            if (widgets[p] == nullptr || !lv_obj_is_valid(widgets[p])) continue;
            lv_obj_t* kc = lv_obj_get_parent(widgets[p]);
            lv_obj_t* col = (kc != nullptr && lv_obj_is_valid(kc)) ? lv_obj_get_parent(kc) : nullptr;
            if (col == nullptr || !lv_obj_is_valid(col)) continue;
            lv_obj_set_style_pad_row(col, scaled(0), 0);
            uiPackKnob(widgets[p], scaled(44), scaled(0), scaled(48));
        }
    }
    static constexpr int kStickEqBands = 5;
    static int eqFreqParam(int b) { return PluginCymbals::kParamEQ1Freq + b * 4; }
    static int eqGainParam(int b) { return eqFreqParam(b) + 1; }
    static int eqQParam(int b) { return eqFreqParam(b) + 2; }
    static int eqTypeParam(int b) { return eqFreqParam(b) + 3; }
    static int eqModelParam(int b) { return PluginCymbals::kParamEQ1Model + b * 3; }

    // IMPULSES dice #1: every rail control + SOUND type + pitch MSEG. The EQ
    // graph has its own dice so the two jobs stay separate.
    static void randomizeImpulseScreen(AbstractCymbalsUI* ui) {
        if (ui == nullptr) return;
        auto rnd = [] { return (float)std::rand() / (float)RAND_MAX; };
        auto put = [&](uint32_t idx, float v) {
            ui->editParameter(idx, true);
            ui->setParamValue(idx, v);
            ui->editParameter(idx, false);
            ui->updateParamLabel(idx);
            ui->syncParamWidget(idx, v);
        };
        put(PluginCymbals::kParamStickType, (float)(std::rand() % 7));
        put(PluginCymbals::kParamStickDirect, 0.15f + 0.70f * rnd());
        put(PluginCymbals::kParamStickBus, 0.30f + 0.70f * rnd());
        put(PluginCymbals::kParamVolume, 0.55f + 0.40f * rnd());
        put(PluginCymbals::kParamStickAmpAttack, 0.01f + 0.30f * rnd());
        put(PluginCymbals::kParamStickAmpDecay, 0.05f + 0.70f * rnd());
        put(PluginCymbals::kParamStickAmpSustain, 0.05f + 0.90f * rnd());
        put(PluginCymbals::kParamStickAmpRelease, 0.02f + 0.50f * rnd());
        put(PluginCymbals::kParamStickPitchAmount, 0.10f + 0.90f * rnd());
        put(PluginCymbals::kParamStickPitchTimer, 0.05f + 0.60f * rnd());
        put(PluginCymbals::kParamStickPitchMsegDuration, 0.05f + 0.60f * rnd());
        put(PluginCymbals::kParamStickPitchMsegOn, (std::rand() % 2) ? 1.0f : 0.0f);
        for (int i = 0; i < 8; ++i)
            put((uint32_t)PluginCymbals::kParamStickPitchMsegPoint0Value + i, 0.05f + 0.90f * rnd());
        // MSEG point X positions are the segment times, so they MUST rise
        // monotonically: independent random values made the polyline double
        // back on itself (point 3 before point 2, etc.). Jitter each point
        // around its even spacing, clamped inside the previous point and
        // leaving every later point room.
        float msegPrevT = 0.0f;
        for (int i = 0; i < 6; ++i) {
            const float base = (float)(i + 1) / 7.0f;
            const float lo = std::max(msegPrevT + 0.02f, base - 0.06f);
            const float hi = std::max(lo + 0.001f, std::min(0.99f, base + 0.06f));
            const float t = lo + (hi - lo) * rnd();
            put((uint32_t)PluginCymbals::kParamStickPitchMsegPoint1Time + i,
                std::clamp(t, 0.0f, 0.99f));
            msegPrevT = t;
        }
        for (int i = 0; i < 7; ++i)
            put((uint32_t)PluginCymbals::kParamStickPitchMsegPoint0Curve + i, 0.10f + 0.80f * rnd());
        put(PluginCymbals::kParamStickPitchAttack, rnd());
        put(PluginCymbals::kParamStickPitchDecay, rnd());
        put(PluginCymbals::kParamStickPitchRelease, rnd());
        // "All of the screen" includes the EQ panel.
        randomizeStickEq(ui);
    }

    // IMPULSES dice #2: the stick EQ only (same shape as the final-EQ dice).
    static void randomizeStickEq(AbstractCymbalsUI* ui) {
        if (ui == nullptr) return;
        auto rnd = [] { return (float)std::rand() / (float)RAND_MAX; };
        auto put = [&](uint32_t idx, float v) {
            ui->editParameter(idx, true);
            ui->setParamValue(idx, v);
            ui->editParameter(idx, false);
            ui->updateParamLabel(idx);
            ui->syncParamWidget(idx, v);
        };
        for (int b = 0; b < kStickEqBands; ++b) {
            put((uint32_t)eqFreqParam(b), 0.05f + 0.90f * rnd());
            put((uint32_t)eqGainParam(b), 0.35f + 0.30f * rnd());
            put((uint32_t)eqQParam(b), 0.20f + 0.60f * rnd());
            put((uint32_t)eqTypeParam(b), (float)(std::rand() % 8));
            const int m = eqModelParam(b);
            put((uint32_t)m, (float)(std::rand() % (int)UnifiedFilter::Count));
            put((uint32_t)(m + 1), (float)(std::rand() % 8));
            put((uint32_t)(m + 2), (float)(std::rand() % 4));
            put((uint32_t)PluginCymbals::kParamEQ1Sat + b, rnd());
        }
        put(PluginCymbals::kParamEQMix, 0.50f + 0.50f * rnd());
        put(PluginCymbals::kParamTiltAmount, 0.25f + 0.50f * rnd());
        put(PluginCymbals::kParamTiltPivot, 0.15f + 0.70f * rnd());
    }

    // RESET: stick EQ bands + Mix/Tilt/Pivot back to init defaults.
    static void resetStickEq(AbstractCymbalsUI* ui) {
        if (ui == nullptr) return;
        auto putDefault = [&](uint32_t idx) {
            const float def = ui->getParameterDefault(idx);
            ui->setParamValue(idx, def);
            ui->updateParamLabel(idx);
            ui->syncParamWidget(idx, def);
        };
        ui->editParameter((uint32_t)eqFreqParam(0), true);
        for (int b = 0; b < kStickEqBands; ++b) {
            putDefault((uint32_t)eqFreqParam(b));
            putDefault((uint32_t)eqGainParam(b));
            putDefault((uint32_t)eqQParam(b));
            putDefault((uint32_t)eqTypeParam(b));
            const int m = eqModelParam(b);
            putDefault((uint32_t)m);
            putDefault((uint32_t)(m + 1));
            putDefault((uint32_t)(m + 2));
            putDefault((uint32_t)PluginCymbals::kParamEQ1Sat + b);
        }
        putDefault(PluginCymbals::kParamEQMix);
        putDefault(PluginCymbals::kParamTiltAmount);
        putDefault(PluginCymbals::kParamTiltPivot);
        ui->editParameter((uint32_t)eqFreqParam(0), false);
    }

    static void randomScreenCb(lv_event_t* e) {
        randomizeImpulseScreen(static_cast<AbstractCymbalsUI*>(lv_event_get_user_data(e)));
    }
    static void randomEqCb(lv_event_t* e) {
        randomizeStickEq(static_cast<AbstractCymbalsUI*>(lv_event_get_user_data(e)));
    }
    static void resetEqCb(lv_event_t* e) {
        resetStickEq(static_cast<AbstractCymbalsUI*>(lv_event_get_user_data(e)));
    }

    static void build(lv_obj_t* tab, AbstractCymbalsUI* ui, UIStyles& styles,
                     lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                     lv_event_cb_t valueCb, lv_event_cb_t rollerCb,
                     lv_obj_t** /*outChart*/, lv_chart_series_t** /*outSeries*/) {

        // Tab: uniform strip on top, ROW body below
        lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_layout(tab, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_all(tab, scaled(8), 0);
        lv_obj_set_style_pad_row(tab, scaled(8), 0);

        lv_obj_t* strip = UIWidgets::createScreenStrip(tab, styles, "IMPULSES");

        // Per-SOUND character control in the strip's spare width. Only the
        // control relevant to the selected type is visible; all five are real
        // automatable/mod-routable parameters.
        auto* stickCtl = new StickHeaderCtl();
        {
            static const int kCharParams[12] = {
                PluginCymbals::kParamStickPulseWidth,
                PluginCymbals::kParamStickNoiseColor,
                PluginCymbals::kParamStickMetallicSpread,
                PluginCymbals::kParamStickNoiseMetalBlend,
                PluginCymbals::kParamStickHatTone,
                PluginCymbals::kParamStickSquareWidth,
                PluginCymbals::kParamStickSawCurve,
                PluginCymbals::kParamStickTriangleSkew,
                PluginCymbals::kParamStickSineDrive,
                PluginCymbals::kParamStickPhase,
                PluginCymbals::kParamStickGrit,
                PluginCymbals::kParamStickDetune,
            };
            static const char* const kCharTitles[12] = {
                "WIDTH", "COLOR", "SPREAD", "BLEND", "TONE",
                "WIDTH", "CURVE", "SKEW", "DRIVE", "PHASE", "GRIT", "DETUNE"
            };
            UIWidgets::ArcVisualSpec charSpec = UIWidgets::smallArcVisualSpec();
            charSpec.arcSize = scaled(18);
            charSpec.containerW = scaled(40);
            charSpec.containerH = scaled(48);
            charSpec.ringPad = scaled(2);
            charSpec.titleExtraW = scaled(8);
            charSpec.labelMarginBottom = 0;
            charSpec.valueMarginTop = 0;
            for (int i = 0; i < 12; ++i) {
                UIWidgets::createArcEx(strip, kCharParams[i], kCharTitles[i], ui, styles,
                                       widgets, labels, fParams, valueCb, charSpec, true);
                lv_obj_t* arc = widgets[kCharParams[i]];
                lv_obj_t* knobCont = arc ? lv_obj_get_parent(arc) : nullptr;
                lv_obj_t* col = knobCont ? lv_obj_get_parent(knobCont) : nullptr;
                stickCtl->cols[i] = col;
                if (col) {
                    // Compact header labels: the default adaptive title face
                    // is too wide for the 40px columns and CURVE/PHASE collide.
                    lv_obj_t* title = lv_obj_get_child(col, 0);
                    if (title) {
                        lv_obj_set_style_text_font(title, uiMicroFont(), 0);
                        lv_obj_set_style_text_letter_space(title, 0, 0);
                    }
                    if (labels[kCharParams[i]])
                        lv_obj_set_style_text_font(labels[kCharParams[i]], uiMicroFont(), 0);
                    lv_obj_add_flag(col, LV_OBJ_FLAG_HIDDEN);
                }
            }
            lv_obj_t* uniDd = UIWidgets::createCompactLabeledDropdown(
                strip, PluginCymbals::kParamStickUnison, "UNISON", "1\n2\n3\n4",
                ui, styles, widgets, fParams, rollerCb, scaled(64), scaled(28), scaled(2));
            if (uniDd) {
                uiEnableDropdownListScroll(uniDd);
                uiFixDropdownPopup(uniDd);
            }
            // The factory returns the dropdown; the visibility slot is its
            // compact control column so the UNISON title hides with it.
            stickCtl->cols[12] = uniDd ? lv_obj_get_parent(uniDd) : nullptr;
            if (stickCtl->cols[12]) lv_obj_add_flag(stickCtl->cols[12], LV_OBJ_FLAG_HIDDEN);
        }

        // SOUND SELECT lives in the strip, right-aligned by the kit spacer:
        // five rail modules needed 579px in a 492px pane (the invisible
        // PITCH/MSEG bug); the strip documents room for caller controls.
        // Pure relocation — same control, options, styles and callbacks.
        lv_obj_t* stripSndLbl = lv_label_create(strip);
        lv_label_set_text(stripSndLbl, "SOUND");
        lv_obj_add_style(stripSndLbl, &styles.title, 0);
        lv_obj_set_user_data(stripSndLbl, (void*)(intptr_t)-1);
        lv_obj_t* stickTypeDd = lv_dropdown_create(strip);
        lv_dropdown_set_options(stickTypeDd, "Sine\nTriangle\nSaw\nSquare\nPulse\nNoise\nMETALLIC\nNOISE+METAL\nHI-HAT\nNONE");
        lv_obj_set_width(stickTypeDd, scaled(240));
        lv_obj_set_height(stickTypeDd, scaled(36));
        lv_obj_set_scrollbar_mode(stickTypeDd, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(stickTypeDd, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_style(stickTypeDd, &styles.compactSelectMain, LV_PART_MAIN);
        lv_obj_add_style(stickTypeDd, &styles.compactSelectIndicator, LV_PART_INDICATOR);
        lv_obj_t* ddList = lv_dropdown_get_list(stickTypeDd);
        if (ddList != nullptr) {
            lv_obj_add_style(ddList, &styles.compactSelectListMain, LV_PART_MAIN);
            lv_obj_add_style(ddList, &styles.compactSelectListSelected, LV_PART_SELECTED);
            // Long option lists must scroll (sanctioned exemption); panels stay locked.
            uiEnableDropdownListScroll(stickTypeDd);
        }
        lv_dropdown_set_selected(stickTypeDd, (int)fParams[PluginCymbals::kParamStickType]);
        lv_obj_set_user_data(stickTypeDd, reinterpret_cast<void*>((intptr_t)PluginCymbals::kParamStickType));
        stickHeaderCtlMap()[stickTypeDd] = stickCtl;
        lv_obj_add_event_cb(stickTypeDd, rollerCb, LV_EVENT_VALUE_CHANGED, ui);
        // Second listener on the same event: reveal the character control that
        // belongs to the newly selected SOUND. The map key also lets host /
        // preset sync refresh it without firing VALUE_CHANGED.
        lv_obj_add_event_cb(stickTypeDd, stickHeaderVisCb, LV_EVENT_VALUE_CHANGED, nullptr);
        lv_obj_add_event_cb(stickTypeDd, stickHeaderDeleteCb, LV_EVENT_DELETE, nullptr);
        syncStickHeader(stickTypeDd);
        uiFixDropdownPopup(stickTypeDd);
        widgets[PluginCymbals::kParamStickType] = stickTypeDd;

        // Dice #1: whole impulse screen (not the EQ). Dice #2: stick EQ.
        // RESET: stick EQ defaults, matching the final-bus EQ controls.
        auto stripRandomBtn = [&](lv_event_cb_t cb, const char* tip, bool resetStyle) {
            lv_obj_t* btn = lv_btn_create(strip);
            lv_obj_set_size(btn, scaled(resetStyle ? 64 : 28), scaled(resetStyle ? 28 : 28));
            styles.applyMomentaryButton(btn);
            lv_obj_clear_state(btn, LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x36393E), 0);
            lv_obj_set_user_data(btn, (void*)(intptr_t)-1);
            lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, ui);
            uiAttachTooltip(btn, tip);
            if (resetStyle) {
                lv_obj_t* l = lv_label_create(btn);
                lv_label_set_text(l, "RESET");
                lv_obj_set_style_text_font(l, getScaledMicroFont(), 0);
                lv_obj_center(l);
            } else {
                lv_obj_t* die = uiDiceIcon(btn, COL_TEXT);
                if (die) lv_obj_center(die);
            }
        };
        stripRandomBtn(randomScreenCb, "Randomize the impulse screen and EQ", false);

        lv_obj_t* contentRow = lv_obj_create(tab);
        lv_obj_set_size(contentRow, lv_pct(100), 0);  // height from flex_grow — pct children resolve against a definite height
        lv_obj_set_flex_grow(contentRow, 1);
        lv_obj_set_style_bg_opa(contentRow, 0, 0);
        lv_obj_set_style_border_width(contentRow, 0, 0);
        lv_obj_set_layout(contentRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(contentRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(contentRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_column(contentRow, scaled(8), 0);
        // Kill the default-theme vertical card padding (pad_ver 20/20 on plain
        // lv_obj): it stole 40px from the rail and pushed PITCH MSEG past the
        // left pane. Siblings levelRow/ampGrid/oscEnvRow already do this.
        lv_obj_set_style_pad_ver(contentRow, 0, 0);
        lv_obj_clear_flag(contentRow, LV_OBJ_FLAG_SCROLLABLE);

        // =========================================================
        // LEFT: Impulse Generator
        // =========================================================
        lv_obj_t* leftPane = lv_obj_create(contentRow);
        // 460 (was 300): the blind critic's page gap — "an oversized, largely
        // vacant spectrum display ... the functional controls are squeezed into
        // a cramped left rail". The rail carries every strike parameter in the
        // plugin (stick output, both envelopes, pitch MSEG, excitation); it was
        // running at 28px columns, which starved it of the width those modules
        // need. The right side no longer hosts the big EQ canvas: it is now a
        // column of the instruments the page's own content supports (impact
        // forecast, live spectrum, impulse output), each sized to what it
        // carries. Rail 460 + pad 8 + right column 796 = 1280 - 16 tab pad.
        lv_obj_set_size(leftPane, scaled(430), lv_pct(100));
        lv_obj_add_style(leftPane, &styles.panel, 0);
        lv_obj_set_flex_align(leftPane, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        // 1280x720 design canvas (uniform transform scale), so fixed child
        // geometry is deterministic — nested flex mis-computed well heights
        // and clipped value rows (critic round 6). No scrolling, ever.
        lv_obj_set_layout(leftPane, LV_LAYOUT_NONE);
        lv_obj_set_style_pad_all(leftPane, 0, 0);
        // Tinted module well with an inline tracked ALL-CAPS header.
        // knob well: top pad 4 + header 14 + pad_row 8 + row 104 + bot pad 2
        //            + border 2 = 134.
        // Wells are 420 wide: the pane is 430 with a 1px left inset, a 1px
        // right inset and an 8px breathing strip (scaled(1) + scaled(420) + 9).
        // The old 460 pane was 30px wider than the modules needed; that width
        // is handed to the EQ editor, which was visibly cramped at 1280.
        auto railGroup = [&](const char* header, lv_coord_t wellY, lv_coord_t wellH) -> lv_obj_t* {
            lv_obj_t* well = lv_obj_create(leftPane);
            lv_obj_set_pos(well, scaled(1), wellY);
            lv_obj_set_size(well, scaled(420), wellH);
            lv_obj_set_style_bg_color(well, COL_PANEL_DARK, 0);
            lv_obj_set_style_bg_opa(well, LV_OPA_COVER, 0);
            lv_obj_set_style_border_color(well, COL_ACCENT, 0);
            lv_obj_set_style_border_width(well, 1, 0);
            lv_obj_set_style_radius(well, scaled(4), 0);
            // Breathing room between section header and knob titles (user call).
            lv_obj_set_style_pad_top(well, scaled(4), 0);
            lv_obj_set_style_pad_bottom(well, scaled(2), 0);
            lv_obj_set_style_pad_hor(well, scaled(4), 0);
            lv_obj_set_style_pad_row(well, scaled(8), 0);
            lv_obj_set_layout(well, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(well, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_scrollbar_mode(well, LV_SCROLLBAR_MODE_OFF);
            lv_obj_clear_flag(well, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_t* h = lv_label_create(well);
            lv_label_set_text(h, header);
            lv_obj_add_style(h, &styles.title, 0);
            lv_obj_set_style_text_font(h, getScaledSmallFont(), 0);
            lv_obj_set_height(h, scaled(14));
            return well;
        };
        // Rail vertical map (design px @1.0): output 2..112, amp 114..224,
        // pitch 226..336, then the PITCH MSEG / EXCITATION module from 338 to
        // the pane bottom. That last module is the only one without a fixed
        // height: applyRailLayout() measures it on every pane resize, and its
        // active body is what fills the rail's leftover space.
        // SOUND SELECT lives in the strip (see above).
        // Knob rows are 80px: packed columns (15px title + 48px knobCont +
        // 15px value = 78 content, measured).
        const lv_coord_t kKnobWellH = scaled(110);  // pad4 + header14 + gap8 + row80 + pad2 + border2
        const lv_coord_t kYOutput = scaled(2);
        const lv_coord_t kYAmp = scaled(114);
        const lv_coord_t kYPitch = scaled(226);
        const lv_coord_t kYMseg = scaled(338);
        // Build-time height only (the pane has no size yet while the tree is
        // being built, so the first SIZE_CHANGED resolves the real one). 84
        // fits header 26 + gap 6 + an MSEG editor at its 44px floor.
        const lv_coord_t kRailWellH = scaled(84);
        UIWidgets::ArcVisualSpec smallSpec = UIWidgets::smallArcVisualSpec();
        // Rail pack: zero column pad_row AND zero spec margins (kit default
        // 1/1), plus uiPackKnob pins the arc at the 48px box top edge
        // (title bearing below matches digit bearing above: both visible
        // gaps read equal — see packRailCol below). Content: 15 + 48 + 15
        // = 78 in an 80px column. Rows are 80 with cross CENTER: level knobs.
        smallSpec.arcSize = scaled(44);
        smallSpec.containerW = scaled(68);
        smallSpec.containerH = scaled(80);
        smallSpec.labelMarginBottom = 0;
        smallSpec.valueMarginTop = 0;
        // Rail contract (unchanged): knob titles stay ONE line (short names,
        // wide columns) so values clear the row; rows are 80px for arcSize 44.




        // Rail-local pack: zero column pad_row + uiPackKnob top-pins the
        // arc at the knobCont top edge: title bearing below the text matches
        // value bearing above the digits, so both visible gaps read equal.
        // Column = knobCont's parent.
        // Pad now (harmless style store); the pack itself is deferred to the
        // footer tick (packRailWidgets) — packing mid-build silently misfires.
        auto packRailCol = [&](int param) {
            if (widgets[param] == nullptr) return;
            lv_obj_t* kc = lv_obj_get_parent(widgets[param]);
            lv_obj_t* col = (kc != nullptr) ? lv_obj_get_parent(kc) : nullptr;
            if (col == nullptr) return;
            lv_obj_set_style_pad_row(col, scaled(0), 0);
        };
        auto smallArc = [&](lv_obj_t* parent, int param, const char* name) {
            UIWidgets::createArcEx(parent, param, name, ui, styles, widgets, labels, fParams, valueCb, smallSpec);
            packRailCol(param);
        };
        // Compact curve knobs: D/R shape powers beside the main ADSR row.
        // They are deliberately narrower than the stage knobs; the LIN/LOG/EXP
        // readout under the cap and the adjacent shape preview carry the state.
        UIWidgets::ArcVisualSpec ampCurveSpec = smallSpec;
        ampCurveSpec.containerW = scaled(30);
        ampCurveSpec.containerH = scaled(80);
        ampCurveSpec.arcSize = scaled(24);
        ampCurveSpec.labelMarginBottom = 0;
        ampCurveSpec.valueMarginTop = 0;
        // The EXCITATION curve knobs declare their own spec inside that body
        // (36px columns, 58px tall) — they no longer share the AMP one.
        auto curveArc = [&](lv_obj_t* parent, int param, const char* name,
                            const UIWidgets::ArcVisualSpec& spec, bool packKnob) {
            UIWidgets::createArcEx(parent, param, name, ui, styles, widgets, labels, fParams, valueCb, spec);
            lv_obj_t* arc = widgets[param];
            if (arc != nullptr && packKnob)
                uiPackKnob(arc, spec.arcSize, scaled(0), spec.arcSize + scaled(4));
            packRailCol(param);
        };
        // Lane hue colour-coding (ENV orange / time olive arcs)
        // is deliberately gone — EVERY knob value arc is the same light blue,
        // COL_HIGHLIGHT (styles.arcIndicator default), per user call. The hue
        // lambdas collapsed into plain smallArc/wideArc; section identity now
        // reads from the well titles only.

        // OUTPUT module: Direct / Bus / Master.
        lv_obj_t* outputWell = railGroup("OUTPUT", kYOutput, kKnobWellH);
        lv_obj_t* levelRow = lv_obj_create(outputWell);
        lv_obj_set_size(levelRow, lv_pct(100), scaled(80));
        lv_obj_set_style_bg_opa(levelRow, 0, 0);
        lv_obj_set_style_border_width(levelRow, 0, 0);
        lv_obj_set_layout(levelRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(levelRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(levelRow, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(levelRow, scaled(12), 0);
        // kill the LVGL default-theme card padding (pad 20/20 on plain lv_obj)
        lv_obj_set_style_pad_ver(levelRow, 0, 0);
        smallArc(levelRow, PluginCymbals::kParamStickDirect, "Direct");
        smallArc(levelRow, PluginCymbals::kParamStickBus,    "Bus");
        smallArc(levelRow, PluginCymbals::kParamVolume,      "Master");
        smallArc(levelRow, PluginCymbals::kParamMasterTuning, "Tune");

        // AMP ENVELOPE module (tinted well, legend header).
        lv_obj_t* ampWell = railGroup("AMP ENVELOPE", kYAmp, kKnobWellH);
        lv_obj_t* ampGrid = lv_obj_create(ampWell);
        lv_obj_set_size(ampGrid, lv_pct(100), scaled(80));
        lv_obj_set_style_pad_ver(ampGrid, 0, 0);  // same theme-pad kill as the other two rows: identical row internals, level knobs
        lv_obj_set_style_bg_opa(ampGrid, 0, 0);
        lv_obj_set_style_border_width(ampGrid, 0, 0);
        lv_obj_set_layout(ampGrid, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(ampGrid, LV_FLEX_FLOW_ROW);  // 4x56 columns always fit the 292px pane — one line, no wrap
        lv_obj_set_flex_align(ampGrid, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_scrollbar_mode(ampGrid, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(ampGrid, LV_OBJ_FLAG_SCROLLABLE);
        smallArc(ampGrid, PluginCymbals::kParamStickAmpAttack,  "Attack");
        smallArc(ampGrid, PluginCymbals::kParamStickAmpDecay,   "Decay");
        smallArc(ampGrid, PluginCymbals::kParamStickAmpSustain, "Sustain");
        smallArc(ampGrid, PluginCymbals::kParamStickAmpRelease, "Release");
        curveArc(ampGrid, PluginCymbals::kParamStickAmpDecayCurve,   "DCR", ampCurveSpec, false);
        curveArc(ampGrid, PluginCymbals::kParamStickAmpReleaseCurve, "RCR", ampCurveSpec, false);
        lv_obj_t* ampCurvePreview = AdsrCurvePreview::create(
            ampGrid, ui,
            PluginCymbals::kParamStickAmpAttack, PluginCymbals::kParamStickAmpDecay,
            PluginCymbals::kParamStickAmpSustain, PluginCymbals::kParamStickAmpRelease,
            PluginCymbals::kParamStickAmpDecayCurve, PluginCymbals::kParamStickAmpReleaseCurve,
            scaled(64), scaled(24));
        for (int p : { PluginCymbals::kParamStickAmpAttack,
                       PluginCymbals::kParamStickAmpDecay,
                       PluginCymbals::kParamStickAmpSustain,
                       PluginCymbals::kParamStickAmpRelease,
                       PluginCymbals::kParamStickAmpDecayCurve,
                       PluginCymbals::kParamStickAmpReleaseCurve })
            AdsrCurvePreview::attach(widgets[p], ampCurvePreview);

        // PITCH ENVELOPE module: oscillator pitch-env controls (one-line labels).
        lv_obj_t* pitchWell = railGroup("PITCH ENVELOPE", kYPitch, kKnobWellH);
        lv_obj_t* oscEnvRow = lv_obj_create(pitchWell);
        lv_obj_set_size(oscEnvRow, lv_pct(100), scaled(80));
        lv_obj_set_style_bg_opa(oscEnvRow, 0, 0);
        lv_obj_set_style_border_width(oscEnvRow, 0, 0);
        lv_obj_set_layout(oscEnvRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(oscEnvRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(oscEnvRow, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(oscEnvRow, scaled(12), 0);
        // kill the LVGL default-theme card padding (pad 20/20 on plain lv_obj)
        lv_obj_set_style_pad_ver(oscEnvRow, 0, 0);
        lv_obj_set_scrollbar_mode(oscEnvRow, LV_SCROLLBAR_MODE_OFF);
        UIWidgets::ArcVisualSpec wideSpec = UIWidgets::smallWideTitleArcVisualSpec();
        wideSpec.containerH = smallSpec.containerH;  // one-line titles on the rail
        wideSpec.arcSize = smallSpec.arcSize;
        wideSpec.containerW = scaled(76);
        wideSpec.labelMarginBottom = 0;
        wideSpec.valueMarginTop = 0;
        auto wideArc = [&](lv_obj_t* parent, int param, const char* name) {
            UIWidgets::ArcVisualSpec s = wideSpec;
            UIWidgets::createArcEx(parent, param, name, ui, styles, widgets, labels, fParams, valueCb, s);
            packRailCol(param);
        };
        wideArc(oscEnvRow, PluginCymbals::kParamStickPitchAmount, "OSC Env");
        wideArc(oscEnvRow, PluginCymbals::kParamStickPitchTimer, "Env Time");
        wideArc(oscEnvRow, PluginCymbals::kParamStickPitchMsegDuration, "Length");

        // =========================================================
        // PITCH MSEG / EXCITATION — ONE module, two bodies.
        //
        // These used to be a 26px toggle bar stacked on an 84px MSEG card
        // plus a separate EXCITATION card: two titles, two headers, an empty
        // band above the toggle and only 84px for whichever view was active.
        // Now one well runs from just under PITCH ENVELOPE to the pane
        // bottom, carrying ONE header row (the mode toggle, which names the
        // active view, plus the strike readout in excitation mode) and one
        // body region that the active view fills completely.
        // =========================================================
        lv_obj_t* railWell = lv_obj_create(leftPane);
        lv_obj_set_pos(railWell, scaled(1), kYMseg);
        lv_obj_set_size(railWell, scaled(420), kRailWellH);
        lv_obj_add_style(railWell, &styles.card, 0);
        lv_obj_set_layout(railWell, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(railWell, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(railWell, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        // Same tightened module padding the sibling rail wells use (the card
        // style's 8px all-round inset would eat the body).
        lv_obj_set_style_pad_top(railWell, scaled(4), 0);
        lv_obj_set_style_pad_bottom(railWell, scaled(2), 0);
        lv_obj_set_style_pad_hor(railWell, scaled(4), 0);
        lv_obj_set_style_pad_gap(railWell, scaled(6), 0);
        lv_obj_set_scrollbar_mode(railWell, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(railWell, LV_OBJ_FLAG_SCROLLABLE);

        // The one header row: toggle left, strike readout right.
        lv_obj_t* railHeader = lv_obj_create(railWell);
        lv_obj_set_size(railHeader, lv_pct(100), scaled(26));
        lv_obj_set_style_border_width(railHeader, 0, 0);
        lv_obj_set_style_bg_opa(railHeader, 0, 0);
        lv_obj_set_layout(railHeader, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(railHeader, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(railHeader, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(railHeader, 0, 0);
        lv_obj_clear_flag(railHeader, LV_OBJ_FLAG_SCROLLABLE);

        // The button IS the module heading: it names the active view
        // ("PitchMseg" / "Excitation") instead of a separate title plus an
        // anonymous ON/OFF control. Width follows the caption.
        lv_obj_t* msegOnBtn = lv_btn_create(railHeader);
        lv_obj_set_size(msegOnBtn, LV_SIZE_CONTENT, scaled(24));
        lv_obj_add_flag(msegOnBtn, LV_OBJ_FLAG_CHECKABLE);
        
        bool isOn = fParams[PluginCymbals::kParamStickPitchMsegOn] > 0.5f;
        lv_obj_t* msegOnBtnLbl = lv_label_create(msegOnBtn);
        lv_obj_set_style_text_font(msegOnBtnLbl, getScaledSmallFont(), 0);
        lv_label_set_text(msegOnBtnLbl, isOn ? "PitchMseg" : "Excitation");
        lv_obj_center(msegOnBtnLbl);

        styles.applyToggleButton(msegOnBtn, isOn);
        // This is a two-view mode selector, not a power switch: both captions
        // name an ACTIVE view, so the unchecked Excitation state must keep the
        // lit accent treatment instead of falling back to the dark idle plate.
        lv_obj_add_style(msegOnBtn, &styles.btnChecked, 0);
        // Horizontal padding only: the caption carries the width.
        lv_obj_set_style_pad_hor(msegOnBtn, scaled(12), 0);
        lv_obj_set_style_pad_ver(msegOnBtn, 0, 0);
        
        lv_obj_set_user_data(msegOnBtn, reinterpret_cast<void*>((intptr_t)PluginCymbals::kParamStickPitchMsegOn));
        lv_obj_add_event_cb(msegOnBtn, valueCb, LV_EVENT_VALUE_CHANGED, ui);
        widgets[PluginCymbals::kParamStickPitchMsegOn] = msegOnBtn;

        // Strike-character readout: an excitation-only detail, so it hides
        // with that body and leaves the toggle alone in the header. Starts
        // hidden in PitchMseg mode (the first layout pass may arrive after
        // the pane has a size).
        lv_obj_t* exciteDesc = lv_label_create(railHeader);
        lv_label_set_text(exciteDesc, "CRISP");
        lv_obj_add_style(exciteDesc, &styles.compactValueLabel, 0);
        lv_obj_set_style_text_color(exciteDesc, COL_HIGHLIGHT, 0);
        lv_obj_set_style_text_align(exciteDesc, LV_TEXT_ALIGN_RIGHT, 0);
        if (isOn) lv_obj_add_flag(exciteDesc, LV_OBJ_FLAG_HIDDEN);

        // ON body: the MSEG editor. Fills the one body region; hidden (not
        // resized) in Excitation mode, and hidden children drop out of the
        // well's flex flow so the excitation body gets the space.
        lv_obj_t* msegEditor = UIMsegEditor::create(railWell, ui, styles);
        lv_obj_set_width(msegEditor, lv_pct(100));
        lv_obj_set_height(msegEditor, 0); // base 0 + grow(1) = fill; min floors tiny windows
        lv_obj_set_flex_grow(msegEditor, 1);
        lv_obj_set_style_min_height(msegEditor, scaled(44), 0);
        if (!isOn) lv_obj_add_flag(msegEditor, LV_OBJ_FLAG_HIDDEN);
        widgets[PluginCymbals::kParamStickPitchMsegPoint0Value] = msegEditor;

        // =========================================================
        // OFF body: EXCITATION, the strike itself.
        //
        // Paper anchor (CymbalGPU): what makes a cymbal sound is decided
        // at the excitation — the hammer's hardness, how long the mallet
        // stays in contact and where it lands. This body surfaces the
        // pitch-envelope knobs that carry exactly those characters (Stick
        // Pitch A/D/R were wired to the DSP with NO UI surface at all)
        // beside a live forecast of the impact burst they produce: the
        // selected SOUND shape over a decaying envelope, blended at the
        // current pitch. Display-only drawing — every control is an
        // existing parameter, so no DSP behaviour changes.
        // =========================================================
        lv_obj_t* exciteBody = lv_obj_create(railWell);
        lv_obj_set_width(exciteBody, lv_pct(100));
        lv_obj_set_height(exciteBody, 0);   // grow(1): fills the module body
        lv_obj_set_flex_grow(exciteBody, 1);
        lv_obj_set_style_bg_opa(exciteBody, 0, 0);
        lv_obj_set_style_border_width(exciteBody, 0, 0);
        lv_obj_set_style_pad_all(exciteBody, 0, 0);
        lv_obj_set_style_pad_row(exciteBody, scaled(6), 0);
        lv_obj_set_layout(exciteBody, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(exciteBody, LV_FLEX_FLOW_COLUMN);
        // CENTER: with the scope showing, its grow eats every spare px, so
        // this is identical to START; with the scope dropped on a very short
        // body the knob band sits centred instead of hugging the top.
        lv_obj_set_flex_align(exciteBody, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_scrollbar_mode(exciteBody, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(exciteBody, LV_OBJ_FLAG_SCROLLABLE);
        if (isOn) lv_obj_add_flag(exciteBody, LV_OBJ_FLAG_HIDDEN);

        // =========================================================
        // Excitation body grid — height responsive, two full-width bands.
        //
        // The old body was a single ROW: five 28px caps in a 324px strip with
        // the forecast squashed into a narrow tall box beside them. Whichever
        // way the pane measures, that left most of the body empty.
        //
        //   +-----------------------------------------------------+
        //   | Hardness  Contact  Ring  DCR RCR  [ ADSR preview ]  | 76px band
        //   +-----------------------------------------------------+
        //   |          IMPACT FORECAST — full width, grow(1)      | rest
        //   +-----------------------------------------------------+
        //
        // BAND 1 is a fixed 76px row: 15 title + 44 knob box + 15 value plus
        // the kit's 2px bearing slack, so every cap, title and value fits
        // without clipping at ANY body height (the body is never under ~90).
        // The three pitch-ADSR caps go from 28 to 40px and share the row with
        // the two curve toggles and the ADSR preview, spanning the whole
        // 410px width — no more narrow left cluster.
        // BAND 2 is the forecast scope at full width with grow(1): every px
        // the pane gives the module beyond the knob band becomes scope, so a
        // tall body is a tall scope, not dead space. applyRailLayout() hides
        // band 2 only when the body is too short to hold a usable box.
        // =========================================================
        lv_obj_t* exKnobRow = lv_obj_create(exciteBody);
        lv_obj_set_user_data(exKnobRow, (void*)kExciteGateTag);
        lv_obj_set_size(exKnobRow, lv_pct(100), scaled(76));
        lv_obj_set_style_bg_opa(exKnobRow, 0, 0);
        lv_obj_set_style_border_width(exKnobRow, 0, 0);
        lv_obj_set_style_pad_all(exKnobRow, 0, 0);   // kill the LVGL default card padding
        lv_obj_set_style_pad_column(exKnobRow, scaled(6), 0);
        lv_obj_set_layout(exKnobRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(exKnobRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(exKnobRow, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_scrollbar_mode(exKnobRow, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(exKnobRow, LV_OBJ_FLAG_SCROLLABLE);

        // Pitch-ADSR arcs: 40px caps in 78px columns (was 28 in 64). The rail
        // contract holds — containerH = arcSize + 34 with the packed knob box
        // at arcSize + 4: 15 + 44 + 15 = 74 in a 76px column.
        UIWidgets::ArcVisualSpec exSpec;
        exSpec.containerW = scaled(78);
        exSpec.containerH = scaled(76);
        exSpec.arcSize = scaled(40);
        exSpec.labelMarginBottom = 0;
        exSpec.valueMarginTop = 0;
        auto exArc = [&](lv_obj_t* parent, int param, const char* name) {
            UIWidgets::createArcEx(parent, param, name, ui, styles, widgets, labels, fParams, valueCb, exSpec);
            lv_obj_t* arc = widgets[param];
            if (arc == nullptr) return;
            uiPackKnob(arc, exSpec.arcSize, scaled(0), exSpec.arcSize + scaled(4));
            lv_obj_t* kc = lv_obj_get_parent(arc);
            lv_obj_t* col = (kc != nullptr) ? lv_obj_get_parent(kc) : nullptr;
            if (col != nullptr) lv_obj_set_style_pad_row(col, scaled(0), 0);
        };
        exArc(exKnobRow, PluginCymbals::kParamStickPitchAttack,  "Hardness");
        exArc(exKnobRow, PluginCymbals::kParamStickPitchDecay,   "Contact");
        exArc(exKnobRow, PluginCymbals::kParamStickPitchRelease, "Ring");
        // Curve shape toggles ride the same band (22px caps in 36px columns):
        // they are the DCR/RCR curve powers that shape what the arcs above do.
        UIWidgets::ArcVisualSpec exCurveSpec;
        exCurveSpec.containerW = scaled(36);
        exCurveSpec.containerH = scaled(58);
        exCurveSpec.arcSize = scaled(22);
        exCurveSpec.labelMarginBottom = 0;
        exCurveSpec.valueMarginTop = 0;
        curveArc(exKnobRow, PluginCymbals::kParamStickPitchDecayCurve,   "DCR", exCurveSpec, true);
        curveArc(exKnobRow, PluginCymbals::kParamStickPitchReleaseCurve, "RCR", exCurveSpec, true);
        // ADSR preview closes the band: grows into every px the knobs leave
        // (a 56x22 chip before), so the row reads as a strip of controls
        // topped by a graph rather than a cluster with a void to its right.
        lv_obj_t* pitchCurvePreview = AdsrCurvePreview::create(
            exKnobRow, ui,
            PluginCymbals::kParamStickPitchAttack, PluginCymbals::kParamStickPitchDecay,
            PluginCymbals::kParamStickPitchSustain, PluginCymbals::kParamStickPitchRelease,
            PluginCymbals::kParamStickPitchDecayCurve, PluginCymbals::kParamStickPitchReleaseCurve,
            scaled(24), scaled(58));
        if (pitchCurvePreview != nullptr && lv_obj_is_valid(pitchCurvePreview)) {
            // Redraws from its own coords each frame, so growing it is safe.
            lv_obj_set_width(pitchCurvePreview, 0);
            lv_obj_set_flex_grow(pitchCurvePreview, 1);
            lv_obj_set_height(pitchCurvePreview, scaled(58));
        }
        for (int p : { PluginCymbals::kParamStickPitchAttack,
                       PluginCymbals::kParamStickPitchDecay,
                       PluginCymbals::kParamStickPitchSustain,
                       PluginCymbals::kParamStickPitchRelease,
                       PluginCymbals::kParamStickPitchDecayCurve,
                       PluginCymbals::kParamStickPitchReleaseCurve })
            AdsrCurvePreview::attach(widgets[p], pitchCurvePreview);
        uiAttachTooltip(exKnobRow,
            "Stick pitch ADSR. Overridden while the header toggle reads "
            "PitchMseg (the MSEG then owns the pitch).");
        refreshExcitationGate(tab, fParams);

        // Band 2 — impact forecast: passive display, never a parameter. Full
        // body width and every leftover px of body height, so the burst reads
        // across the module instead of in a narrow side box.
        lv_obj_t* exScope = lv_obj_create(exciteBody);
        lv_obj_set_size(exScope, lv_pct(100), 0);
        lv_obj_set_flex_grow(exScope, 1);
        lv_obj_set_style_bg_color(exScope, COL_PANEL_DARK, 0);
        lv_obj_set_style_bg_opa(exScope, 200, 0);
        lv_obj_set_style_border_color(exScope, COL_ACCENT, 0);
        lv_obj_set_style_border_width(exScope, 1, 0);
        lv_obj_set_style_radius(exScope, scaled(4), 0);
        lv_obj_set_style_pad_all(exScope, 0, 0);
        lv_obj_set_layout(exScope, LV_LAYOUT_NONE);   // the strike pip is absolute
        lv_obj_clear_flag(exScope, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(exScope, LV_OBJ_FLAG_CLICKABLE);

        // Live state: one 100ms timer drives the readout, the strike pip
        // and the scope; the scope repaints only when a driver moves.
        ExciteState* exState = new ExciteState();
        exState->ui = ui;
        exState->desc = exciteDesc;
        exState->scope = exScope;
        // Strike pip — a REAL child object: draw-callback furniture is
        // unreliable in this kit (see the spectrum analyzer's object bars),
        // while a child always renders. Positioned by the timer from the
        // same model the trace is drawn with, so it rides the burst apex.
        lv_obj_t* exPip = lv_obj_create(exScope);
        lv_obj_set_size(exPip, scaled(9), scaled(9));
        lv_obj_set_style_radius(exPip, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(exPip, COL_HIGHLIGHT, 0);
        lv_obj_set_style_bg_opa(exPip, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(exPip, 1, 0);
        lv_obj_set_style_border_color(exPip, COL_WHITE, 0);
        lv_obj_set_style_pad_all(exPip, 0, 0);
        lv_obj_set_scrollbar_mode(exPip, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(exPip, LV_OBJ_FLAG_SCROLLABLE);
        exState->pip = exPip;
        lv_obj_set_user_data(exScope, exState);
        lv_obj_add_event_cb(exScope, exciteEventCb, LV_EVENT_ALL, exState);
        exState->timer = lv_timer_create(exciteTimerCb, 100, exState);
        exciteTick(exState);  // prime the first frame before the first paint

        // The header toggle is the module's mode switch. ON: the MSEG editor
        // is the body and EXCITATION is hidden. OFF: the MSEG body collapses
        // (the header row stays, so the switch is always reachable) and the
        // strike body takes the space. The well is re-measured on every pane
        // resize; absolute wells are out of flow, so setting them cannot loop.
        RailLayout* railLayout = new RailLayout();
        railLayout->stage = tab;
        railLayout->pane = leftPane;
        railLayout->well = railWell;
        railLayout->msegBody = msegEditor;
        railLayout->exciteBody = exciteBody;
        railLayout->desc = exciteDesc;
        railLayout->scope = exScope;
        railLayout->wellY = kYMseg;
        railLayout->on = isOn;
        railLayoutMap()[tab] = railLayout;
        lv_obj_add_event_cb(msegOnBtn, [](lv_event_t* e) {
            RailLayout* rl = static_cast<RailLayout*>(lv_event_get_user_data(e));
            lv_obj_t* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
            if (rl == nullptr || btn == nullptr) return;
            rl->on = lv_obj_has_state(btn, LV_STATE_CHECKED);
            applyRailLayout(rl);
        }, LV_EVENT_VALUE_CHANGED, railLayout);
        lv_obj_add_event_cb(leftPane, [](lv_event_t* e) {
            RailLayout* rl = static_cast<RailLayout*>(lv_event_get_user_data(e));
            if (rl != nullptr) applyRailLayout(rl);
        }, LV_EVENT_SIZE_CHANGED, railLayout);
        lv_obj_add_event_cb(leftPane, [](lv_event_t* e) {
            RailLayout* rl = static_cast<RailLayout*>(lv_event_get_user_data(e));
            if (rl == nullptr) return;
            railLayoutMap().erase(rl->stage);
            delete rl;
        }, LV_EVENT_DELETE, railLayout);
        applyRailLayout(railLayout);

        // =========================================================
        // =========================================================
        // RIGHT: Visual EQ Editor
        // =========================================================
        // The EQ editor is the home page's right pane. It was briefly replaced
        // here by an impulse-shape preview and a duplicate of the global
        // spectrum, on a critic's "oversized vacant graph" reading - but the
        // EQ IS this page's content: it is the one control set the home page
        // exists for, and taking it away was a regression the user called out.
        const UIEQSection::HeaderControlSpec eqArcs[] = {
            {PluginCymbals::kParamEQMix, "Mix", UIEQSection::ControlType::Arc, nullptr},
            {PluginCymbals::kParamTiltAmount, "Tilt", UIEQSection::ControlType::Arc, nullptr},
            {PluginCymbals::kParamTiltPivot, "Pivot", UIEQSection::ControlType::Arc, nullptr},
        };
        UIEQSection::BuildOptions eqOpts;
        eqOpts.eqBaseParam = PluginCymbals::kParamEQ1Freq;
        eqOpts.fillHeight = true;
        eqOpts.grow = true;
        eqOpts.cardStyle = false;
        eqOpts.headerHeight = scaled(72);
        // START (not SPACE_EVENLY): the header shares its strip with the
        // band-editor card, so any width change respread the knobs - their x
        // jumped on band select/deselect, model/type swaps, and resizes.
        // Left-packed columns keep identical x in every state (batch 5).
        eqOpts.headerMainAlign = LV_FLEX_ALIGN_START;  // fixed left-packed knob x
        eqOpts.compactHeaderArcs = true;
        lv_obj_t* eqTopBar = nullptr;
        eqOpts.outTopBar = &eqTopBar;
        UIEQSection::create(contentRow, ui, styles, widgets, labels, fParams, valueCb, rollerCb, eqOpts, eqArcs, 3);

        // Stick-EQ dice + RESET live at the right end of the EQ panel's own
        // top strip (next to Mix/Tilt/Pivot), mirroring the final-bus EQ.
        if (eqTopBar != nullptr) {
            lv_obj_t* tools = lv_obj_create(eqTopBar);
            lv_obj_set_size(tools, LV_SIZE_CONTENT, lv_pct(100));
            lv_obj_set_style_bg_opa(tools, 0, 0);
            lv_obj_set_style_border_width(tools, 0, 0);
            lv_obj_set_style_pad_all(tools, 0, 0);
            lv_obj_set_style_pad_column(tools, scaled(4), 0);
            lv_obj_set_layout(tools, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(tools, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(tools, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(tools, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t* eqDice = lv_btn_create(tools);
            lv_obj_set_size(eqDice, scaled(28), scaled(28));
            styles.applyMomentaryButton(eqDice);
            lv_obj_set_style_bg_color(eqDice, lv_color_hex(0x36393E), 0);
            lv_obj_set_user_data(eqDice, (void*)(intptr_t)-1);
            lv_obj_add_event_cb(eqDice, randomEqCb, LV_EVENT_CLICKED, ui);
            uiAttachTooltip(eqDice, "Randomize the stick EQ");
            lv_obj_t* die = uiDiceIcon(eqDice, COL_TEXT);
            if (die) lv_obj_center(die);

        }

        // RESET sits immediately after Mix/Tilt/Pivot, exactly like the
        // final-bus EQ header; the dice stays at the far right.
        if (eqTopBar != nullptr) {
            lv_obj_t* eqSettingsHost = lv_obj_get_child(eqTopBar, 0);
            if (eqSettingsHost != nullptr) {
                lv_obj_t* reset = lv_btn_create(eqSettingsHost);
                lv_obj_set_size(reset, scaled(64), scaled(28));
                styles.applyMomentaryButton(reset);
                lv_obj_set_style_bg_color(reset, lv_color_hex(0x36393E), 0);
                lv_obj_set_user_data(reset, (void*)(intptr_t)-1);
                lv_obj_add_event_cb(reset, resetEqCb, LV_EVENT_CLICKED, ui);
                uiAttachTooltip(reset, "Reset the stick EQ");
                lv_obj_t* rl = lv_label_create(reset);
                lv_label_set_text(rl, "RESET");
                lv_obj_set_style_text_font(rl, getScaledMicroFont(), 0);
                lv_obj_center(rl);
            }
        }

    }

private:
    // ---- EXCITATION forecast ------------------------------------------
    // Live visualisation state for the EXCITATION module: one 100ms timer
    // drives the strike-character readout, the strike pip and the scope,
    // and the scope repaints only when a driver value actually moves.
    // Ownership: heap-allocated, held in the scope's user_data and deleted
    // with the scope (LV_EVENT_DELETE) — same pattern as UIMsegEditor.
    struct ExciteState {
        AbstractCymbalsUI* ui = nullptr;
        lv_obj_t* scope = nullptr;
        lv_obj_t* pip = nullptr;
        lv_obj_t* desc = nullptr;
        lv_timer_t* timer = nullptr;
        // Mirror of the last-read driver values (init: never read).
        float hardness = -1.0f, contact = -1.0f, ring = -1.0f, wave = -1.0f, pitch = -1.0f;
        // Forecast model, recomputed with the drivers and shared by the pip
        // placement and the trace so the pip always rides the burst apex.
        float tau = 0.05f, rise = 0.002f, winSec = 0.15f, cycles = 4.0f, amp = 0.6f;
        int type = 0;
    };

    // Oscillator shape at phase t (in turns) for stick type 0..7, in
    // [-1,1]. The shapes mirror StickSoundBank's oscillator set (StickType
    // order): Sine, Triangle, Saw, Square, Pulse, Noise, Metallic. Metallic
    // renders the real MetallicPartialBank: 12 inharmonic ratios with
    // 1/(1+0.5i) amps. Display-only math — nothing here touches DSP state.
    static float stickWaveValue(int type, float t) {
        t -= std::floor(t);
        switch (type) {
            case 9: return 0.0f;                                             // NONE: external input only
            case 1: return 2.0f * (1.0f - 2.0f * std::fabs(t - 0.5f)) - 1.0f;  // TRIANGLE
            case 2: return 2.0f * t - 1.0f;                                  // SAW
            case 3: return (t < 0.5f) ? 1.0f : -1.0f;                         // SQUARE
            case 4: return (t < 0.25f) ? 1.0f : -1.0f;                        // PULSE
            case 5: {                                                      // NOISE (deterministic)
                const float k = t * 1.61803f + 7.31f;
                float h = std::fmod(std::sin(k * 12.9898f + 3.17f) * 43758.5453f, 1.0f);
                return (h < 0.0f ? h + 1.0f : h) * 1.8f - 0.9f;
            }
            case 8: {                                                      // HI-HAT: differentiated noise edge
                const float n = ((t * 1.61803f + 7.31f) * 12.9898f + 3.17f);
                const float h = std::fmod(std::sin(n) * 43758.5453f, 1.0f);
                const float noise = (h < 0.0f ? h + 1.0f : h) * 2.0f - 1.0f;
                return std::clamp(noise * 1.6f, -1.0f, 1.0f);
            }
            case 7: {                                                      // NOISE+METAL: hybrid display
                const float n = ((t * 1.61803f + 7.31f) * 12.9898f + 3.17f);
                const float h = std::fmod(std::sin(n) * 43758.5453f, 1.0f);
                const float noise = (h < 0.0f ? h + 1.0f : h) * 2.0f - 1.0f;
                float raw = 0.0f;
                for (int p = 0; p < 12; ++p) {
                    const float amp = 1.0f / (1.0f + 0.5f * (float)p);
                    raw += std::sin(t * 6.2831853f * (1.0f + 0.37f * (float)p) + 0.7f * (float)p) * amp;
                }
                return std::clamp(noise * 0.5f + raw * 0.5f, -1.0f, 1.0f);
            }
            case 6: {                                                      // METALLIC (12 partials)
                static const float kRatios[12] = {
                    1.0f, 1.414f, 1.732f, 2.15f, 2.67f, 3.12f,
                    3.89f, 4.35f, 5.12f, 6.23f, 7.89f, 9.12f
                };
                static const float kAmpSum = []() {
                    float s = 0.0f;
                    for (int p = 0; p < 12; ++p) s += 1.0f / (1.0f + 0.5f * (float)p);
                    return s;
                }();
                float raw = 0.0f;
                for (int p = 0; p < 12; ++p) {
                    const float amp = 1.0f / (1.0f + 0.5f * (float)p);
                    raw += std::sin(t * 6.2831853f * kRatios[p] + 0.7f * (float)p) * amp;
                }
                return std::clamp(raw / (0.5f * kAmpSum) * 2.2f, -1.0f, 1.0f);
            }
            default: return std::sin(t * 6.2831853f);                        // SINE
        }
    }

    // Burst sample at time t (seconds) into the strike: the oscillator
    // shape over a decaying contact envelope, with the ring knob adding a
    // long faint tail (the pitch-env release). Display-only.
    static float exciteSample(const ExciteState* st, float t) {
        float env = std::exp(-t / st->tau);
        env *= (t < st->rise) ? (t / st->rise) : 1.0f;
        env += st->ring * 0.30f * std::exp(-t / std::max(0.05f, st->tau * 6.0f));
        return stickWaveValue(st->type, (t / st->winSec) * st->cycles) * env;
    }

    // Re-reads the drivers, rebuilds the forecast model, updates the strike
    // readout + pip, and repaints the scope when something actually moved.
    static void exciteTick(ExciteState* st) {
        if (st == nullptr || st->ui == nullptr || st->scope == nullptr) return;
        if (!lv_obj_is_valid(st->scope)) return;
        const float hard  = std::clamp(st->ui->getParamValue(PluginCymbals::kParamStickPitchAttack), 0.0f, 1.0f);
        const float cont  = std::clamp(st->ui->getParamValue(PluginCymbals::kParamStickPitchDecay), 0.0f, 1.0f);
        const float ring  = std::clamp(st->ui->getParamValue(PluginCymbals::kParamStickPitchRelease), 0.0f, 1.0f);
        const float wave  = std::clamp(st->ui->getParamValue(PluginCymbals::kParamStickType), 0.0f, 9.0f);
        const float pitch = std::clamp(st->ui->getParamValue(PluginCymbals::kParamStickPitchMsegPoint0Value), 0.0f, 1.0f);
        const bool moved = hard != st->hardness || cont != st->contact || ring != st->ring ||
                           wave != st->wave || pitch != st->pitch;
        st->hardness = hard; st->contact = cont; st->ring = ring; st->wave = wave; st->pitch = pitch;
        // Forecast model: the time mappings mirror the DSP's own
        // control->seconds curves (StickSoundBank: contact 5ms..1.5s,
        // attack 0.3ms..60ms) so the picture stays honest to the sound.
        st->tau    = 0.005f * std::pow(300.0f, cont);            // contact decay 5ms..1.5s
        st->rise   = 0.0003f * std::pow(200.0f, 1.0f - hard);   // onset 0.06ms..60ms
        st->winSec = std::max(0.040f, st->tau * 3.0f);           // shown window
        st->cycles = std::clamp(60.0f * std::pow(2.0f, pitch * 8.0f) * st->winSec, 1.0f, 14.0f);
        st->amp    = 0.30f + 0.70f * hard;                       // transient level
        st->type   = (int)std::lround(wave);
        const bool noStick = ((int)StickSoundBank::SILENT == st->type);

        if (st->desc != nullptr && lv_obj_is_valid(st->desc)) {
            const char* word = noStick ? "INPUT"
                              : hard < 0.22f ? "SOFT" : hard < 0.45f ? "WARM"
                              : hard < 0.72f ? "CRISP" : "AGGRESSIVE";
            lv_label_set_text(st->desc, word);
        }
        if (st->pip != nullptr && lv_obj_is_valid(st->pip)) {
            // The strike pip rides the burst apex: x = onset (tighter with
            // a harder hammer), y = the apex amplitude.
            float bestT = 0.0f, bestV = 0.0f;
            const int kSteps = 48;
            for (int i = 0; i <= kSteps; ++i) {
                const float t = (float)i / (float)kSteps * st->winSec;
                const float v = exciteSample(st, t);
                if (std::fabs(v) > std::fabs(bestV)) { bestV = v; bestT = t; }
            }
            const int cw = lv_obj_get_width(st->scope);
            const int ch = lv_obj_get_height(st->scope);
            const int half = std::max(4, ch / 2 - 6);
            const int cx = kForecastPad + (int)((bestT / st->winSec) * std::max(1, cw - 2 * kForecastPad - 1)) - 4;
            const int cy = (ch / 2) - (int)(bestV * st->amp * half) - 4;
            lv_obj_set_pos(st->pip, std::clamp(cx, 1, std::max(1, cw - 10)),
                                    std::clamp(cy, 1, std::max(1, ch - 10)));
        }
        if (moved) lv_obj_invalidate(st->scope);
    }

    static void exciteTimerCb(lv_timer_t* t) {
        if (t == nullptr || t->user_data == nullptr) return;
        exciteTick(static_cast<ExciteState*>(t->user_data));
    }

    // Paints the impact forecast: scope grid, zero line, then the burst
    // (contact envelope x selected SOUND shape) as a filled silhouette with
    // a bright trace on top. Same DRAW_POST pattern the MSEG editor uses
    // (proven to render in this tab); nothing here is DSP state.
    static const int kForecastPad = 4;

    static void exciteDraw(ExciteState* st, lv_obj_t* obj, lv_layer_t* layer) {
        if (st == nullptr || st->ui == nullptr || layer == nullptr) return;
        lv_area_t coords;
        lv_obj_get_coords(obj, &coords);
        const int w = lv_obj_get_width(obj);
        const int h = lv_obj_get_height(obj);
        if (w < 12 || h < 12) return;

        const int plotW = w - 2 * kForecastPad;
        const int plotH = h - 2 * kForecastPad;
        const int x0 = coords.x1 + kForecastPad;
        const int midY = coords.y1 + kForecastPad + plotH / 2;
        const int halfH = std::max(4, plotH / 2 - 2);
        auto px = [&](float t) { return x0 + (int)(t * (plotW - 1)); };
        auto py = [&](float v) { return midY - (int)(v * st->amp * halfH); };

        // Grid: faint quarter divisions + a brighter zero line.
        lv_draw_line_dsc_t grid;
        lv_draw_line_dsc_init(&grid);
        grid.color = COL_EQ_GRID;
        grid.width = 1;
        grid.opa = 110;
        for (int g = 1; g <= 3; ++g) {
            if (g == 2) continue;  // zero line handled below
            const int gy = py((float)g / 4.0f - 0.5f);
            grid.p1.x = x0; grid.p1.y = gy;
            grid.p2.x = x0 + plotW - 1; grid.p2.y = gy;
            lv_draw_line(layer, &grid);
        }
        grid.color = COL_EQ_GRID_ZERO;
        grid.opa = 180;
        grid.p1.x = x0; grid.p1.y = midY;
        grid.p2.x = x0 + plotW - 1; grid.p2.y = midY;
        lv_draw_line(layer, &grid);

        const int n = std::max(24, plotW);  // ~1 sample per column
        lv_draw_line_dsc_t fill;
        lv_draw_line_dsc_init(&fill);
        fill.color = COL_HIGHLIGHT;
        fill.width = 1;
        fill.opa = 55;
        lv_draw_line_dsc_t trace = fill;
        trace.width = 2;
        trace.opa = 235;

        int prevX = px(0.0f), prevY = midY;
        for (int i = 0; i < n; ++i) {
            const float t = (float)i / (float)(n - 1);
            const float v = std::clamp(exciteSample(st, t * st->winSec) * st->amp, -1.0f, 1.0f);
            const int x = px(t);
            const int y = std::clamp(py(v), coords.y1 + kForecastPad, coords.y1 + h - kForecastPad - 1);
            if (i > 0) {
                fill.p1.x = prevX; fill.p1.y = midY;   // silhouette bar
                fill.p2.x = prevX; fill.p2.y = prevY;
                lv_draw_line(layer, &fill);
                trace.p1.x = prevX; trace.p1.y = prevY; // bright trace
                trace.p2.x = x; trace.p2.y = y;
                lv_draw_line(layer, &trace);
            }
            prevX = x; prevY = y;
        }
    }

    static void exciteEventCb(lv_event_t* e) {
        lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
        ExciteState* st = static_cast<ExciteState*>(lv_event_get_user_data(e));
        if (obj == nullptr || st == nullptr) return;
        const lv_event_code_t code = lv_event_get_code(e);
        if (code == LV_EVENT_DRAW_POST) {
            exciteDraw(st, obj, lv_event_get_layer(e));
        } else if (code == LV_EVENT_DELETE) {
            if (st->timer) lv_timer_del(st->timer);
            st->timer = nullptr; st->scope = nullptr; st->pip = nullptr; st->desc = nullptr;
            delete st;
        }
    }
};

END_NAMESPACE_DISTRHO

#endif // STAGE1_TAB_HPP
