#ifndef COMB_GROUP_HEADER_HPP
#define COMB_GROUP_HEADER_HPP

#include "UIWidgets.hpp"
#include "UIStyles.hpp"
#include "UICommon.hpp"
#include "PluginCymbals.hpp"

START_NAMESPACE_DISTRHO

struct CombGroupHeader {
    struct Config {
        const char* title = "FILTER GROUP";
        const char* lpfLabel = "Low Cut";
        const char* hpfLabel = "High Cut";
        int paramLPF = -1;
        int paramHPF = -1;
        int paramShift = -1;
        int paramGain = -1;
        int paramSeries = -1;
        int paramCluster = -1;
        int paramOn = -1;
        int groupIdx = -1; // 0-9 for comb bank, 0xFF for final

        // Per-group peak limiter. Left at -1 the header derives the three
        // indices from groupIdx (comb groups only; the final group header has
        // no limiter), so every comb group header exposes them without the
        // call site having to spell out three more param bases.
        int paramLimitOn = -1;
        int paramLimitThresh = -1;
        int paramLimitRel = -1;

        lv_event_cb_t valueCb = nullptr;
        lv_event_cb_t tuningCb = nullptr;
        lv_event_cb_t randomizeCb = nullptr;
        lv_event_cb_t randomizeTunedCb = nullptr;
        lv_event_cb_t groupLPFHPFCb = nullptr;
        // 1-4 filter-selection strip (editor selection), built under the title.
        // The caller's callback decodes the encoded local filter index; the
        // four button pointers are published back so the selection can sync.
        lv_event_cb_t selectionCb = nullptr;
        void* selectionUserData = nullptr;
        lv_obj_t** selectionButtons = nullptr;
    };

    static void build(lv_obj_t* parent, AbstractCymbalsUI* ui, UIStyles& styles,
                     lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                     const Config& cfg) {

        // R4 piece 5: every hardcoded pixel in this header is now scaled().
        // At non-1280 widths (1568 / 736) the original 90 / 420 / 80 / 28 / 10
        // values over-crowded the card or left gaps that hid the tune row,
        // causing the header to overlap the body. Scaling all sizes to gUIScale
        // keeps the header's flex reflow consistent at every window size.
        lv_obj_t* header = lv_obj_create(parent);
        lv_obj_set_size(header, lv_pct(100), LV_SIZE_CONTENT);
        // Definite min-height so the body of the card starts below the header
        // even when the LPF/HPF/Shift/Gain/Cluster arcs are 0-px (defaults).
        lv_obj_set_style_min_height(header, scaled(40), 0);
        lv_obj_set_style_bg_opa(header, 0, 0);
        lv_obj_set_style_border_width(header, 0, 0);
        lv_obj_set_layout(header, LV_LAYOUT_FLEX);
        // ROW_WRAP: the header now also carries the loop limiter (LIM + two
        // knobs) and the nine tuning presets. On the COLOUR bank those pushed
        // Lim Rel past the right edge; wrapping moves the trailing controls
        // to a second line instead of clipping them.
        lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(header, scaled(8), 0);
        lv_obj_set_style_pad_row(header, scaled(4), 0);
        lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

        // Title column: the group name, with four per-filter on/off buttons
        // underneath (comb groups only). Each button owns one stage's Enable.
        const bool finalGroupTitle = (cfg.groupIdx < 0);
        lv_obj_t* titleCol = lv_obj_create(header);
        lv_obj_set_size(titleCol, scaled(finalGroupTitle ? 110 : 88), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(titleCol, 0, 0);
        lv_obj_set_style_border_width(titleCol, 0, 0);
        lv_obj_set_style_pad_all(titleCol, 0, 0);
        lv_obj_set_style_pad_row(titleCol, scaled(2), 0);
        lv_obj_set_layout(titleCol, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(titleCol, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(titleCol, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(titleCol, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* titleLbl = lv_label_create(titleCol);
        lv_label_set_text(titleLbl, cfg.title);
        lv_obj_add_style(titleLbl, &styles.title, 0);
        lv_obj_set_style_text_color(titleLbl, COL_TITLE, 0);
        lv_obj_set_width(titleLbl, lv_pct(100));
        lv_label_set_long_mode(titleLbl, LV_LABEL_LONG_CLIP);

        const bool mainGroup = (cfg.groupIdx >= 0 && cfg.groupIdx < 10);
        const bool finalGroup = (cfg.groupIdx < 0);

        // Filter-selection strip directly under the group title. The numbers
        // select which filter the editor dock targets; the enable toggles in
        // the row below are power glyphs so the two roles stay distinct.
        if ((mainGroup || finalGroup) && cfg.selectionCb != nullptr) {
            lv_obj_t* selRow = lv_obj_create(titleCol);
            lv_obj_set_size(selRow, scaled(86), scaled(18));
            lv_obj_set_style_bg_opa(selRow, 0, 0);
            lv_obj_set_style_border_width(selRow, 0, 0);
            lv_obj_set_style_pad_all(selRow, 0, 0);
            lv_obj_set_style_pad_column(selRow, scaled(2), 0);
            lv_obj_set_layout(selRow, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(selRow, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(selRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(selRow, LV_OBJ_FLAG_SCROLLABLE);
            for (int f = 0; f < 4; ++f) {
                lv_obj_t* b = lv_btn_create(selRow);
                lv_obj_set_size(b, scaled(20), scaled(18));
                styles.applyToggleButton(b, false);
                lv_obj_set_style_pad_all(b, 0, 0);
                lv_obj_set_user_data(b, (void*)uiEncodeNonParamPayload(f));
                lv_obj_add_event_cb(b, cfg.selectionCb, LV_EVENT_CLICKED, cfg.selectionUserData);
                lv_obj_t* n = lv_label_create(b);
                lv_obj_remove_style_all(n);
                char nb[2] = {(char)('1' + f), '\0'};
                lv_label_set_text(n, nb);
                lv_obj_set_style_text_font(n, uiMicroFont(), 0);
                lv_obj_set_style_text_color(n, COL_TEXT, 0);
                lv_obj_center(n);
                if (cfg.selectionButtons != nullptr) cfg.selectionButtons[f] = b;
            }
        }

        if ((mainGroup || finalGroup) && cfg.valueCb != nullptr) {
            lv_obj_t* row = lv_obj_create(titleCol);
            lv_obj_set_size(row, scaled(86), scaled(18));
            lv_obj_set_style_bg_opa(row, 0, 0);
            lv_obj_set_style_border_width(row, 0, 0);
            lv_obj_set_style_pad_all(row, 0, 0);
            lv_obj_set_style_pad_column(row, scaled(2), 0);
            lv_obj_set_layout(row, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
            for (int f = 0; f < 4; ++f) {
                const uint32_t p = mainGroup
                    ? (uint32_t)(PluginCymbals::kParamComb1Enable + cfg.groupIdx * 4 + f)
                    : (uint32_t)(PluginCymbals::kParamFinalComb1Enable + f);
                lv_obj_t* b = lv_btn_create(row);
                lv_obj_set_size(b, scaled(20), scaled(18));
                // Power glyph, not a digit: the numbered strip above owns
                // selection, this row owns each filter's enable state.
                styles.applyToggleButton(b, fParams[p] > 0.5f, true);
                lv_obj_set_style_pad_all(b, 0, 0);
                lv_obj_set_user_data(b, (void*)(intptr_t)p);
                lv_obj_add_event_cb(b, cfg.valueCb, LV_EVENT_VALUE_CHANGED, ui);
                widgets[p] = b;
            }
        }

        lv_event_cb_t lpfHpfCb = cfg.groupLPFHPFCb ? cfg.groupLPFHPFCb : cfg.valueCb;
        if (cfg.paramLPF >= 0) UIWidgets::createArc(header, cfg.paramLPF, cfg.lpfLabel, ui, styles, widgets, labels, fParams, lpfHpfCb);
        if (cfg.paramHPF >= 0) UIWidgets::createArc(header, cfg.paramHPF, cfg.hpfLabel, ui, styles, widgets, labels, fParams, lpfHpfCb);
        if (cfg.paramShift >= 0) UIWidgets::createArc(header, cfg.paramShift, "Shift", ui, styles, widgets, labels, fParams, cfg.valueCb);
        if (cfg.paramGain >= 0) UIWidgets::createArc(header, cfg.paramGain, "Gain", ui, styles, widgets, labels, fParams, cfg.valueCb);

        // Series/Parallel Toggle (R4 piece 5: scaled())
        if (cfg.paramSeries >= 0) {
            lv_obj_t* swBt = lv_btn_create(header);
            lv_obj_set_size(swBt, scaled(90), scaled(30));
            lv_obj_t* swLbl = lv_label_create(swBt);
            lv_label_set_text(swLbl, (fParams[cfg.paramSeries] > 0.5f) ? "SERIES" : "PARALLEL");
            lv_obj_set_style_text_font(swLbl, getScaledFont(), 0); // unified scaled
            lv_obj_center(swLbl);

            styles.applyToggleButton(swBt, fParams[cfg.paramSeries] > 0.5f);

            lv_obj_set_user_data(swBt, reinterpret_cast<void*>((intptr_t)cfg.paramSeries));
            lv_obj_add_event_cb(swBt, cfg.valueCb, LV_EVENT_VALUE_CHANGED, ui);
            widgets[cfg.paramSeries] = swBt;
        }

        // tuning unit: eight presets in two rows (plus RAND for this group; header RANDOM covers all groups).
        // R4 piece 5: 420 -> scaled(420) so the 9-button tune row tracks window width
        // (the 1280 design assumed 420 logical px; at 1568 the row no longer
        // covers all 9 buttons and the body card's flex grows into the gap).
        lv_obj_t* tuneRow = lv_obj_create(header);
        lv_obj_set_size(tuneRow, scaled(300), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(tuneRow, 0, 0);
        lv_obj_set_style_border_width(tuneRow, 0, 0);
        lv_obj_set_style_pad_all(tuneRow, 0, 0);
        lv_obj_set_layout(tuneRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(tuneRow, LV_FLEX_FLOW_ROW_WRAP);
        // Column gap 2 -> scaled(2) so the buttons stay visually tight
        // at every scale (the original 2-px gap disappears at gUIScale=0.75
        // and balloons at 1.5x; either extreme clips the 9th button).
        lv_obj_set_style_pad_column(tuneRow, scaled(2), 0);
        lv_obj_set_style_pad_row(tuneRow, scaled(2), 0);
        lv_obj_clear_flag(tuneRow, LV_OBJ_FLAG_SCROLLABLE);

        static const int tunePresets[9] = { PRESET_BESSEL, PRESET_HARM, PRESET_PRIME,
                                          PRESET_OCT, PRESET_FIFTH, PRESET_GOLD,
                                          PRESET_ET, PRESET_PRIMES, PRESET_JUST };
        static const char* tuneLabels[9] = { "BESSEL","HARM","PRIME","OCT","5TH","GOLD","ET","PRIM","JUST" };
        static const char* tuneTips[9] = { "Preset ratios: Bessel", "Preset ratios: Harmonic", "Preset ratios: Prime", "Preset ratios: Octave", "Preset ratios: Fifth", "Preset ratios: Golden ratio", "Preset ratios: Equal temperament", "Preset ratios: Primes", "Preset ratios: Just intonation" };
        for (int pi = 0; pi < 9; ++pi) {
            // Tune preset button (R4 piece 5: 80/28 -> scaled())
            lv_obj_t* btn = lv_btn_create(tuneRow);
            lv_obj_set_size(btn, scaled(80), scaled(32));
            styles.applyMomentaryButton(btn);
            lv_obj_t* lbl = lv_label_create(btn);
            lv_label_set_text(lbl, tuneLabels[pi]);
            lv_obj_set_style_text_font(lbl, getScaledFont(), 0);
            lv_obj_center(lbl);
            // Fix A.2: (preset<<8)|group stays the payload, but it sits below
            // kParameterCount and would poison widgets[] during a repoint —
            // shift it out of the guard's range. tuningPresetCallback decodes
            // before applying the >>8 / &0xFF split (0xFF still = group -1).
            intptr_t data = uiEncodeNonParamPayload(
                (int)(((intptr_t)tunePresets[pi] << 8) | (cfg.groupIdx & 0xFF)));
            lv_obj_set_user_data(btn, (void*)data);
            lv_obj_add_event_cb(btn, cfg.tuningCb, LV_EVENT_CLICKED, ui);
            uiAttachTooltip(btn, tuneTips[pi]);
        }

        // Cluster knob
        if (cfg.paramCluster >= 0) UIWidgets::createArc(header, cfg.paramCluster, "Cluster", ui, styles, widgets, labels, fParams, cfg.valueCb);

        // ---- Per-group limiter (off by default) ----
        // Compact strip: a LIM toggle plus Threshold/Release on the small knob
        // spec (56px columns, same height as the full-size arcs beside them, so
        // the header keeps its one-row height). Indices are derived from
        // groupIdx unless the call site overrode them.
        int limOn = cfg.paramLimitOn, limThr = cfg.paramLimitThresh, limRel = cfg.paramLimitRel;
        if (limOn < 0 && cfg.groupIdx >= 0 && cfg.groupIdx < 10) {
            limOn  = PluginCymbals::kParamCombGroup1LimiterOn + cfg.groupIdx;
            limThr = PluginCymbals::kParamCombGroup1LimiterThreshold + cfg.groupIdx;
            limRel = PluginCymbals::kParamCombGroup1LimiterRelease + cfg.groupIdx;
        }
        if (limOn >= 0 && limOn < PluginCymbals::kParameterCount) {
            lv_obj_t* limBt = lv_btn_create(header);
            lv_obj_set_size(limBt, scaled(44), scaled(32));
            lv_obj_add_flag(limBt, LV_OBJ_FLAG_CHECKABLE);
            const bool limIsOn = fParams[limOn] > 0.5f;
            lv_obj_t* limLbl = lv_label_create(limBt);
            // State is the toggle COLOUR (applyToggleButton); the caption stays
            // "LIM" so it always fits and does not change width when switched.
            lv_label_set_text(limLbl, "LIM");
            lv_obj_set_style_text_font(limLbl, getScaledFont(), 0);
            lv_obj_center(limLbl);
            styles.applyToggleButton(limBt, limIsOn);
            lv_obj_set_user_data(limBt, reinterpret_cast<void*>((intptr_t)limOn));
            lv_obj_add_event_cb(limBt, cfg.valueCb, LV_EVENT_VALUE_CHANGED, ui);
            uiAttachTooltip(limBt, "Group limiter on/off (threshold + release below)");
            widgets[limOn] = limBt;
            labels[limOn] = limLbl;
        }
        if (limThr >= 0 && limThr < PluginCymbals::kParameterCount) {
            UIWidgets::ArcVisualSpec limSpec = UIWidgets::smallArcVisualSpec();
            limSpec.containerW = scaled(48);   // room for the "Lim Thr" title
            limSpec.containerH = scaled(102);  // same row height as the full-size arcs
            UIWidgets::createArcEx(header, limThr, "Lim Thr", ui, styles, widgets, labels, fParams, cfg.valueCb, limSpec);
        }
        if (limRel >= 0 && limRel < PluginCymbals::kParameterCount) {
            UIWidgets::ArcVisualSpec limSpec = UIWidgets::smallArcVisualSpec();
            limSpec.containerW = scaled(48);   // matches the threshold knob
            limSpec.containerH = scaled(102);  // same row height as the full-size arcs
            UIWidgets::createArcEx(header, limRel, "Lim Rel", ui, styles, widgets, labels, fParams, cfg.valueCb, limSpec);
        }
        // Simple ON/OFF toggle (used by the final comb group header).
        if (cfg.paramOn >= 0) {
            lv_obj_t* onBt = lv_btn_create(header);
            lv_obj_set_size(onBt, scaled(56), scaled(32));
            lv_obj_add_flag(onBt, LV_OBJ_FLAG_CHECKABLE);
            bool isOn = fParams[cfg.paramOn] > 0.5f;
            lv_obj_t* onLbl = lv_label_create(onBt);
            // The whole post-filter bank: colour = state, "BUS" names the scope.
            lv_label_set_text(onLbl, "BUS");
            lv_obj_set_style_text_font(onLbl, getScaledFont(), 0);
            lv_obj_center(onLbl);
            styles.applyToggleButton(onBt, isOn);
            lv_obj_set_user_data(onBt, reinterpret_cast<void*>((intptr_t)cfg.paramOn));
            lv_obj_add_event_cb(onBt, cfg.valueCb, LV_EVENT_VALUE_CHANGED, ui);
            widgets[cfg.paramOn] = onBt;
            labels[cfg.paramOn] = onLbl;
        }

        // Group dice at the right end of the header row (was second in the
        // tune row). Local sentinel payload, decoded by randomizeCombGroupCb.
        // Guard on the callback: a header built without one must not grow a
        // dead button.
        if (cfg.randomizeTunedCb != nullptr) {
            // Cymbal = structural/settings random with a musical tuning.
            lv_obj_t* cymBtn = lv_btn_create(header);
            lv_obj_set_size(cymBtn, scaled(28), scaled(32));
            styles.applyMomentaryButton(cymBtn);
            lv_obj_t* cym = uiCymbalIcon(cymBtn, COL_TEXT);
            if (cym != nullptr) lv_obj_center(cym);
            lv_obj_set_user_data(cymBtn,
                reinterpret_cast<void*>(uiEncodeNonParamPayload(cfg.groupIdx)));
            lv_obj_add_event_cb(cymBtn, cfg.randomizeTunedCb, LV_EVENT_CLICKED, ui);
            uiAttachTooltip(cymBtn, "Randomize this group's filters and tuning");
        }
        if (cfg.randomizeCb != nullptr) {
        lv_obj_t* randBtn = lv_btn_create(header);
        lv_obj_set_size(randBtn, scaled(52), scaled(32));
        styles.applyMomentaryButton(randBtn);
        lv_obj_t* randDie = uiDiceIcon(randBtn, COL_TEXT);
        if (randDie != nullptr) lv_obj_center(randDie);
        lv_obj_set_user_data(randBtn,
                             reinterpret_cast<void*>(uiEncodeNonParamPayload(cfg.groupIdx)));
        lv_obj_add_event_cb(randBtn, cfg.randomizeCb, LV_EVENT_CLICKED, ui);
        uiAttachTooltip(randBtn, "Randomize this group (type, tuning and settings)");
        }
    }
};

END_NAMESPACE_DISTRHO

#endif // COMB_GROUP_HEADER_HPP
