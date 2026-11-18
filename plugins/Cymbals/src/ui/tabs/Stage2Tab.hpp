#ifndef STAGE2_TAB_HPP
#define STAGE2_TAB_HPP

#include "../UIWidgets.hpp"
#include "DispersionTabShared.hpp"
#include "DispersionStageSection.hpp"
#include <algorithm>
#include <cstdio>

START_NAMESPACE_DISTRHO

// STAGE 2: DISPERSION - single tab holding both dispersion stages behind a
// DISP 1 / DISP 2 sub-tab bar. Both stages render with the identical
// DispersionStageSection component (they run in parallel in the DSP; the
// global row exposes the shared skip/solo/cross-link bus). Sub-tab switching
// toggles CHECKED state + HIDDEN flags only - nothing is rebuilt or deleted,
// so the clicked button (the indev's active target) stays alive.
struct Stage2Tab {
    using NudgeBinding = DispersionStageSection::NudgeBinding;

    static void build(lv_obj_t* tab, AbstractCymbalsUI* ui, UIStyles& styles,
                      lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                      lv_event_cb_t valueCb, lv_event_cb_t rollerCallback,
                      const NudgeBinding& nudge1, const NudgeBinding& nudge2,
                      lv_event_cb_t randomizeNudgesCb,
                      lv_event_cb_t subTabCb, lv_obj_t** outSubTabBar,
                      lv_obj_t** outStageArea) {
        DispersionTabShared::configureTabRoot(tab);

        // =========================================================
        // Uniform screen strip: DISP 1 / DISP 2 split sub-tab buttons + the
        // shared bus controls (SOLO, Cross-link) on one line. No separate
        // global panel - the stage content starts right below.
        // =========================================================
        *outSubTabBar = UIWidgets::createScreenStrip(tab, styles, "DISPERSION");
        lv_obj_t* subTabBar = *outSubTabBar;
        // Keep the Cross-link ring clear of the window's right edge; without
        // this the knob's ring is clipped and reads as off-centre.
        lv_obj_set_style_pad_right(subTabBar, scaled(12), 0);
        // Layout recomputation after the tabs are measured can leave the old
        // label pixels behind (reported as a clipped "DIS" ghost inside the
        // DISP 1 zone). Invalidate the whole strip once layout settles.
        lv_obj_add_event_cb(subTabBar, [](lv_event_t* e) {
            lv_obj_invalidate(static_cast<lv_obj_t*>(lv_event_get_target(e)));
        }, LV_EVENT_LAYOUT_CHANGED, nullptr);
        lv_obj_t* stripSpacer = lv_obj_get_child(subTabBar, 1);  // strip's flex spacer

        // Audio flow readout: IN -> [DISP 1 | DISP 2] -> BUS -> OUT. The two
        // stages run in parallel and feed the shared dispersion bus; SOLO and
        // Cross-link live at the far right. user_data -1 keeps these labels out
        // of the sub-tab switcher (which filters children by user_data 0/1).
        auto flowLbl = [&](const char* text, lv_color_t color) {
            lv_obj_t* l = lv_label_create(subTabBar);
            lv_label_set_text(l, text);
            lv_obj_set_style_text_font(l, getScaledSmallFont(), 0);
            lv_obj_set_style_text_color(l, color, 0);
            lv_obj_set_user_data(l, (void*)(intptr_t)-1);
            return l;
        };
        flowLbl("IN", COL_TEXT_DIM);
        flowLbl(LV_SYMBOL_RIGHT, COL_ACCENT);

        for (int g = 0; g < 2; ++g) {
            const int skipParam = (g == 0) ? PluginCymbals::kParamDisp1Skip
                                           : PluginCymbals::kParamDisp2Skip;

            // SELECT button (right zone): switches the visible stage section.
            lv_obj_t* btn = lv_btn_create(subTabBar);
            lv_obj_set_size(btn, scaled(150), scaled(32));
            lv_obj_set_style_pad_all(btn, 0, 0);
            lv_obj_set_style_radius(btn, 0, 0);
            styles.applyMomentaryButton(btn);
            lv_obj_set_layout(btn, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_user_data(btn, reinterpret_cast<void*>((intptr_t)g));
            // Switch on PRESSED (fires the instant the mouse goes down - immune
            // to click-vs-drag classification), RELEASED, and CLICKED.
            lv_obj_add_event_cb(btn, subTabCb, LV_EVENT_PRESSED, ui);
            lv_obj_add_event_cb(btn, subTabCb, LV_EVENT_RELEASED, ui);
            lv_obj_add_event_cb(btn, subTabCb, LV_EVENT_CLICKED, ui);
            // Active sub-tab = LV_STATE_CHECKED (solid amber bg). btnMain's
            // vertical gradient would survive a bg_color override, so kill it.
            lv_obj_set_style_bg_color(btn, COL_HIGHLIGHT, LV_STATE_CHECKED);
            lv_obj_set_style_bg_grad_dir(btn, LV_GRAD_DIR_NONE, LV_STATE_CHECKED);
            if (g == 0) lv_obj_add_state(btn, LV_STATE_CHECKED);

            // LEFT ZONE (child button): stage on/off = the skip param.
            // Child hits land on the zone, not the select button.
            lv_obj_t* zone = lv_btn_create(btn);
            lv_obj_set_size(zone, scaled(26), scaled(32));
            lv_obj_set_style_radius(zone, 0, 0);
            lv_obj_set_style_pad_all(zone, 0, 0);
            lv_obj_add_flag(zone, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_user_data(zone, reinterpret_cast<void*>((intptr_t)skipParam));
            lv_obj_add_event_cb(zone, valueCb, LV_EVENT_VALUE_CHANGED, ui);
            // Repaint the whole strip after a zone toggle: the stage label's
            // pre-flex position can otherwise survive as a clipped ghost.
            lv_obj_add_event_cb(zone, [](lv_event_t* e) {
                lv_obj_invalidate(static_cast<lv_obj_t*>(lv_event_get_user_data(e)));
            }, LV_EVENT_CLICKED, subTabBar);
            widgets[skipParam] = zone;
            const bool isOn = fParams[skipParam] < 0.5f;  // skip is inverted
            styles.applyPowerZoneToggle(zone, isOn);
            // Quiet tab zone: the glyph carries the state, not a red/blue
            // block. The tab body supplies the selected background.
            lv_obj_set_style_bg_opa(zone, LV_OPA_TRANSP, 0);
            lv_obj_set_style_bg_opa(zone, LV_OPA_TRANSP, LV_STATE_CHECKED);
            lv_obj_set_style_bg_opa(zone, LV_OPA_TRANSP, LV_STATE_PRESSED);
            lv_obj_set_style_bg_opa(zone, LV_OPA_TRANSP,
                                    LV_STATE_CHECKED | LV_STATE_PRESSED);
            lv_obj_set_style_bg_grad_dir(zone, LV_GRAD_DIR_NONE, LV_STATE_CHECKED);
            lv_obj_set_style_border_width(zone, 0, 0);
            UIStyles::setToggleTick(zone, isOn, true);
            UIStyles::setPowerGlyphColor(zone, isOn ? (g == 0 ? COL_BG : COL_TEXT) : COL_ERROR);
            // Fixed geometry instead of flex: the label's pre-layout position
            // used to survive as a clipped "DIS" ghost inside the power zone.
            lv_obj_t* lbl = lv_label_create(btn);
            lv_label_set_text(lbl, g == 0 ? "DISP 1" : "DISP 2");
            lv_obj_set_style_text_font(lbl, getScaledSmallFont(), 0);
            lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
            // Centre the caption in the WHOLE button, not in the space left
            // after the power zone: excluding the zone shifted the text right
            // of the button's optical centre. The label is a transparent
            // overlay; the icon keeps its own hit area.
            lv_obj_add_flag(lbl, LV_OBJ_FLAG_IGNORE_LAYOUT);
            // Content-height label, centred in the button on BOTH axes. A
            // 32px-tall label draws its single text line at the top, which
            // left the caption ~13px above the power glyph's centre.
            lv_obj_set_size(lbl, scaled(150), LV_SIZE_CONTENT);
            lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
            lv_obj_set_style_text_color(lbl, g == 0 ? COL_BG : COL_TEXT, 0);
        }

        flowLbl(LV_SYMBOL_RIGHT, COL_ACCENT);
        lv_obj_t* busLbl = flowLbl("BUS", COL_HIGHLIGHT);
        lv_obj_set_style_bg_color(busLbl, COL_HIGHLIGHT, 0);
        lv_obj_set_style_bg_opa(busLbl, LV_OPA_20, 0);
        lv_obj_set_style_border_color(busLbl, COL_HIGHLIGHT, 0);
        lv_obj_set_style_border_width(busLbl, 1, 0);
        lv_obj_set_style_border_opa(busLbl, LV_OPA_40, 0);
        lv_obj_set_style_pad_hor(busLbl, scaled(4), 0);
        flowLbl(LV_SYMBOL_RIGHT, COL_ACCENT);
        flowLbl("OUT", COL_TEXT_DIM);

        // Push the strip's spacer past the flow group so SOLO + Cross-link
        // stay anchored to the right end.
        lv_obj_move_to_index(stripSpacer, lv_obj_get_child_cnt(subTabBar) - 1);

        {
            lv_obj_t* soloBtn = lv_btn_create(subTabBar);
            lv_obj_set_size(soloBtn, scaled(96), scaled(32));
            lv_obj_add_flag(soloBtn, LV_OBJ_FLAG_CHECKABLE);

            bool isSolo = fParams[PluginCymbals::kParamDispSolo] > 0.5f;
            lv_obj_t* lbl = lv_label_create(soloBtn);
            lv_label_set_text(lbl, "SOLO");
            lv_obj_center(lbl);

            styles.applyToggleButton(soloBtn, isSolo);

            lv_obj_set_user_data(soloBtn, reinterpret_cast<void*>((intptr_t)PluginCymbals::kParamDispSolo));
            lv_obj_add_event_cb(soloBtn, valueCb, LV_EVENT_VALUE_CHANGED, ui);
            widgets[PluginCymbals::kParamDispSolo] = soloBtn;

            // Nudge dice: randomize both nudge banks (counts and all values).
            lv_obj_t* rndBtn = lv_btn_create(subTabBar);
            lv_obj_set_size(rndBtn, scaled(44), scaled(32));
            styles.applyMomentaryButton(rndBtn);
            lv_obj_clear_state(rndBtn, LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(rndBtn, COL_CONTROL_FACE, 0);
            // Local payload -1: setActiveStage() only skips user_data 0/1, and
            // a null payload (0) made this dice inherit DISP 1's CHECKED/red
            // state whenever the sub-tab changed.
            lv_obj_set_user_data(rndBtn, (void*)(intptr_t)-1);
            lv_obj_t* die = uiDiceIcon(rndBtn, COL_TEXT);
            if (die != nullptr) lv_obj_center(die);
            lv_obj_add_event_cb(rndBtn, randomizeNudgesCb, LV_EVENT_CLICKED, ui);
            uiAttachTooltip(rndBtn, "Randomize both nudge engines (count and all settings)");
        }
        // The kit's smallWideTitleNoValueArcVisualSpec() sets containerH=52, but
        // createArcEx sizes the knob box to arcSize + ringPad(28) — a 30px arc
        // needs 58px, so the ring and cap were clipped by the container bottom
        // and the X-Link knob rendered as a cropped half-disc. Give the box an
        // honest height (one title line + arc + ring pad) for THIS control.
        // (This used to say the kit was an untracked shared checkout that
        // out-of-band edits would not survive. That is no longer true — the kit
        // is project-owned at plugins/Cymbals/src/cymbals-ui — but the override
        // stays because it is a per-call-site geometry choice for a strip
        // control, not a kit defect: the kit's own spec is right for a normal
        // knob column.)
        UIWidgets::ArcVisualSpec xlinkSpec = UIWidgets::smallWideTitleNoValueArcVisualSpec();
        // Natural content height: one title line + labelMarginBottom(1) + the
        // arcSize+ringPad(28) knob box. Sized to the real content so the strip
        // is as short as the knob allows — this strip sits above a stage with a
        // HARD vertical budget (the dispersion section needs 514px), and every
        // pixel the strip takes is a pixel the stage loses. The arc is kept
        // modest (20px) for the same reason: it still reads and drags, and the
        // ring that must be reserved around it is 28px regardless of arc size,
        // so arc size is the only free variable.
        xlinkSpec.arcSize = scaled(22);
        xlinkSpec.containerW = xlinkSpec.arcSize + scaled(28);
        // No title line (createArcEx skips an empty name): the shared 56px
        // strip fits the ring+cap, centred against the X-LINK caption.
        xlinkSpec.containerH = xlinkSpec.arcSize + scaled(28);
        lv_obj_t* xlinkLbl = lv_label_create(subTabBar);
        lv_label_set_text(xlinkLbl, "X-LINK");
        lv_obj_set_style_text_font(xlinkLbl, uiMicroFont(), 0);
        lv_obj_set_style_text_color(xlinkLbl, COL_TEXT_DIM, 0);
        lv_obj_set_user_data(xlinkLbl, (void*)(intptr_t)-1);
        UIWidgets::createArcEx(subTabBar,
                       PluginCymbals::kParamDispCrossLink,
                       "",
                       ui,
                       styles,
                       widgets,
                       labels,
                       fParams,
                       valueCb,
                       xlinkSpec,
                       false);
        if (widgets[PluginCymbals::kParamDispCrossLink]) {
            lv_obj_set_user_data(widgets[PluginCymbals::kParamDispCrossLink], (void*)(intptr_t)-1);
            if (lv_obj_t* c = lv_obj_get_parent(widgets[PluginCymbals::kParamDispCrossLink]))
                lv_obj_set_user_data(c, (void*)(intptr_t)-1);
        }
        uiAttachTooltip(widgets[PluginCymbals::kParamDispCrossLink], "X-Link (dispersion cross-link)");

        // =========================================================
        // Stage area: both sections built once, one visible at a time
        // =========================================================
        lv_obj_t* stageArea = lv_obj_create(tab);
        lv_obj_set_size(stageArea, lv_pct(100), 0);
        lv_obj_set_flex_grow(stageArea, 1);
        lv_obj_set_style_bg_opa(stageArea, 0, 0);
        lv_obj_set_style_border_width(stageArea, 0, 0);
        lv_obj_set_style_pad_all(stageArea, 0, 0);
        lv_obj_set_layout(stageArea, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(stageArea, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(stageArea, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        // USER RULE: nothing ever scrolls. Both dispersion stages are
        // compressed to fit the stage area (see DispersionStageSection.hpp);
        // the audit harness proves the fit at gUIScale=1.0.
        uiLockScroll(stageArea);
        *outStageArea = stageArea;

        lv_obj_t* section1 = DispersionStageSection::build(
            stageArea, DispersionStageSection::specStage1(), ui, styles, widgets, labels,
            fParams, valueCb, rollerCallback, nudge1);
        lv_obj_set_user_data(section1, (void*)0);
        lv_obj_t* section2 = DispersionStageSection::build(
            stageArea, DispersionStageSection::specStage2(), ui, styles, widgets, labels,
            fParams, valueCb, rollerCallback, nudge2);
        lv_obj_set_user_data(section2, (void*)1);
        lv_obj_add_flag(section2, LV_OBJ_FLAG_HIDDEN);
        refreshZoneGlyphs(subTabBar);
    }

    static void refreshZoneGlyphs(lv_obj_t* subTabBar) {
        if (subTabBar == nullptr) return;
        const uint32_t nb = lv_obj_get_child_cnt(subTabBar);
        for (uint32_t i = 0; i < nb; ++i) {
            lv_obj_t* btn = lv_obj_get_child(subTabBar, i);
            const int g = (int)(intptr_t)lv_obj_get_user_data(btn);
            if (g != 0 && g != 1) continue;
            lv_obj_t* zone = lv_obj_get_child(btn, 0);
            if (zone == nullptr) continue;
            const bool on = lv_obj_has_state(zone, LV_STATE_CHECKED);
            const bool active = lv_obj_has_state(btn, LV_STATE_CHECKED);
            // The glyph alone carries the state; no plate or border, so an
            // active tab reads as one continuous cyan bar.
            lv_obj_set_style_bg_opa(zone, LV_OPA_TRANSP, 0);
            lv_obj_set_style_bg_opa(zone, LV_OPA_TRANSP, LV_STATE_CHECKED);
            lv_obj_set_style_border_width(zone, 0, 0);
            UIStyles::setPowerGlyphColor(zone,
                !on ? COL_ERROR : (active ? COL_BG : COL_TEXT));
        }
    }

    // Toggles sub-tab button states + section visibility. No deletion: called
    // from the sub-tab click (the clicked button must survive the event).
    static void setActiveStage(lv_obj_t* subTabBar, lv_obj_t* stageArea, int active) {
        if (!subTabBar || !stageArea) return;
        const uint32_t nb = lv_obj_get_child_cnt(subTabBar);
        for (uint32_t i = 0; i < nb; ++i) {
            lv_obj_t* btn = lv_obj_get_child(subTabBar, i);
            const int g = (int)(intptr_t)lv_obj_get_user_data(btn);
            if (g != 0 && g != 1) continue;  // spacer / SOLO / cross-link
            const bool isActive = (g == active);
            if (isActive) lv_obj_add_state(btn, LV_STATE_CHECKED);
            else          lv_obj_clear_state(btn, LV_STATE_CHECKED);
            lv_obj_t* lbl = lv_obj_get_child(btn, 1);  // child 0 = on/off zone
            if (lbl && lv_obj_check_type(lbl, &lv_label_class)) {
                lv_obj_set_style_text_color(lbl, isActive ? COL_BG : COL_TEXT, 0);
            }
        }
        refreshZoneGlyphs(subTabBar);
        const uint32_t ns = lv_obj_get_child_cnt(stageArea);
        for (uint32_t i = 0; i < ns; ++i) {
            lv_obj_t* section = lv_obj_get_child(stageArea, i);
            const int g = (int)(intptr_t)lv_obj_get_user_data(section);
            if (g == active) lv_obj_clear_flag(section, LV_OBJ_FLAG_HIDDEN);
            else             lv_obj_add_flag(section, LV_OBJ_FLAG_HIDDEN);
        }
    }
};

END_NAMESPACE_DISTRHO

#endif // STAGE2_TAB_HPP
