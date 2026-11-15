#ifndef STAGE_FINAL_TAB_HPP
#define STAGE_FINAL_TAB_HPP

#include "../UIWidgets.hpp"
#include "../UIXYPad.hpp"
#include "../CombEditorDock.hpp"
#include "../CombEditorSelectionController.hpp"
#include "../UIEQSection.hpp"
#include "../CombGroupHeader.hpp"
#include "../../DSPUtils.hpp"
#include "../../UnifiedFilter.hpp"

START_NAMESPACE_DISTRHO

struct StageFinalTab {
    struct FinalEditorState {
        // Ownership: heap-allocated with 'new' during buildFinalEditor(),
        // deleted in deleteStateCb when the LVGL editor panel is destroyed
        AbstractCymbalsUI* ui = nullptr;
        int selectedComb = -1;
        int groupIdx = 0;
        bool isFinal = false;   // dock: which bank's bypass flag to report
        // Mod-target registration: widgets[] entries for the selected comb's
        // routable dock arcs. Rebound by CombEditorDock::refreshEditorFromSelection
        // whenever the selection changes; cleared on teardown.
        lv_obj_t** widgets = nullptr;
        uint32_t modParams[24] = {};
        int modParamCount = 0;
        // Fix B: slot owners evicted by the mod-target registration above,
        // aligned with modParams[] — handed back by
        // CombEditorDock::releaseModTargetBindings on rebind/dock-close
        // (otherwise closing the dock left e.g. the header's LPF/HPF
        // unregistered for the rest of the session). Fix #1: deleteStateCb
        // does NOT hand them back — the whole combPane subtree, including
        // every saved owner, is being deleted, so it nulls instead.
        lv_obj_t* savedWidgets[24] = {};
        lv_obj_t* xyPad = nullptr;          // Ownership: parent (LVGL)
        lv_obj_t* selectionButtons[4] = {nullptr, nullptr, nullptr, nullptr};
        lv_obj_t* panel = nullptr;          // Ownership: top layer when panelDetached
        bool panelDetached = false;
        lv_obj_t* titleLabel = nullptr;     // Ownership: parent (LVGL)
        lv_obj_t* arcFreq = nullptr;        // Ownership: parent (LVGL)
        lv_obj_t* arcFB = nullptr;          // the comb's feedback (live DSP param)
        lv_obj_t* arcQ = nullptr;           // final-bank per-filter Q
        lv_obj_t* arcLPF = nullptr;         // Ownership: parent (LVGL)
        lv_obj_t* arcHPF = nullptr;         // Ownership: parent (LVGL)
        lv_obj_t* arcScale = nullptr;       // Ownership: parent (LVGL)
        lv_obj_t* arcMix = nullptr;         // Ownership: parent (LVGL)
        lv_obj_t* arcSatAmt = nullptr;      // Ownership: parent (LVGL)
        lv_obj_t* arcStressOnset = nullptr; // Ownership: parent (LVGL)
        lv_obj_t* arcStressKnee = nullptr;  // Ownership: parent (LVGL)
        lv_obj_t* arcModeRate = nullptr;    // Ownership: parent (LVGL)
        lv_obj_t* arcModeA = nullptr;
        lv_obj_t* arcModeB = nullptr;
        lv_obj_t* arcModeAmount = nullptr;  // Ownership: parent (LVGL)
        lv_obj_t* arcPartRelPos = nullptr;  // Saike model controls (partials types only)
        lv_obj_t* arcPartFreqDamp = nullptr;
        lv_obj_t* arcPartStiffness = nullptr;
        lv_obj_t* arcPartStiffExp = nullptr;
        lv_obj_t* arcPartCount = nullptr;
        lv_obj_t* soloButton = nullptr;     // Ownership: parent (LVGL)
        lv_obj_t* enableButton = nullptr;   // Ownership: parent (LVGL)
        lv_obj_t* enableLabel = nullptr;    // Ownership: parent (LVGL)
        lv_obj_t* phaseToggleButton = nullptr; // Ownership: parent (LVGL)
        lv_obj_t* satTypeDropdown = nullptr;// Ownership: parent (LVGL)
        lv_obj_t* filterTypeDropdown = nullptr;// Ownership: parent (LVGL)
    };

    // Ownership: heap-allocated 'new' during buildFinalEqPanel, deleted in
    // the eqPane's LV_EVENT_DELETE. Holds the 5 band-readout chip labels and
    // the live timer that refreshes them.
    struct EqBandStripState {
        AbstractCymbalsUI* ui = nullptr;
        lv_obj_t* tiltReadout = nullptr;       // Ownership: parent (LVGL)
        lv_obj_t* chipLabels[5] = {nullptr};   // Ownership: parent (LVGL)
        lv_obj_t* chipMeters[5] = {nullptr};  // Ownership: parent (LVGL)
        lv_timer_t* timer = nullptr;           // Ownership: LVGL timer
    };

    static constexpr int kFinalEqBandCount = 5;

    static int finalEqBandFreqParam(int band) { return PluginCymbals::kParamFinalEQ1Freq + band * 4; }
    static int finalEqBandGainParam(int band) { return PluginCymbals::kParamFinalEQ1Freq + band * 4 + 1; }
    static int finalEqBandQParam(int band)    { return PluginCymbals::kParamFinalEQ1Freq + band * 4 + 2; }
    static int finalEqBandTypeParam(int band) { return PluginCymbals::kParamFinalEQ1Freq + band * 4 + 3; }

    static const char* finalEqBandName(int band) {
        static const char* names[kFinalEqBandCount] = {"HPF", "LOW", "MID", "HIGH", "LPF"};
        return (band >= 0 && band < kFinalEqBandCount) ? names[band] : "?";
    }

    static lv_color_t finalEqBandColor(int band) {
        static const lv_color_t colors[kFinalEqBandCount] = {
            COL_EQ_HPF_BG, COL_EQ_LOW, COL_EQ_MID, COL_EQ_HIGH, COL_EQ_LPF_BG
        };
        return (band >= 0 && band < kFinalEqBandCount) ? colors[band] : COL_TEXT;
    }

    // RESET: write every Final EQ parameter back to its canonical default.
    // Uses getParameterDefault so the values match what a host preset-load
        // would produce. editParameter brackets + syncParamWidget + updateParamLabel
    // keep the host automation and every widget in lockstep.
    static void finalEqResetAll(AbstractCymbalsUI* ui, UIStyles* styles,
                                 lv_obj_t** widgets, lv_obj_t** labels, float* fParams) {
        if (!ui) return;
        auto putDefault = [&](uint32_t idx) {
            const float def = ui->getParameterDefault(idx);
            ui->setParamValue(idx, def);
            ui->updateParamLabel(idx);
            ui->syncParamWidget(idx, def);
        };
        ui->editParameter(PluginCymbals::kParamFinalEQ1Freq, true);
        for (int band = 0; band < kFinalEqBandCount; ++band) {
            const int freqP = finalEqBandFreqParam(band);
            const int gainP = finalEqBandGainParam(band);
            const int qP = finalEqBandQParam(band);
            const int typeP = finalEqBandTypeParam(band);
            const int modelP = PluginCymbals::kParamFinalEQ1Model + band * 3;
            putDefault((uint32_t)freqP);
            putDefault((uint32_t)gainP);
            putDefault((uint32_t)qP);
            putDefault((uint32_t)typeP);
            // UnifiedFilter model / filter type / slope and the saturator are
            // part of the EQ band too; the reset must clear them or a
            // randomize/model change survives the RESET.
            putDefault((uint32_t)modelP);
            putDefault((uint32_t)(modelP + 1));
            putDefault((uint32_t)(modelP + 2));
            putDefault((uint32_t)PluginCymbals::kParamFinalEQ1Sat + (uint32_t)band);
        }
        putDefault(PluginCymbals::kParamFinalEQMix);
        putDefault(PluginCymbals::kParamFinalTiltAmount);
        putDefault(PluginCymbals::kParamFinalTiltPivot);
        putDefault(PluginCymbals::kParamFinalEQOn);
        ui->editParameter(PluginCymbals::kParamFinalEQ1Freq, false);
        (void)styles; (void)widgets; (void)labels; (void)fParams;
    }

    static void finalEqResetCb(lv_event_t* e) {
        auto* ui = static_cast<AbstractCymbalsUI*>(lv_event_get_user_data(e));
        if (!ui) return;
        finalEqResetAll(ui, nullptr, nullptr, nullptr, nullptr);
    }

    // Final-EQ dice: randomize every band plus the tilt section, leaving the
    // five band positions recognisable (freq is spread, gain stays moderate).
    static void finalEqRandomize(AbstractCymbalsUI* ui) {
        if (!ui) return;
        auto rnd = [] { return (float)std::rand() / (float)RAND_MAX; };
        auto put = [&](int idx, float v) {
            ui->editParameter(idx, true);
            ui->setParamValue(idx, v);
            ui->editParameter(idx, false);
            ui->updateParamLabel(idx);
            ui->syncParamWidget(idx, v);
        };
        for (int band = 0; band < kFinalEqBandCount; ++band) {
            put(finalEqBandFreqParam(band), 0.05f + 0.90f * rnd());
            put(finalEqBandGainParam(band), 0.35f + 0.30f * rnd());
            put(finalEqBandQParam(band), 0.20f + 0.60f * rnd());
            put(finalEqBandTypeParam(band), (float)(std::rand() % 8));
            const int model = PluginCymbals::kParamFinalEQ1Model + band * 3;
            put(model, (float)(std::rand() % (int)UnifiedFilter::Count));
            put(model + 1, (float)(std::rand() % 8));
            put(model + 2, (float)(std::rand() % 4));
            put(PluginCymbals::kParamFinalEQ1Sat + band, rnd());
        }
        put(PluginCymbals::kParamFinalEQMix, 0.50f + 0.50f * rnd());
        put(PluginCymbals::kParamFinalTiltAmount, 0.25f + 0.50f * rnd());
        put(PluginCymbals::kParamFinalTiltPivot, 0.15f + 0.70f * rnd());
    }

    static void finalEqRandomCb(lv_event_t* e) {
        auto* ui = static_cast<AbstractCymbalsUI*>(lv_event_get_user_data(e));
        if (ui) finalEqRandomize(ui);
    }

    // 33ms timer: refresh the 5 band-readout chips (freq / gain / Q + level).
    static void eqBandStripTimerCb(lv_timer_t* t) {
        auto* st = static_cast<EqBandStripState*>(lv_timer_get_user_data(t));
        if (!st || !st->ui) return;
        for (int band = 0; band < kFinalEqBandCount; ++band) {
            if (!st->chipLabels[band]) continue;
            const float freqN = st->ui->getParamValue((uint32_t)finalEqBandFreqParam(band));
            const float gainN = st->ui->getParamValue((uint32_t)finalEqBandGainParam(band));
            const float qN = st->ui->getParamValue((uint32_t)finalEqBandQParam(band));
            const int typeP = finalEqBandTypeParam(band);
            const float typeRaw = (typeP >= 0) ? st->ui->getParamValue((uint32_t)typeP) : 0.0f;
            const int typeIdx = std::clamp((int)std::round(typeRaw), 0, 7);
            const bool gainRelevant = (typeIdx >= 4 && typeIdx <= 6);
            const bool qRelevant = (typeIdx <= 3 || typeIdx == 7);
            const float freqHz = DSPUtils::normToHz(std::clamp(freqN, 0.0f, 1.0f));
            const float gainDb = (gainN - 0.5f) * 48.0f;
            const float qVal = DSPUtils::qFromParam(qN);
            char buf[64];
            if (gainRelevant) {
                if (freqHz >= 1000.0f)
                    snprintf(buf, sizeof(buf), "%s  %.1fk  %+.1f dB", finalEqBandName(band), freqHz / 1000.0f, gainDb);
                else
                    snprintf(buf, sizeof(buf), "%s  %.0f  %+.1f dB", finalEqBandName(band), freqHz, gainDb);
            } else if (qRelevant) {
                if (freqHz >= 1000.0f)
                    snprintf(buf, sizeof(buf), "%s  %.1fk  Q %.2f", finalEqBandName(band), freqHz / 1000.0f, qVal);
                else
                    snprintf(buf, sizeof(buf), "%s  %.0f  Q %.2f", finalEqBandName(band), freqHz, qVal);
            } else {
                if (freqHz >= 1000.0f)
                    snprintf(buf, sizeof(buf), "%s  %.1fk", finalEqBandName(band), freqHz / 1000.0f);
                else
                    snprintf(buf, sizeof(buf), "%s  %.0f", finalEqBandName(band), freqHz);
            }
            lv_label_set_text(st->chipLabels[band], buf);
            // Mini level meter from the analyzer
            if (st->chipMeters[band]) {
                const float level = st->ui->getAnalyzerBand(band * 12 + 4);
                lv_bar_set_value(st->chipMeters[band], (int)(level * 1000.0f), LV_ANIM_OFF);
            }
        }
        // Update the tilt readout label
        if (st->tiltReadout) {
            const float tiltAmt = st->ui->getParamValue(PluginCymbals::kParamFinalTiltAmount);
            const float tiltDb = DSPUtils::tiltDbFromParam(tiltAmt);
            const float tiltPivot = st->ui->getParamValue(PluginCymbals::kParamFinalTiltPivot);
            const float pivotHz = DSPUtils::normToHz(tiltPivot);
            char tiltBuf[32];
            if (pivotHz >= 1000.0f)
                snprintf(tiltBuf, sizeof(tiltBuf), "TILT %+.1f dB @ %.1fk", tiltDb, pivotHz / 1000.0f);
            else
                snprintf(tiltBuf, sizeof(tiltBuf), "TILT %+.1f dB @ %.0f", tiltDb, pivotHz);
            lv_label_set_text(st->tiltReadout, tiltBuf);
        }
    }

    static void eqBandStripDeleteCb(lv_event_t* e) {
        auto* st = static_cast<EqBandStripState*>(lv_event_get_user_data(e));
        if (!st) return;
        if (st->timer) { lv_timer_del(st->timer); st->timer = nullptr; }
        delete st;
    }

    // Build the 5-chip band strip as a child of eqPane.
    static void buildEqBandStrip(lv_obj_t* eqPane, AbstractCymbalsUI* ui, lv_obj_t* tiltReadout) {
        lv_obj_t* strip = lv_obj_create(eqPane);
        lv_obj_set_size(strip, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(strip, 0, 0);
        lv_obj_set_style_border_width(strip, 0, 0);
        lv_obj_set_style_pad_all(strip, 0, 0);
        lv_obj_set_style_pad_row(strip, 0, 0);
        lv_obj_set_style_pad_column(strip, scaled(4), 0);
        lv_obj_set_layout(strip, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(strip, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(strip, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(strip, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(strip, LV_SCROLLBAR_MODE_OFF);
        // Move to just after mainCont (which is child 0 of eqPane)
        lv_obj_move_to_index(strip, 1);

        auto* st = new EqBandStripState();
        st->ui = ui;
        st->tiltReadout = tiltReadout;
        lv_obj_add_event_cb(strip, eqBandStripDeleteCb, LV_EVENT_DELETE, st);
        st->timer = lv_timer_create(eqBandStripTimerCb, 33, st);

        for (int band = 0; band < kFinalEqBandCount; ++band) {
            lv_obj_t* chip = lv_obj_create(strip);
            // FIXED width, not LV_SIZE_CONTENT: the strip is SPACE_BETWEEN, so
            // a label whose text changes width (e.g. "LOW 378 +21.9 dB" ->
            // "LOW 3.8k +21.9 dB") re-distributed all five chips every 33 ms
            // and the meter line under each chip jumped sideways while a knob
            // was moving. The widest readout ("HIGH  16.0k  +24.0 dB") sets
            // the width; the label is centred and DOT-truncated so it can
            // never wrap or push the strip taller.
            lv_obj_set_size(chip, scaled(158), LV_SIZE_CONTENT);
            lv_obj_set_style_bg_color(chip, COL_PANEL_DARK, 0);
            lv_obj_set_style_bg_opa(chip, LV_OPA_COVER, 0);
            lv_obj_set_style_border_color(chip, finalEqBandColor(band), 0);
            lv_obj_set_style_border_width(chip, 1, 0);
            lv_obj_set_style_radius(chip, scaled(4), 0);
            lv_obj_set_style_pad_hor(chip, scaled(6), 0);
            lv_obj_set_style_pad_ver(chip, scaled(3), 0);
            lv_obj_set_layout(chip, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(chip, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(chip, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(chip, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_scrollbar_mode(chip, LV_SCROLLBAR_MODE_OFF);

            lv_obj_t* lbl = lv_label_create(chip);
            lv_label_set_text(lbl, finalEqBandName(band));
            lv_obj_set_width(lbl, scaled(146));
            lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
            lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
            lv_obj_set_style_text_font(lbl, getScaledSmallFont(), 0);
            lv_obj_set_style_text_color(lbl, finalEqBandColor(band), 0);
            st->chipLabels[band] = lbl;

            lv_obj_t* meter = lv_bar_create(chip);
            lv_obj_set_size(meter, scaled(20), scaled(4));
            lv_obj_set_style_bg_color(meter, COL_EQ_GRID, LV_PART_MAIN);
            lv_obj_set_style_bg_color(meter, finalEqBandColor(band), LV_PART_INDICATOR);
            lv_bar_set_range(meter, 0, 1000);
            lv_bar_set_value(meter, 0, LV_ANIM_OFF);
            st->chipMeters[band] = meter;
        }
    }

    static int finalFreqParam(int comb) { return PluginCymbals::kParamFinalComb1Freq + comb * 4; }
    static int finalFBParam(int comb) { return PluginCymbals::kParamFinalComb1Freq + comb * 4 + 1; }
    static int finalQParam(int comb) { return PluginCymbals::kParamFinalComb1Q + comb; }
    static int finalLPFParam(int comb) { return PluginCymbals::kParamFinalComb1Freq + comb * 4 + 2; }
    static int finalHPFParam(int comb) { return PluginCymbals::kParamFinalComb1Freq + comb * 4 + 3; }
    static int finalScaleParam(int comb) { (void)comb; return PluginCymbals::kParamFinalCombScale; }
    static int finalMixParam(int comb) { return PluginCymbals::kParamFinalComb1Mix + comb; }
    static int finalSatAmountParam(int comb) { (void)comb; return PluginCymbals::kParamFinalComb4SatAmount; }
    static int finalStressOnsetParam(int comb) { (void)comb; return PluginCymbals::kParamFinalComb4StressOnset; }
    static int finalStressKneeParam(int comb) { (void)comb; return PluginCymbals::kParamFinalComb4StressKnee; }
    static int finalSatTypeParam(int comb) { (void)comb; return PluginCymbals::kParamFinalComb4SatType; }
    static int finalFilterTypeParam(int comb) { return PluginCymbals::kParamFinalComb1FilterType + comb; }
    static int finalModeRateParam(int comb) { return PluginCymbals::kParamFinalComb1ModeRate + comb; }
    static int finalModeAmountParam(int comb) { return PluginCymbals::kParamFinalComb1ModeAmount + comb; }
    static int finalModeAParam(int comb) { return PluginCymbals::kParamFinalComb1ModeA + comb; }
    static int finalModeBParam(int comb) { return PluginCymbals::kParamFinalComb1ModeB + comb; }
    static int finalPartRelPosParam(int comb) { return PluginCymbals::kParamFinalComb1PartRelPos + comb; }
    static int finalPartFreqDampParam(int comb) { return PluginCymbals::kParamFinalComb1PartFreqDamp + comb; }
    static int finalPartStiffnessParam(int comb) { return PluginCymbals::kParamFinalComb1PartStiffness + comb; }
    static int finalPartStiffExpParam(int comb) { return PluginCymbals::kParamFinalComb1PartStiffExp + comb; }
    static int finalPartCountParam(int comb) { return PluginCymbals::kParamFinalComb1PartCount + comb; }
    static int finalSoloParam(int comb) { return PluginCymbals::kParamFinalComb1Solo + comb; }
    static int finalEnableParam(int comb) { return PluginCymbals::kParamFinalComb1Enable + comb; }

    static const CombEditorDock::BehaviorParamMap& finalBehaviorMap() {
        static const CombEditorDock::BehaviorParamMap kMap = {
            4,
            4,
            finalFreqParam,
            finalFBParam,
            finalQParam,
            finalLPFParam,
            finalHPFParam,
            finalScaleParam,
            finalMixParam,
            finalSatAmountParam,
            finalStressOnsetParam,
            finalStressKneeParam,
            finalSatTypeParam,
            finalFilterTypeParam,
            finalSoloParam,
            finalEnableParam,
            finalModeRateParam,
            finalModeAmountParam,
            finalPartRelPosParam,
            finalPartFreqDampParam,
            finalPartStiffnessParam,
            finalPartStiffExpParam,
            finalPartCountParam,
            finalModeAParam,
            finalModeBParam,
        };
        return kMap;
    }

    static bool isDescendantOf(lv_obj_t* obj, lv_obj_t* ancestor) {
        while (obj != nullptr) {
            if (obj == ancestor) return true;
            obj = lv_obj_get_parent(obj);
        }
        return false;
    }

    // LVGL bubbles an event to an ancestor only when the ORIGINAL target (or
    // an intermediate child) carries LV_OBJ_FLAG_EVENT_BUBBLE. Setting it on
    // combPane alone therefore never delivered presses on the FILTERBANK
    // header controls to clearSelectionOutsideXyCb: clicking the header left
    // the filter editor open. Mirror the recursive setup Stage3Tab uses so
    // every header control reports its press to the pane, while presses on
    // the XY pad or the dock are filtered out by clearSelectionOutsideXyCb.
    static void enablePressBubbleRecursive(lv_obj_t* root) {
        if (!root) return;
        if (lv_obj_is_clickable(root)) lv_obj_add_flag(root, LV_OBJ_FLAG_EVENT_BUBBLE);
        const uint32_t n = lv_obj_get_child_count(root);
        for (uint32_t i = 0; i < n; ++i)
            enablePressBubbleRecursive(lv_obj_get_child(root, i));
    }

    static void clearSelectionOutsideXyCb(lv_event_t* e) {
        auto* st = static_cast<FinalEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->xyPad) return;
        lv_obj_t* target = static_cast<lv_obj_t*>(lv_event_get_target(e));
        if (isDescendantOf(target, st->xyPad)) return;
        if (st->panel && isDescendantOf(target, st->panel)) return;
        // The 1-4 strip is an editor control, not "outside": its CLICKED
        // handler toggles one bit and must see the pre-click mask. If the
        // press bubbled through the tab/pane dismiss handler first, every
        // strip click would clear all four and multi-select could never
        // accumulate 1+2+3+4.
        for (int i = 0; i < 4; ++i) {
            lv_obj_t* b = st->selectionButtons[i];
            if (b && isDescendantOf(target, b)) return;
        }
        UIXYPad::clearSelection(st->xyPad);
    }

    static int getTargetMask(FinalEditorState* st) {
        return CombEditorDock::getTargetMask(st);
    }

    static void refreshFinalEditor(FinalEditorState* st) {
        CombEditorDock::refreshEditorFromSelection(st, finalBehaviorMap());
    }

    // OWNERSHIP CLEANUP: Delete heap-allocated state when LVGL destroys object
    static void deleteStateCb(lv_event_t* e) {
        auto* st = static_cast<FinalEditorState*>(lv_event_get_user_data(e));
        // Fix #1 (Round 2): registered on combPane (LV_EVENT_DELETE at
        // buildFinalEditor). The COLOUR tab has no mid-session rebuild (fixed
        // 4-comb bank, no lv_obj_clean of registered controls), so today this
        // fires during whole-screen teardown — but the owners still die in
        // that same deletion, so restoring them (old Fix B behavior) is just
        // as wrong as on Stage3Tab: it plants pointers to objects queued to
        // die in this very pass. Null the slots instead; restore stays in
        // the rebind/dock-close paths (CombEditorDock.hpp:252 / :441).
        if (st) {
            if (st->panel && st->panelDetached) {
                lv_obj_delete(st->panel);
                st->panel = nullptr;
            }
            for (int k = 0; k < st->modParamCount; ++k) {
                const uint32_t p = st->modParams[k];
                if (st->widgets && p && p < PluginCymbals::kParameterCount)
                    st->widgets[p] = nullptr;
                st->savedWidgets[k] = nullptr;
            }
        }
        delete st;
    }

    static void finalArcSetCb(lv_event_t* e) {
        auto* st = static_cast<FinalEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->ui) return;
        // Programmatic sync (syncParamWidget during an XY-pad drag) re-fires
        // this callback. Only respond to real user edits on the arc itself.
        if (st->ui->isInSync()) return;
        lv_obj_t* arc = static_cast<lv_obj_t*>(lv_event_get_target(e));
        const auto wIt = CombEditorDock::arcWhichMap().find(arc);
        const int which = (wIt != CombEditorDock::arcWhichMap().end()) ? wIt->second : -1;
        if (which < 0) return;
        const float v = (float)lv_arc_get_value(arc) / 1000.0f;
        CombEditorDock::applyArcToSelection(st, finalBehaviorMap(), arc, which, v);
    }

    static void finalCombEnableCb(lv_event_t* e) {
        auto* st = static_cast<FinalEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->ui) return;
        const float v = lv_obj_has_state(static_cast<lv_obj_t*>(lv_event_get_target(e)), LV_STATE_CHECKED) ? 1.0f : 0.0f;
        CombEditorDock::applyEnableToSelection(st, finalBehaviorMap(), v);
    }

    static void finalSoloCb(lv_event_t* e) {
        auto* st = static_cast<FinalEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->ui) return;
        const float v = lv_obj_has_state(static_cast<lv_obj_t*>(lv_event_get_target(e)), LV_STATE_CHECKED) ? 1.0f : 0.0f;
        CombEditorDock::applySoloToSelection(st, finalBehaviorMap(), v);
    }

    static void finalSatTypeCb(lv_event_t* e) {
        auto* st = static_cast<FinalEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->ui) return;
        const int t = (int)lv_dropdown_get_selected(static_cast<lv_obj_t*>(lv_event_get_target(e)));
        CombEditorDock::applySatTypeToSelection(st, finalBehaviorMap(), t);
    }

    static void finalFilterTypeCb(lv_event_t* e) {
        auto* st = static_cast<FinalEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->ui) return;
        const int t = (int)lv_dropdown_get_selected(static_cast<lv_obj_t*>(lv_event_get_target(e)));
        CombEditorDock::applyFilterTypeToSelection(st, finalBehaviorMap(), t);
    }

    static void finalPhaseCb(lv_event_t* e) {
        auto* st = static_cast<FinalEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->ui || !st->xyPad) return;
        lv_obj_t* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
        UIXYPad::setPhaseView(st->xyPad, lv_obj_has_state(btn, LV_STATE_CHECKED));
    }

    static lv_coord_t clampCoord(lv_coord_t v, lv_coord_t lo, lv_coord_t hi) {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

    static void dragPanelByHeaderCb(lv_event_t* e) {
        lv_obj_t* panel = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
        if (!panel) return;
        lv_obj_t* parent = lv_obj_get_parent(panel);
        if (!parent) return;
        lv_indev_t* indev = lv_indev_get_act();
        if (!indev) return;

        lv_point_t vect;
        lv_indev_get_vect(indev, &vect);

        const lv_coord_t pw = lv_obj_get_width(parent);
        const lv_coord_t ph = lv_obj_get_height(parent);
        const lv_coord_t w = lv_obj_get_width(panel);
        const lv_coord_t h = lv_obj_get_height(panel);

        lv_coord_t nx = lv_obj_get_x(panel) + vect.x;
        lv_coord_t ny = lv_obj_get_y(panel) + vect.y;

        nx = clampCoord(nx, 0, pw > w ? pw - w : 0);
        ny = clampCoord(ny, 0, ph > h ? ph - h : 0);
        lv_obj_set_pos(panel, nx, ny);
    }

    static void finalCircleTouched(void* userData, int groupIdx, int localCombIdx) {
        (void)groupIdx;
        auto* st = static_cast<FinalEditorState*>(userData);
        if (!st) return;
        st->selectedComb = std::clamp(localCombIdx, 0, 3);
        if (st->panel) {
            // Show first: the dock is content-sized, and the default placement
            // re-measures its settled height so the initial position clears
            // the keyboard/spectrum footer. A saved user drag is reused as-is
            // (including a deliberate position over the footer).
            lv_obj_clear_flag(st->panel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(st->panel); // footer is a later sibling
            if (!CombEditorDock::applySavedDockPos(st->panel)) {
                CombEditorDock::placeDockAtDefault(st->panel, st->xyPad);
            }
        }
        syncEditorFromSelection(st);
        syncSelectionButtons(st);
    }

    static void syncEditorFromSelection(FinalEditorState* st) {
        CombEditorDock::syncEditorFromXySelection(st, finalBehaviorMap());
    }

    static void syncSelectionButtons(FinalEditorState* st) {
        if (!st) return;
        // Mirror the FULL pad selection (a lasso can select 1+2+3+4); see the
        // main-bank Stage3Tab twin. selectedComb remains the dock's edit focus.
        const int mask = UIXYPad::getSelectedMask(st->xyPad);
        for (int i = 0; i < 4; ++i) {
            lv_obj_t* b = st->selectionButtons[i];
            if (!b) continue;
            if (mask & (1 << i)) lv_obj_add_state(b, LV_STATE_CHECKED);
            else                 lv_obj_clear_state(b, LV_STATE_CHECKED);
        }
    }

    static void selectionButtonCb(lv_event_t* e) {
        auto* st = static_cast<FinalEditorState*>(lv_event_get_user_data(e));
        lv_obj_t* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
        if (!st || !btn || !st->xyPad) return;
        const int idx = uiDecodeNonParamPayload((intptr_t)lv_obj_get_user_data(btn));
        UIXYPad::selectComb(st->xyPad, idx);
        // Toggle-off: keep the dock focused on a filter that is still selected
        // (or close it when the mask empties); see the Stage3 twin.
        const int mask = UIXYPad::getSelectedMask(st->xyPad);
        if (mask == 0) {
            st->selectedComb = -1;
        } else if ((mask & (1 << idx)) == 0) {
            for (int i = 0; i < 4; ++i) {
                if (mask & (1 << i)) {
                    st->selectedComb = st->groupIdx * 4 + i;
                    break;
                }
            }
        }
        syncEditorFromSelection(st);
        syncSelectionButtons(st);
    }

    static void xySelectionSyncCb(lv_event_t* e) {
        auto* st = static_cast<FinalEditorState*>(lv_event_get_user_data(e));
        if (!st) return;
        syncEditorFromSelection(st);
        syncSelectionButtons(st);
    }

    static void build(lv_obj_t* tab, AbstractCymbalsUI* ui, UIStyles& styles,
                     lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                     lv_event_cb_t valueCb, lv_event_cb_t rollerCb,
                     lv_obj_t** outFinalXYPad, lv_event_cb_t randomizeCb, lv_event_cb_t tuningCb,
                     lv_event_cb_t groupLPFHPFCb = nullptr,
                     lv_event_cb_t subTabCb = nullptr, lv_obj_t** outSubTabBar = nullptr,
                     lv_obj_t** outArea = nullptr,
                     lv_event_cb_t randomizeColourCb = nullptr,
                     lv_obj_t** secondaryWidgets = nullptr,
                     lv_event_cb_t randomizeTunedCb = nullptr) {
        // The final EQ has no roller control any more (the band strip and the
        // node graph drive it), so the roller callback is part of the shared
        // tab-builder signature but unused here.
        (void)rollerCb;

        // Tab: COLUMN layout
        lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_layout(tab, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(tab, scaled(8), 0);
        lv_obj_set_style_pad_row(tab, scaled(8), 0);
        lv_obj_set_scrollbar_mode(tab, LV_SCROLLBAR_MODE_OFF);

        // =========================================================
        // Uniform screen strip: COMB BANK / EQ sub-tab bar
        // =========================================================
        lv_obj_t* subTabBar = nullptr;
        if (outSubTabBar) {
            *outSubTabBar = UIWidgets::createScreenStrip(tab, styles, "COLOUR");
            subTabBar = *outSubTabBar;
        } else {
            subTabBar = lv_obj_create(tab);
            lv_obj_set_size(subTabBar, lv_pct(100), scaled(34));
            lv_obj_set_style_bg_opa(subTabBar, 0, 0);
            lv_obj_set_style_border_width(subTabBar, 0, 0);
            lv_obj_set_style_pad_all(subTabBar, 0, 0);
            lv_obj_set_layout(subTabBar, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(subTabBar, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(subTabBar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_style_pad_column(subTabBar, scaled(6), 0);
            lv_obj_clear_flag(subTabBar, LV_OBJ_FLAG_SCROLLABLE);
        }

        const char* names[2] = {"FILTER BANK", "EQ"};
        const int tabOnParam[2] = {PluginCymbals::kParamFinalCombBankOn,
                                   PluginCymbals::kParamFinalEQOn};
        for (int g = 0; g < 2; ++g) {
            lv_obj_t* btn = lv_btn_create(subTabBar);
            lv_obj_set_size(btn, scaled(140), scaled(32));
            lv_obj_set_style_radius(btn, 0, 0);
            lv_obj_set_style_pad_all(btn, 0, 0);
            lv_obj_set_layout(btn, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            styles.applyMomentaryButton(btn);
            // Caption colour follows the button state so it can never be
            // left black for an unselected tab: white normally, black on the
            // checked (cyan) tab.
            lv_obj_set_style_text_color(btn, COL_TEXT, 0);
            lv_obj_set_style_text_color(btn, COL_BG, LV_STATE_CHECKED);
            lv_obj_set_user_data(btn, reinterpret_cast<void*>((intptr_t)g));
            if (subTabCb != nullptr) {
                lv_obj_add_event_cb(btn, subTabCb, LV_EVENT_PRESSED, ui);
                lv_obj_add_event_cb(btn, subTabCb, LV_EVENT_RELEASED, ui);
                lv_obj_add_event_cb(btn, subTabCb, LV_EVENT_CLICKED, ui);
            }
            lv_obj_set_style_bg_color(btn, COL_HIGHLIGHT, LV_STATE_CHECKED);
            lv_obj_set_style_bg_grad_dir(btn, LV_GRAD_DIR_NONE, LV_STATE_CHECKED);

            // LEFT ZONE: same ON/OFF language as each comb group tab.
            const int onParam = tabOnParam[g];
            lv_obj_t* onZone = lv_btn_create(btn);
            lv_obj_set_size(onZone, scaled(26), lv_pct(100));
            lv_obj_set_style_radius(onZone, 0, 0);
            lv_obj_set_style_pad_all(onZone, 0, 0);
            lv_obj_add_flag(onZone, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_user_data(onZone, reinterpret_cast<void*>((intptr_t)onParam));
            lv_obj_add_event_cb(onZone, valueCb, LV_EVENT_VALUE_CHANGED, ui);
            widgets[onParam] = onZone;
            // FX CHAIN registers its own BUS toggle on kParamFinalCombBankOn
            // later in the build and overwrites widgets[]. Keep the tab's
            // power zone in the secondary slot so both twins follow the value
            // (the random dice proved the primary-only registration left this
            // plate cyan while the bank was off).
            if (secondaryWidgets) secondaryWidgets[onParam] = onZone;
            styles.applyPowerZoneToggle(onZone, fParams[onParam] > 0.5f);
            lv_obj_set_style_bg_color(onZone, COL_HIGHLIGHT, LV_STATE_CHECKED);
            lv_obj_set_style_bg_grad_dir(onZone, LV_GRAD_DIR_NONE, LV_STATE_CHECKED);

            // Caption in a plain wrapper, not directly in the checkable
            // button: a label parented to the button resolved its text colour
            // from the button's CHECKED style on first paint and stayed black.
            lv_obj_t* capBox = lv_obj_create(btn);
            lv_obj_set_size(capBox, 0, lv_pct(100));
            lv_obj_set_flex_grow(capBox, 1);
            lv_obj_set_style_bg_opa(capBox, 0, 0);
            lv_obj_set_style_border_width(capBox, 0, 0);
            lv_obj_set_style_pad_all(capBox, 0, 0);
            lv_obj_clear_flag(capBox, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(capBox, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_t* lbl = lv_label_create(capBox);
            lv_label_set_text(lbl, names[g]);
            lv_obj_set_style_text_font(lbl, getScaledSmallFont(), 0);
            lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(lbl, g == 0 ? COL_BG : COL_TEXT, 0);
            lv_obj_center(lbl);
            if (g == 0) lv_obj_add_state(btn, LV_STATE_CHECKED);
        }

        // Signal-flow cue: the post-filter bank feeds the EQ on this screen,
        // matching the "-> BUS / -> DELAY" captions on DISPERSION.
        {
            lv_obj_t* flow = lv_label_create(subTabBar);
            lv_label_set_text(flow, "FILTER BANK  " LV_SYMBOL_RIGHT "  EQ");
            lv_obj_set_style_text_font(flow, uiMicroFont(), 0);
            lv_obj_set_style_text_color(flow, COL_TEXT_DIM, 0);
            lv_obj_set_style_text_letter_space(flow, scaled(1), 0);
            lv_obj_set_user_data(flow, (void*)(intptr_t)-1);
            lv_obj_set_style_pad_left(flow, scaled(8), 0);
        }

        // Keep the navigation tabs on the left, matching FILTERS/DISPERSION;
        // the strip spacer moves to the end so the flow cue follows them.
        if (lv_obj_get_child_cnt(subTabBar) > 0)
            lv_obj_move_to_index(lv_obj_get_child(subTabBar, 1),
                                 (int32_t)lv_obj_get_child_cnt(subTabBar) - 1);

        // Right-side dice: randomize the post-filter bank and the final EQ.
        if (randomizeColourCb != nullptr) {
            lv_obj_t* dice = lv_btn_create(subTabBar);
            lv_obj_set_size(dice, scaled(28), scaled(28));
            styles.applyMomentaryButton(dice);
            lv_obj_clear_state(dice, LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(dice, lv_color_hex(0x36393E), 0);
            lv_obj_t* die = uiDiceIcon(dice, COL_TEXT);
            if (die != nullptr) lv_obj_center(die);
            lv_obj_set_user_data(dice, (void*)(intptr_t)-1);
            lv_obj_add_event_cb(dice, randomizeColourCb, LV_EVENT_CLICKED, ui);
            uiAttachTooltip(dice, "Randomize the filter bank and EQ");
        }

        // =========================================================
        // Sub-tab area: comb pane + EQ pane, one visible at a time
        // =========================================================
        lv_obj_t* area = lv_obj_create(tab);
        lv_obj_set_size(area, lv_pct(100), 0);
        lv_obj_set_flex_grow(area, 1);
        lv_obj_set_style_bg_opa(area, 0, 0);
        lv_obj_set_style_border_width(area, 0, 0);
        lv_obj_set_style_pad_all(area, 0, 0);
        lv_obj_set_layout(area, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(area, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_clear_flag(area, LV_OBJ_FLAG_SCROLLABLE);
        if (outArea) *outArea = area;

        // =========================================================
        // TOP: Final Comb Group
        // =========================================================
        lv_obj_t* combPane = lv_obj_create(area);
        lv_obj_set_size(combPane, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(combPane, 1);
        lv_obj_add_style(combPane, &styles.panel, 0);
        lv_obj_set_flex_align(combPane, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(combPane, LV_OBJ_FLAG_SCROLLABLE);

        CombGroupHeader::Config headerCfg;
        headerCfg.title = "FILTERBANK";
        headerCfg.paramLPF = PluginCymbals::kParamFinalComb1LPF;
        headerCfg.paramHPF = PluginCymbals::kParamFinalComb1HPF;
        headerCfg.paramShift = PluginCymbals::kParamFinalCombShift;
        headerCfg.paramGain = PluginCymbals::kParamFinalCombGroupGain;
        headerCfg.paramSeries = PluginCymbals::kParamFinalCombBankSeries;
        headerCfg.paramCluster = PluginCymbals::kParamFinalCombGroupCluster;
        headerCfg.paramOn = -1; // BUS moved into the FILTER BANK tab ON zone
        headerCfg.groupIdx = -1; // Final bank
        // The post-filter bank gets the same peak limiter as the ten main-bank
        // groups (DSP: finalCombBank group 0, where this bank routes).
        headerCfg.paramLimitOn = PluginCymbals::kParamFinalCombLimiterOn;
        headerCfg.paramLimitThresh = PluginCymbals::kParamFinalCombLimiterThreshold;
        headerCfg.paramLimitRel = PluginCymbals::kParamFinalCombLimiterRelease;
        headerCfg.valueCb = valueCb;
        headerCfg.tuningCb = tuningCb;
        headerCfg.randomizeCb = randomizeCb;
        // Cymbal random: reroll the final bank and apply a musical tuning
        // (the same per-group action the FILTERS headers expose).
        headerCfg.randomizeTunedCb = randomizeTunedCb;
        headerCfg.groupLPFHPFCb = groupLPFHPFCb;

        // Created before the header: the header builds the 1-4 selection strip
        // and publishes its buttons into this state.
        auto* finalState = new FinalEditorState();
        finalState->isFinal = true;
        finalState->ui = ui;
        finalState->widgets = widgets;
        headerCfg.selectionCb = selectionButtonCb;
        headerCfg.selectionUserData = finalState;
        headerCfg.selectionButtons = finalState->selectionButtons;

        CombGroupHeader::build(combPane, ui, styles, widgets, labels, fParams, headerCfg);

        lv_obj_add_flag(combPane, LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_obj_add_event_cb(combPane, clearSelectionOutsideXyCb, LV_EVENT_PRESSED, finalState);
        lv_obj_add_event_cb(combPane, deleteStateCb, LV_EVENT_DELETE, finalState);

        lv_obj_t* xy = UIXYPad::create(combPane, 0, ui, styles, true, finalCircleTouched, finalState);
        finalState->xyPad = xy;
        lv_obj_set_width(xy, lv_pct(100));
        // Grow to absorb the pane's spare height (min-height floors it).
        lv_obj_set_flex_grow(xy, 1);
        // 296, the same floor the FILTERS bank's pad uses (Stage3Tab.hpp) -
        // the two banks are the same control surface and must not differ in
        // plot height.
        lv_obj_set_style_min_height(xy, scaled(296), 0);
        lv_obj_add_event_cb(xy, xySelectionSyncCb, LV_EVENT_VALUE_CHANGED, finalState);



        // FIX (dock layout): width covers the widest control row (10
        // ring-safe arc columns @64px, ROW_WRAP); the SOLO/ON/PHASE stack
        // lives in its own full-width end-aligned row below the arcs row
        // (createActionRow), so no filter type can overflow the panel width
        // or jump position. Height is content-sized so the panel always
        // encloses its rows (the old fixed scaled(206) clipped the second
        // row once partials types showed it).
        //
        // 896, the same width the FILTERS bank's dock uses (Stage3Tab.hpp):
        // at 792 the 10 knob columns plus the 76px SOLO/ON/PHASE action column
        // do not fit on one line (10*72 + 9*6 + 6 + 76 = 856), so the action
        // column takes its own full-width band and the two knob rows read as
        // separated by a 95px gap. This page's pane is the full tab width -
        // it has no left rail - so 896 fits its parent exactly, as it does
        // on FILTERS.
        lv_obj_t* editor = CombEditorDock::createPanel(combPane, styles, scaled(1024), LV_SIZE_CONTENT);
        finalState->panel = editor;
        if (lv_obj_t* host = uiOverlayRootFor(combPane)) {
            lv_obj_set_parent(editor, host);
            lv_obj_move_foreground(editor);
            lv_obj_set_user_data(editor, (void*)(intptr_t)kUIFloatDockPayload);
            finalState->panelDetached = true;
        }
        lv_obj_add_flag(editor, LV_OBJ_FLAG_HIDDEN);
        // Floating overlay dock: flex-skipped, so opening it never overflows or
        // clips the pane. Positioned in finalCircleTouched.
        lv_obj_add_flag(editor, LV_OBJ_FLAG_FLOATING);
        CombEditorDock::createHeader(editor,
                         styles,
                         "FILTER",
                         "SAT TYPE",
                         "Tanh\nSoftsign\nArctan\nCubic\nHardClip\nFoldback\nAsymmetric\nSine",
                         finalSatTypeCb,
                         finalState,
                         dragPanelByHeaderCb,
                         editor,
                         &finalState->titleLabel,
                         &finalState->satTypeDropdown,
                         "FILTER TYPE",
                         "Comb\nAllpass\nNotch\nResoLP\nResoBP\nResoHP\nPhaser\nFlange\nGlitchPM\nBitcrush\nPop\nStutter\nVowel\nVocal\nTalkbox\nBeam Clamp\nBeam Open\nBeating\nMarimba\nMembrane\nMetal\nPan\nTube",
                         finalFilterTypeCb,
                         &finalState->filterTypeDropdown);

        lv_dropdown_set_selected(finalState->satTypeDropdown, (int)fParams[PluginCymbals::kParamFinalComb4SatType]);
        // Initial selection only: at build time selectedComb is -1, so any
        // selection path triggered here is a no-op in applyFilterTypeToSelection.
        lv_dropdown_set_selected(finalState->filterTypeDropdown, (int)fParams[PluginCymbals::kParamFinalComb1FilterType]);
        widgets[PluginCymbals::kParamFinalComb4SatType] = finalState->satTypeDropdown;
        // NOT registered in widgets[]: global param sync (preset load / automation)
        // would lv_dropdown_set_selected() this dropdown to comb 0's value,
        // corrupting what the dock shows for the current multi-comb selection.
        // (lv_dropdown_set_selected sends no VALUE_CHANGED event; the risk is
        // display corruption, not DSP state.) The dock refreshes the dropdown
        // from the selection instead (refreshEditorFromSelection); the Stage3
        // dropdown follows the same unregistered pattern.

        lv_obj_t* eTop = CombEditorDock::createControlRow(editor, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER);
        // SOLO/ON/PHASE host: the action column is the LAST child of the arcs
        // row, right of every arc - the same arrangement the FILTERS bank
        // uses. It used to be a dedicated full-width row between the arcs row
        // and the model row, and that third row took the height the model row
        // needs: the panel is content-sized, so the model row measured 0 and
        // the Saike partials controls never appeared on this page.
        lv_obj_t* eActions = eTop;

        finalState->arcFreq = CombEditorDock::createArc(eTop, styles, "Tune", ui->getParamValue(PluginCymbals::kParamFinalComb1Freq), 14, finalArcSetCb, finalState);
        finalState->arcFB = CombEditorDock::createArc(eTop, styles, "FB", ui->getParamValue(PluginCymbals::kParamFinalComb1FB), 15, finalArcSetCb, finalState);
        finalState->arcQ = CombEditorDock::createArc(eTop, styles, "Q", ui->getParamValue(PluginCymbals::kParamFinalComb1Q), 18, finalArcSetCb, finalState);
        finalState->arcLPF = CombEditorDock::createArc(eTop, styles, "Low Cut", ui->getParamValue(PluginCymbals::kParamFinalComb1LPF), 0, finalArcSetCb, finalState);
        finalState->arcHPF = CombEditorDock::createArc(eTop, styles, "High Cut", ui->getParamValue(PluginCymbals::kParamFinalComb1HPF), 1, finalArcSetCb, finalState);
        finalState->arcScale = CombEditorDock::createArc(eTop, styles, "Scale", ui->getParamValue(PluginCymbals::kParamFinalCombScale), 2, finalArcSetCb, finalState);
        finalState->arcMix = CombEditorDock::createArc(eTop, styles, "Mix", ui->getParamValue(PluginCymbals::kParamFinalComb1Mix), 3, finalArcSetCb, finalState);
        finalState->arcSatAmt = CombEditorDock::createArc(eTop, styles, "Sat Amt", ui->getParamValue(PluginCymbals::kParamFinalComb4SatAmount), 4, finalArcSetCb, finalState);
        finalState->arcStressOnset = CombEditorDock::createArc(eTop, styles, "Stress On", ui->getParamValue(PluginCymbals::kParamFinalComb4StressOnset), 5, finalArcSetCb, finalState);
        finalState->arcStressKnee = CombEditorDock::createArc(eTop, styles, "Stress Knee", ui->getParamValue(PluginCymbals::kParamFinalComb4StressKnee), 6, finalArcSetCb, finalState);
        finalState->arcModeRate = CombEditorDock::createArc(eTop, styles, "Mode Rate", ui->getParamValue(PluginCymbals::kParamFinalComb1ModeRate), 7, finalArcSetCb, finalState);
        finalState->arcModeAmount = CombEditorDock::createArc(eTop, styles, "Mode Amt", ui->getParamValue(PluginCymbals::kParamFinalComb1ModeAmount), 8, finalArcSetCb, finalState);
        // Mode arcs hidden until a selection refresh shows them per filter type.
        CombEditorDock::setArcVisible(finalState->arcModeRate, false);
        CombEditorDock::setArcVisible(finalState->arcModeAmount, false);

        // Saike Partials model controls (second row; hidden until a partials
        // type is selected — see refreshEditorFromSelection).
        // Left-packed at the top row's pitch, matching the FILTERS bank's model
        // row: five knobs spread across the full dock width sat ~136px apart
        // while the knobs above sat 74px apart.
        lv_obj_t* eModel = CombEditorDock::createControlRow(editor, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        finalState->arcPartRelPos = CombEditorDock::createArcBelow(eModel, styles, "Rel Pos", ui->getParamValue(PluginCymbals::kParamFinalComb1PartRelPos), 9, finalArcSetCb, finalState);
        finalState->arcPartFreqDamp = CombEditorDock::createArcBelow(eModel, styles, "Freq Damp", ui->getParamValue(PluginCymbals::kParamFinalComb1PartFreqDamp), 10, finalArcSetCb, finalState);
        finalState->arcPartStiffness = CombEditorDock::createArcBelow(eModel, styles, "Stiff", ui->getParamValue(PluginCymbals::kParamFinalComb1PartStiffness), 11, finalArcSetCb, finalState);
        finalState->arcPartStiffExp = CombEditorDock::createArcBelow(eModel, styles, "Stiff Exp", ui->getParamValue(PluginCymbals::kParamFinalComb1PartStiffExp), 12, finalArcSetCb, finalState);
        finalState->arcPartCount = CombEditorDock::createArcBelow(eModel, styles, "Parts", ui->getParamValue(PluginCymbals::kParamFinalComb1PartCount), 13, finalArcSetCb, finalState);
        CombEditorDock::setArcVisible(finalState->arcPartRelPos, false);
        CombEditorDock::setArcVisible(finalState->arcPartFreqDamp, false);
        CombEditorDock::setArcVisible(finalState->arcPartStiffness, false);
        CombEditorDock::setArcVisible(finalState->arcPartStiffExp, false);
        CombEditorDock::setArcVisible(finalState->arcPartCount, false);

        lv_obj_t* soloEnableCol = lv_obj_create(eActions);
        lv_obj_set_size(soloEnableCol, scaled(76), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(soloEnableCol, 0, 0);
        lv_obj_set_style_border_width(soloEnableCol, 0, 0);
        lv_obj_set_style_pad_all(soloEnableCol, 0, 0);
        lv_obj_set_style_pad_row(soloEnableCol, scaled(4), 0);
        lv_obj_set_layout(soloEnableCol, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(soloEnableCol, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(soloEnableCol, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_top(soloEnableCol, scaled(-4), 0);

        lv_obj_t* soloBtn = lv_btn_create(soloEnableCol);
        lv_obj_set_size(soloBtn, scaled(70), scaled(30));
        styles.applyToggleButton(soloBtn, false);
        lv_obj_t* soloLbl = lv_label_create(soloBtn);
        lv_label_set_text(soloLbl, "SOLO");
        lv_obj_center(soloLbl);
        lv_obj_add_event_cb(soloBtn, finalSoloCb, LV_EVENT_VALUE_CHANGED, finalState);
        finalState->soloButton = soloBtn;

        const float initFinalEnabled = ui->getParamValue(PluginCymbals::kParamFinalComb1Enable);
        lv_obj_t* enableBtn = lv_btn_create(soloEnableCol);
        lv_obj_set_size(enableBtn, scaled(70), scaled(30));
        styles.applyToggleButton(enableBtn, initFinalEnabled > 0.5f);
        lv_obj_t* enableLbl = lv_label_create(enableBtn);
        lv_label_set_text(enableLbl, initFinalEnabled > 0.5f ? "ON" : "OFF");
        lv_obj_center(enableLbl);
        lv_obj_add_event_cb(enableBtn, finalCombEnableCb, LV_EVENT_VALUE_CHANGED, finalState);
        finalState->enableButton = enableBtn;
        finalState->enableLabel = enableLbl;

        // PHASE toggle: flips the group's overlay curves between magnitude (dB)
        // and phase (degrees) — most useful for ALLPASS (flat magnitude).
        lv_obj_t* phaseBtn = lv_btn_create(soloEnableCol);
        lv_obj_set_size(phaseBtn, scaled(70), scaled(30));
        styles.applyToggleButton(phaseBtn, false);
        lv_obj_t* phaseLbl = lv_label_create(phaseBtn);
        lv_label_set_text(phaseLbl, "PHASE");
        lv_obj_center(phaseLbl);
        lv_obj_add_event_cb(phaseBtn, finalPhaseCb, LV_EVENT_VALUE_CHANGED, finalState);
        finalState->phaseToggleButton = phaseBtn;

        if (outFinalXYPad) *outFinalXYPad = xy;

        // --- Final EQ Panel ---
        // Built manually (not via UIEQSection::create) so the header band can
        // hold a live tilt readout + RESET/FLATTEN buttons alongside the 3 knobs,
        // and a 5-chip band-overview strip can sit below the canvas.
        lv_obj_t* eqPane = lv_obj_create(area);
        lv_obj_set_size(eqPane, lv_pct(100), lv_pct(100));
        lv_obj_set_flex_grow(eqPane, 1);
        lv_obj_set_style_bg_color(eqPane, COL_PANEL, 0);
        lv_obj_set_style_bg_opa(eqPane, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(eqPane, COL_ACCENT, 0);
        lv_obj_set_style_border_width(eqPane, 2, 0);
        lv_obj_set_style_radius(eqPane, 8, 0);
        lv_obj_set_style_pad_all(eqPane, 4, 0);
        lv_obj_set_style_pad_row(eqPane, 4, 0);
        lv_obj_set_layout(eqPane, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(eqPane, LV_FLEX_FLOW_COLUMN);
        lv_obj_clear_flag(eqPane, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(eqPane, LV_SCROLLBAR_MODE_OFF);

        // Header band: title + 3 knobs + tilt readout + RESET/FLATTEN
        lv_obj_t* eqHeader = lv_obj_create(eqPane);
        lv_obj_set_size(eqHeader, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_min_height(eqHeader, scaled(72), 0);
        lv_obj_set_style_bg_opa(eqHeader, 0, 0);
        lv_obj_set_style_border_width(eqHeader, 0, 0);
        lv_obj_set_style_pad_all(eqHeader, 0, 0);
        lv_obj_set_style_pad_column(eqHeader, scaled(6), 0);
        lv_obj_set_layout(eqHeader, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(eqHeader, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(eqHeader, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(eqHeader, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(eqHeader, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t* eqTitle = lv_label_create(eqHeader);
        lv_label_set_text(eqTitle, "FINAL BUS EQUALIZER");
        lv_obj_add_style(eqTitle, &styles.title, 0);
        lv_obj_set_flex_grow(eqTitle, 0);

        // 3 compact arcs: Mix, Tilt, Pivot
        const UIEQSection::HeaderControlSpec eqArcs[] = {
            {PluginCymbals::kParamFinalEQMix, "Mix", UIEQSection::ControlType::Arc, nullptr},
            {PluginCymbals::kParamFinalTiltAmount, "Tilt", UIEQSection::ControlType::Arc, nullptr},
            {PluginCymbals::kParamFinalTiltPivot, "Pivot", UIEQSection::ControlType::Arc, nullptr},
        };
        UIWidgets::ArcVisualSpec compactEq = UIWidgets::smallArcVisualSpec();
        compactEq.labelMarginBottom = 0;
        compactEq.valueMarginTop = 0;
        compactEq.containerH = scaled(72);
        for (int i = 0; i < 3; ++i) {
            UIWidgets::createArcEx(eqHeader, eqArcs[i].param, eqArcs[i].label,
                                   ui, styles, widgets, labels, fParams, valueCb, compactEq);
            lv_obj_t* mkc = (widgets[eqArcs[i].param] != nullptr) ? lv_obj_get_parent(widgets[eqArcs[i].param]) : nullptr;
            lv_obj_t* mcol = (mkc != nullptr) ? lv_obj_get_parent(mkc) : nullptr;
            if (mcol != nullptr) lv_obj_set_style_pad_row(mcol, 0, 0);
            uiPackKnob(widgets[eqArcs[i].param], compactEq.arcSize, 0, scaled(40));
        }

        // Tilt readout intentionally omitted: the TILT/PIVOT knobs already
        // show their values, and the extra line crowded the EQ header.
        lv_obj_t* tiltReadout = nullptr;

        // RESET (styled like every other momentary button) + right-side dice.
        lv_obj_t* resetBtn = lv_btn_create(eqHeader);
        lv_obj_set_size(resetBtn, scaled(64), scaled(28));
        styles.applyMomentaryButton(resetBtn);
        lv_obj_t* resetLbl = lv_label_create(resetBtn);
        lv_label_set_text(resetLbl, "RESET");
        lv_obj_set_style_text_font(resetLbl, getScaledSmallFont(), 0);
        lv_obj_center(resetLbl);
        lv_obj_add_event_cb(resetBtn, finalEqResetCb, LV_EVENT_CLICKED, ui);
        uiAttachTooltip(resetBtn, "Reset every final EQ band, model, slope, saturator, tilt and ON state");

        // EQ editor (canvas + freq axis + reparented header/settings)
        lv_obj_t* finalEqTopBar = nullptr;
        UIEQEditor::create(eqPane, PluginCymbals::kParamFinalEQ1Freq, ui, styles, eqHeader,
                           widgets, 0, &finalEqTopBar);

        // The random dice belongs to the RIGHT of the settings card, not
        // between the Mix/Tilt/Pivot knobs and that card.
        if (finalEqTopBar != nullptr) {
            lv_obj_t* tools = lv_obj_create(finalEqTopBar);
            lv_obj_set_size(tools, LV_SIZE_CONTENT, lv_pct(100));
            lv_obj_set_style_bg_opa(tools, 0, 0);
            lv_obj_set_style_border_width(tools, 0, 0);
            lv_obj_set_style_pad_all(tools, 0, 0);
            lv_obj_set_style_pad_column(tools, scaled(4), 0);
            lv_obj_set_layout(tools, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(tools, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(tools, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(tools, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_t* eqRandBtn = lv_btn_create(tools);
            lv_obj_set_size(eqRandBtn, scaled(28), scaled(28));
            styles.applyMomentaryButton(eqRandBtn);
            lv_obj_clear_state(eqRandBtn, LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(eqRandBtn, lv_color_hex(0x36393E), 0);
            lv_obj_t* eqDie = uiDiceIcon(eqRandBtn, COL_TEXT);
            if (eqDie != nullptr) lv_obj_center(eqDie);
            lv_obj_add_event_cb(eqRandBtn, finalEqRandomCb, LV_EVENT_CLICKED, ui);
            uiAttachTooltip(eqRandBtn, "Randomize every final EQ band and tilt setting");
        }

        // Band-overview strip: 5 chips with live freq/gain/Q + level meters
        buildEqBandStrip(eqPane, ui, tiltReadout);

        lv_obj_set_user_data(eqPane, (void*)1);
        lv_obj_add_flag(eqPane, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_user_data(combPane, (void*)0);

        // Apply the initial caption colours now. Without this the EQ label
        // kept LVGL's default black until the user clicked EQ and then
        // FILTER BANK again.
        setActiveSubtab(subTabBar, area, 0);
        lv_obj_invalidate(subTabBar);

        // Must run after the header, pad and dock children exist. LVGL
        // bubbles only from the original target, so flagging just combPane
        // left FILTERBANK header controls unable to reach the dismiss handler.
        // Keep the fix pane-scoped: the reported click is inside combPane, and
        // the existing combPane registration already excludes the XY pad and
        // the floating dock.
        enablePressBubbleRecursive(combPane);
    }

    // Toggles the COMB BANK / EQ sub-tab states + pane visibility. No
    // deletion: called from the sub-tab click (the clicked button must
    // survive the event).
    static void setActiveSubtab(lv_obj_t* subTabBar, lv_obj_t* area, int active) {
        if (!subTabBar || !area) return;
        const uint32_t nb = lv_obj_get_child_cnt(subTabBar);
        for (uint32_t i = 0; i < nb; ++i) {
            lv_obj_t* btn = lv_obj_get_child(subTabBar, i);
            const int g = (int)(intptr_t)lv_obj_get_user_data(btn);
            if (g != 0 && g != 1) continue;
            const bool isActive = (g == active);
            if (isActive) lv_obj_add_state(btn, LV_STATE_CHECKED);
            else          lv_obj_clear_state(btn, LV_STATE_CHECKED);
            lv_obj_t* cap = lv_obj_get_child(btn, 1);
            lv_obj_t* lbl = cap ? lv_obj_get_child(cap, 0) : nullptr;
            if (lbl && lv_obj_check_type(lbl, &lv_label_class)) {
                lv_obj_set_style_text_color(lbl, isActive ? COL_BG : COL_TEXT, 0);
                lv_obj_invalidate(lbl);
            }
        }
        const uint32_t ns = lv_obj_get_child_cnt(area);
        for (uint32_t i = 0; i < ns; ++i) {
            lv_obj_t* pane = lv_obj_get_child(area, i);
            const int g = (int)(intptr_t)lv_obj_get_user_data(pane);
            if (g == active) lv_obj_clear_flag(pane, LV_OBJ_FLAG_HIDDEN);
            else             lv_obj_add_flag(pane, LV_OBJ_FLAG_HIDDEN);
        }
    }
};

END_NAMESPACE_DISTRHO

#endif // STAGE_FINAL_TAB_HPP
