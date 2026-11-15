#ifndef STAGE3_TAB_HPP
#define STAGE3_TAB_HPP

#include "../UIWidgets.hpp"
#include "../UIXYPad.hpp"
#include "../CombEditorSelectionController.hpp"
#include "../CombEditorDock.hpp"
#include "../CombGroupHeader.hpp"
#include <string>

START_NAMESPACE_DISTRHO

struct Stage3Tab {
    // GROUP on/off dimming. A disabled group must read as disabled on every
    // surface (reported: switching filter screens showed disabled groups still
    // fully colourful).
    //
    // Takes the live containers and re-queries their children. Deliberately no
    // cached pointers: lv_obj_is_valid is compiled out in this build
    // (LV_USE_CHECK_OBJ_VALIDITY=0), so a stale entry after a populate rebuild
    // would be indistinguishable from a live one.
    static void refreshGroupDimming(const float* fParams, lv_obj_t* subTabBar, lv_obj_t* stage,
                                    int activeGroup = -1) {
        if (fParams == nullptr || subTabBar == nullptr || stage == nullptr) return;
        const uint32_t tabCount = lv_obj_get_child_count(subTabBar);
        const uint32_t cardCount = lv_obj_get_child_count(stage);
        const bool bankOn = fParams[PluginCymbals::kParamCombBankOn] > 0.5f;
        for (uint32_t g = 0; g < tabCount && g < 10; ++g) {
            const bool groupOn = fParams[PluginCymbals::kParamCombGroup1On + g] > 0.5f;
            const bool on = bankOn && groupOn;
            lv_obj_t* tab = lv_obj_get_child(subTabBar, g);
            if (tab != nullptr) {
                // No LV_STATE_CHECKED here: the shared checked button style
                // is ADDED and forces text_color=COL_BG, which paints the
                // caption black on a dark tab. Group selection is fActiveCombGroup;
                // paint the plate explicitly instead (cyan only when active + on).
                const bool selected = (activeGroup >= 0 && (int)g == activeGroup);
                const bool activeOn = selected && on;
                lv_obj_set_style_bg_color(tab,
                    activeOn ? COL_HIGHLIGHT : (selected ? COL_PANEL_DARK : COL_PANEL), 0);
                lv_obj_set_style_bg_opa(tab, LV_OPA_COVER, 0);
                lv_obj_set_style_bg_grad_dir(tab, LV_GRAD_DIR_NONE, 0);
                // Dim the tab's NAME and X, never the ON/OFF pill: that zone is
                // child 0 and is the only way to re-enable the group, so it
                // must stay fully legible/clickable.
                const uint32_t n = lv_obj_get_child_count(tab);
                for (uint32_t c = 1; c < n; ++c) {
                    lv_obj_t* child = lv_obj_get_child(tab, c);
                    if (child == nullptr) continue;
                    // The GROUP name lives in a plain wrapper (child 1), not
                    // directly on the checkable tab. LVGL's ADDED checked
                    // button style carries text_color=COL_BG and wins over a
                    // child label's local colour, which rendered OFF groups
                    // black-on-black. Give the wrapper its own local colour and
                    // dim only the X zone; the name stays legible.
                    const bool isName = (c == 1);
                    lv_obj_set_style_opa(child,
                        (on || isName) ? LV_OPA_COVER : LV_OPA_50, 0);
                    if (isName) {
                        lv_obj_set_style_text_color(child,
                            ((on && selected) ? COL_BG : COL_TEXT), 0);
                    }
                }
                // The zone mirrors the effective state: when BANKS is off the
                // whole row reads off (red + visible power glyph), while the
                // per-group parameter itself is left untouched.
                if (lv_obj_t* zone = lv_obj_get_child(tab, 0)) {
                    if (on) lv_obj_add_state(zone, LV_STATE_CHECKED);
                    else    lv_obj_clear_state(zone, LV_STATE_CHECKED);
                    lv_obj_set_style_bg_color(zone, on ? COL_HIGHLIGHT : COL_ERROR, 0);
                    lv_obj_set_style_bg_opa(zone, LV_OPA_COVER, 0);
                    lv_obj_set_style_bg_grad_dir(zone, LV_GRAD_DIR_NONE, 0);
                    UIStyles::setToggleTick(zone, on);
                }
            }
            if (g < cardCount) {
                lv_obj_t* card = lv_obj_get_child(stage, g);
                if (card != nullptr)
                    lv_obj_set_style_opa(card, on ? LV_OPA_COVER : LV_OPA_50, 0);
            }
        }
    }

    struct CombEditorState {
        // Ownership: heap-allocated with 'new' during buildCombEditor(),
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
        // does NOT hand them back — the whole card subtree, including every
        // saved owner, is being deleted, so it nulls the slots instead.
        lv_obj_t* savedWidgets[24] = {};
        lv_obj_t* xyPad = nullptr;          // Ownership: parent (LVGL)
        lv_obj_t* selectionButtons[4] = {nullptr, nullptr, nullptr, nullptr};
        lv_obj_t* panel = nullptr;          // Ownership: top layer when panelDetached
        bool panelDetached = false;         // dock re-parented to the display top layer
        lv_obj_t* titleLabel = nullptr;     // Ownership: parent (LVGL)
        lv_obj_t* arcFreq = nullptr;        // Ownership: parent (LVGL)
        lv_obj_t* arcFB = nullptr;          // the comb's feedback (live DSP param)
        lv_obj_t* arcQ = nullptr;           // per-filter Q (AUTO at the bottom)
        lv_obj_t* arcLPF = nullptr;         // Ownership: parent (LVGL)
        lv_obj_t* arcHPF = nullptr;         // Ownership: parent (LVGL)
        lv_obj_t* arcScale = nullptr;       // Ownership: parent (LVGL)
        lv_obj_t* arcMix = nullptr;         // Ownership: parent (LVGL)
        lv_obj_t* arcSatAmt = nullptr;      // Ownership: parent (LVGL)
        lv_obj_t* arcStressOnset = nullptr; // Ownership: parent (LVGL)
        lv_obj_t* arcStressKnee = nullptr;  // Ownership: parent (LVGL)
        lv_obj_t* arcModeRate = nullptr;    // Ownership: parent (LVGL)
        lv_obj_t* arcModeA = nullptr;       // per-filter-type control A
        lv_obj_t* arcModeB = nullptr;       // per-filter-type control B
        lv_obj_t* arcModeAmount = nullptr;  // Ownership: parent (LVGL)
        lv_obj_t* arcPartRelPos = nullptr;  // Saike model controls (partials types only)
        lv_obj_t* arcPartFreqDamp = nullptr;
        lv_obj_t* arcPartStiffness = nullptr;
        lv_obj_t* arcPartStiffExp = nullptr;
        lv_obj_t* arcPartCount = nullptr;
        lv_obj_t* phaseToggleButton = nullptr; // Ownership: parent (LVGL)
        lv_obj_t* soloButton = nullptr;     // Ownership: parent (LVGL)
        lv_obj_t* enableButton = nullptr;   // Ownership: parent (LVGL)
        lv_obj_t* enableLabel = nullptr;    // Ownership: parent (LVGL)
        lv_obj_t* satTypeDropdown = nullptr;// Ownership: parent (LVGL)
        lv_obj_t* filterTypeDropdown = nullptr;// Ownership: parent (LVGL)
    };

    static int stage3FreqParam(int comb) { return PluginCymbals::kParamComb1Freq + comb * 4; }
    static int stage3QParam(int comb) { return PluginCymbals::kParamComb1Q + comb; }
    static int stage3FBParam(int comb) { return PluginCymbals::kParamComb1Freq + comb * 4 + 1; }
    static int stage3LPFParam(int comb) { return PluginCymbals::kParamComb1Freq + comb * 4 + 2; }
    static int stage3HPFParam(int comb) { return PluginCymbals::kParamComb1Freq + comb * 4 + 3; }
    static int stage3ScaleParam(int comb) { return PluginCymbals::kParamComb1Scale + comb; }
    static int stage3MixParam(int comb) { return PluginCymbals::kParamComb1Mix + comb; }
    static int stage3SatAmountParam(int comb) { return PluginCymbals::kParamComb1SatAmount + comb; }
    static int stage3StressOnsetParam(int comb) { return PluginCymbals::kParamComb1StressOnset + comb; }
    static int stage3StressKneeParam(int comb) { return PluginCymbals::kParamComb1StressKnee + comb; }
    static int stage3SatTypeParam(int comb) { return PluginCymbals::kParamComb1SatType + comb; }
    static int stage3FilterTypeParam(int comb) { return PluginCymbals::kParamComb1FilterType + comb; }
    static int stage3ModeRateParam(int comb) { return PluginCymbals::kParamComb1ModeRate + comb; }
    static int stage3ModeAmountParam(int comb) { return PluginCymbals::kParamComb1ModeAmount + comb; }
    static int stage3ModeAParam(int comb) { return PluginCymbals::kParamComb1ModeA + comb; }
    static int stage3ModeBParam(int comb) { return PluginCymbals::kParamComb1ModeB + comb; }
    static int stage3PartRelPosParam(int comb) { return PluginCymbals::kParamComb1PartRelPos + comb; }
    static int stage3PartFreqDampParam(int comb) { return PluginCymbals::kParamComb1PartFreqDamp + comb; }
    static int stage3PartStiffnessParam(int comb) { return PluginCymbals::kParamComb1PartStiffness + comb; }
    static int stage3PartStiffExpParam(int comb) { return PluginCymbals::kParamComb1PartStiffExp + comb; }
    static int stage3PartCountParam(int comb) { return PluginCymbals::kParamComb1PartCount + comb; }
    static int stage3SoloParam(int comb) { return PluginCymbals::kParamComb1Solo + comb; }
    static int stage3EnableParam(int comb) { return PluginCymbals::kParamComb1Enable + comb; }

    static const CombEditorDock::BehaviorParamMap& stage3BehaviorMap() {
        static const CombEditorDock::BehaviorParamMap kMap = {
            40,
            40,
            stage3FreqParam,
            stage3FBParam,
            stage3QParam,
            stage3LPFParam,
            stage3HPFParam,
            stage3ScaleParam,
            stage3MixParam,
            stage3SatAmountParam,
            stage3StressOnsetParam,
            stage3StressKneeParam,
            stage3SatTypeParam,
            stage3FilterTypeParam,
            stage3SoloParam,
            stage3EnableParam,
            stage3ModeRateParam,
            stage3ModeAmountParam,
            stage3PartRelPosParam,
            stage3PartFreqDampParam,
            stage3PartStiffnessParam,
            stage3PartStiffExpParam,
            stage3PartCountParam,
            stage3ModeAParam,
            stage3ModeBParam,
        };
        return kMap;
    }

    // LVGL delivers a pointer press to the topmost CLICKABLE object and then
    // walks up the parent chain, but only THROUGH objects carrying
    // LV_OBJ_FLAG_EVENT_BUBBLE. Neither the group card's children nor the
    // GROUP sub-tab buttons carry it, so the card-level
    // clearSelectionOutsideXyCb (and the tab-level equivalent below) only ever
    // saw presses that landed on a container itself — never on a knob, a tune
    // button or a group tab. Enabling bubbling on the FILTERS tab's clickable
    // objects is what makes "press anywhere outside the dock" actually reach
    // the dock. Arcs are exempt by design: both the kit's arcDragCb and the
    // dock's own arcDragCb call lv_event_stop_bubbling on LV_EVENT_PRESSED, so
    // turning a knob never closes the dock.
    static void enablePressBubbleRecursive(lv_obj_t* root) {
        if (!root) return;
        if (lv_obj_is_clickable(root)) lv_obj_add_flag(root, LV_OBJ_FLAG_EVENT_BUBBLE);
        const uint32_t n = lv_obj_get_child_count(root);
        for (uint32_t i = 0; i < n; ++i) enablePressBubbleRecursive(lv_obj_get_child(root, i));
    }

    static bool isDescendantOf(lv_obj_t* obj, lv_obj_t* ancestor) {
        while (obj != nullptr) {
            if (obj == ancestor) return true;
            obj = lv_obj_get_parent(obj);
        }
        return false;
    }

    static void clearSelectionOutsideXyCb(lv_event_t* e) {
        auto* st = static_cast<CombEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->xyPad) return;
        lv_obj_t* target = static_cast<lv_obj_t*>(lv_event_get_target(e));
        if (isDescendantOf(target, st->xyPad)) return;
        if (st->panel && isDescendantOf(target, st->panel)) return;
        UIXYPad::clearSelection(st->xyPad);
    }

    // FIX: the FILTER dock used to stay open unless the pad selection was
    // cleared from inside the group card, so a press on the GROUP sub-tab bar,
    // the tab background or another group's header left it floating. This is
    // the same bubbling-press pattern as clearSelectionOutsideXyCb, but on the
    // whole tab and driven off the xyPads[] array, so a SINGLE registration
    // covers every group and can never go stale when populateCombGroups
    // rebuilds the cards (a per-card registration on the tab would leak one
    // dead callback per group per rebuild). A screen-wide scrim is
    // deliberately NOT used — it would swallow XY-pad presses and the pad has
    // to stay live.
    static void closeSelectionOnOutsidePressCb(lv_event_t* e) {
        lv_obj_t* target = static_cast<lv_obj_t*>(lv_event_get_target(e));
        auto* const pads = static_cast<lv_obj_t**>(lv_event_get_user_data(e));
        if (!pads) return;
        for (int g = 0; g < 10; ++g) {
            lv_obj_t* pad = pads[g];
            if (!pad) continue;
            auto* pst = static_cast<UIXYPad::State*>(lv_obj_get_user_data(pad));
            if (!pst) continue;
            // The pad handles its own clicks (select / lasso / deselect), and a
            // press inside a dock must not close that dock — knob drags,
            // dropdowns and the header drag would all close it.
            if (isDescendantOf(target, pad)) continue;
            if (pst->dockPanel && isDescendantOf(target, pst->dockPanel)) continue;
            UIXYPad::clearSelection(pad);
        }
    }

    static void combCircleTouched(void* userData, int groupIdx, int localCombIdx) {
        auto* st = static_cast<CombEditorState*>(userData);
        if (!st) return;
        st->selectedComb = groupIdx * 4 + std::clamp(localCombIdx, 0, 3);
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
        updateSelectionTitle(st);
        refreshCombEditor(st);
        syncSelectionButtons(st);
    }
    
    // OWNERSHIP CLEANUP: Delete heap-allocated state when LVGL destroys object
    static void deleteStateCb(lv_event_t* e) {
        auto* st = static_cast<CombEditorState*>(lv_event_get_user_data(e));
        // Fix #1 (Round 2): registered on the CARD (LV_EVENT_DELETE below),
        // this runs WHILE populateCombGroups' lv_obj_clean(stage) tears the
        // subtree down. Restoring savedWidgets[] owners here (the old Fix B
        // behavior) planted DYING pointers into widgets[]: every possible
        // owner dies in the same deletion — this card's children are freed
        // right after this event, and subTabBar controls were already freed
        // by lv_obj_clean(subTabBar) — and lv_obj_is_valid cannot detect an
        // owner queued to die in this very pass (the restore kept it). The
        // next full-array walk (preset load PluginUI.cpp:1037-1042, host
        // automation :773) then writes through the freed object past the
        // null-only guards (updateLabel :1967, syncParamWidget :354).
        // Null the slots instead; restore stays in the rebind/dock-close
        // paths (CombEditorDock.hpp:252 / :441), which never run during a
        // deletion.
        if (st) {
            // The dock lives on the display top layer now: the card no longer
            // owns it, so delete it explicitly or it outlives its State.
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

    static int getTargetMask(CombEditorState* st) {
        return CombEditorDock::getTargetMask(st);
    }

    static void updateSelectionTitle(CombEditorState* st) {
        CombEditorDock::updateSelectionTitle(st);
    }

    static void refreshCombEditor(CombEditorState* st) {
        CombEditorDock::refreshEditorFromSelection(st, stage3BehaviorMap());
    }

    static void combEditorSelectCb(lv_event_t* e) {
        auto* st = static_cast<CombEditorState*>(lv_event_get_user_data(e));
        if (!st) return;
        st->selectedComb = (int)lv_dropdown_get_selected(static_cast<lv_obj_t*>(lv_event_get_target(e)));
        refreshCombEditor(st);
    }

    static void combEditorArcSetCb(lv_event_t* e) {
        auto* st = static_cast<CombEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->ui) return;
        // Programmatic sync (syncParamWidget during an XY-pad drag) re-fires
        // this callback. Only respond to real user edits on the arc itself.
        if (st->ui->isInSync()) return;
        lv_obj_t* arc = static_cast<lv_obj_t*>(lv_event_get_target(e));
        const auto wIt = CombEditorDock::arcWhichMap().find(arc);
        const int which = (wIt != CombEditorDock::arcWhichMap().end()) ? wIt->second : -1;
        if (which < 0) return;
        const float v = (float)lv_arc_get_value(arc) / 1000.0f;
        CombEditorDock::applyArcToSelection(st, stage3BehaviorMap(), arc, which, v);
    }

    static void combEditorSoloCb(lv_event_t* e) {
        auto* st = static_cast<CombEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->ui) return;
        const float v = lv_obj_has_state(static_cast<lv_obj_t*>(lv_event_get_target(e)), LV_STATE_CHECKED) ? 1.0f : 0.0f;
        CombEditorDock::applySoloToSelection(st, stage3BehaviorMap(), v);
    }

    static void combEditorEnableCb(lv_event_t* e) {
        auto* st = static_cast<CombEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->ui) return;
        lv_obj_t* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
        const float v = lv_obj_has_state(btn, LV_STATE_CHECKED) ? 1.0f : 0.0f;
        CombEditorDock::applyEnableToSelection(st, stage3BehaviorMap(), v);
    }

    static void combEditorPhaseCb(lv_event_t* e) {
        auto* st = static_cast<CombEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->ui || !st->xyPad) return;
        lv_obj_t* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
        UIXYPad::setPhaseView(st->xyPad, lv_obj_has_state(btn, LV_STATE_CHECKED));
    }

    static void combEditorSatTypeCb(lv_event_t* e) {
        auto* st = static_cast<CombEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->ui) return;
        const int t = (int)lv_dropdown_get_selected(static_cast<lv_obj_t*>(lv_event_get_target(e)));
        CombEditorDock::applySatTypeToSelection(st, stage3BehaviorMap(), t);
    }

    static void combEditorFilterTypeCb(lv_event_t* e) {
        auto* st = static_cast<CombEditorState*>(lv_event_get_user_data(e));
        if (!st || !st->ui) return;
        const int t = (int)lv_dropdown_get_selected(static_cast<lv_obj_t*>(lv_event_get_target(e)));
        CombEditorDock::applyFilterTypeToSelection(st, stage3BehaviorMap(), t);
    }

    static void syncEditorFromSelection(CombEditorState* st) {
        CombEditorDock::syncEditorFromXySelection(st, stage3BehaviorMap());
    }

    // The 1-4 strip under the group header selects the filter the dock edits,
    // mirroring the EQ band strip. `selectedComb` is the last circle touched;
    // the XY mask also clears the strip when the pad selection is dismissed.
    static void syncSelectionButtons(CombEditorState* st) {
        if (!st) return;
        // Mirror the FULL pad selection, not just the last circle touched: a
        // lasso can select filters 1+2+3+4 together, and the numbered strip has
        // to show all of them (the dock's title already does). selectedComb
        // stays the edit focus for the dock's single value readouts.
        const int mask = UIXYPad::getSelectedMask(st->xyPad);
        for (int i = 0; i < 4; ++i) {
            lv_obj_t* b = st->selectionButtons[i];
            if (!b) continue;
            if (mask & (1 << i)) lv_obj_add_state(b, LV_STATE_CHECKED);
            else                 lv_obj_clear_state(b, LV_STATE_CHECKED);
        }
    }

    static void selectionButtonCb(lv_event_t* e) {
        auto* st = static_cast<CombEditorState*>(lv_event_get_user_data(e));
        lv_obj_t* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
        if (!st || !btn || !st->xyPad) return;
        const int idx = uiDecodeNonParamPayload((intptr_t)lv_obj_get_user_data(btn));
        UIXYPad::selectComb(st->xyPad, idx);
        // Toggle-off: keep the dock focused on a filter that is still selected
        // (or close it when the mask empties). selectComb only fires the
        // circle-touched callback when ADDING, so the focus move lives here.
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
        auto* st = static_cast<CombEditorState*>(lv_event_get_user_data(e));
        if (!st) return;
        syncEditorFromSelection(st);
        syncSelectionButtons(st);
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

    static void build(lv_obj_t* tab, AbstractCymbalsUI* ui, UIStyles& styles,
                     lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                     lv_event_cb_t valueCb, lv_obj_t** outCombList, lv_obj_t** outSubTabBar,
                     int nVisible,
                     lv_event_cb_t addCombCb, lv_event_cb_t removeCombCb,
                     lv_event_cb_t randomizeGroupCb, lv_event_cb_t randomizeAllCb,
                     lv_event_cb_t tuningCb, lv_event_cb_t subTabCb,
                     lv_obj_t** xyPads,
                     lv_event_cb_t groupLPFHPFCb,
                     lv_event_cb_t randomizeCountCb = nullptr,
                     lv_event_cb_t randomizeGroupTunedCb = nullptr) {

        // Tab: COLUMN layout, no scrolling - one group visible at a time
        // behind a GROUP sub-tab bar.
        lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(tab, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_layout(tab, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(tab, scaled(10), 0);
        lv_obj_set_style_pad_row(tab, scaled(10), 0);
        // --- ONE top row: title, ON, +, RANDOM, then the browser-style
        // GROUP tabs (ON/OFF | name | X) populated by populateCombGroups. ---
        lv_obj_t* header = lv_obj_create(tab);
        lv_obj_set_size(header, lv_pct(100), scaled(56));
        lv_obj_set_style_min_height(header, scaled(56), 0);
        lv_obj_set_style_bg_opa(header, 0, 0);
        lv_obj_set_style_border_width(header, 0, 0);
        lv_obj_set_style_pad_all(header, 0, 0);
        lv_obj_set_style_pad_left(header, scaled(10), 0);
        // Reserve the right edge for the dice so tab wrapping never pushes it
        // onto a second row (matches the DISPERSION strip's right-side dice).
        // Two right-aligned random buttons: full random dice, and the
        // structural cymbal (count only).
        lv_obj_set_style_pad_right(header, scaled(96), 0);
        lv_obj_set_layout(header, LV_LAYOUT_FLEX);
        // Single row: group tabs shrink once there are five or more groups so
        // the six-tab maximum + the right-aligned dice always fit one line.
        lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(header, scaled(4), 0);
        lv_obj_set_style_pad_column(header, scaled(8), 0);
        lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* sTitle = lv_label_create(header);
        lv_label_set_text(sTitle, "FILTERS");
        lv_obj_add_style(sTitle, &styles.title, 0);
        lv_obj_set_user_data(sTitle, (void*)(intptr_t)-1);

        lv_obj_t* hTitle = lv_label_create(header);
        lv_label_set_text(hTitle, "DYNAMIC FILTER BANK");
        lv_obj_add_style(hTitle, &styles.title, 0);

        lv_obj_t* onBt = lv_btn_create(header);
        lv_obj_set_size(onBt, scaled(68), scaled(32));  // "BANK" at the 14px face needs 37px inside the pads
        lv_obj_add_flag(onBt, LV_OBJ_FLAG_CHECKABLE);
        int onParam = PluginCymbals::kParamCombBankOn;
        bool isOn = fParams[onParam] > 0.5f;
        lv_obj_t* onLbl = lv_label_create(onBt);
        // Colour carries the state; "BANK" names the scope so the bare
        // OFF/ON glyph no longer reads like the per-stage filter switch.
        lv_label_set_text(onLbl, "BANKS");
        lv_obj_set_style_text_font(onLbl, getScaledFont(), 0);
        lv_obj_center(onLbl);
        styles.applyToggleButton(onBt, isOn);
        lv_obj_set_user_data(onBt, reinterpret_cast<void*>((intptr_t)onParam));
        lv_obj_add_event_cb(onBt, valueCb, LV_EVENT_VALUE_CHANGED, ui);
        uiAttachTooltip(onBt, "Whole filter bank bypass. Off disables every group; per-filter ON stays on the dock.");
        widgets[onParam] = onBt;
        labels[onParam] = onLbl;

        lv_obj_t* addBtn = lv_btn_create(header);
        lv_obj_set_size(addBtn, scaled(40), scaled(32));
        styles.applyMomentaryButton(addBtn);
        lv_obj_t* addLbl = lv_label_create(addBtn);
        lv_label_set_text(addLbl, "+");
        lv_obj_set_style_text_font(addLbl, getScaledFont(), 0);
        lv_obj_center(addLbl);
        lv_obj_add_event_cb(addBtn, addCombCb, LV_EVENT_CLICKED, ui);

        // GROUP tab container (cleaned by populateCombGroups on rebuild).
        lv_obj_t* btnArea = lv_obj_create(header);
        // SIZE_CONTENT, not pct(100): the header is SIZE_CONTENT too, and with
        // six tabs the percentage resolved against a wrapping row and collapsed
        // the group card area (reported: adding the 6th group hid them all).
        lv_obj_set_size(btnArea, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(btnArea, 0, 0);
        lv_obj_set_style_border_width(btnArea, 0, 0);
        lv_obj_set_style_pad_all(btnArea, 0, 0);
        lv_obj_set_layout(btnArea, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(btnArea, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(btnArea, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(btnArea, scaled(6), 0);
        lv_obj_clear_flag(btnArea, LV_OBJ_FLAG_SCROLLABLE);
        *outSubTabBar = btnArea;

        lv_obj_t* randAllBtn = lv_btn_create(header);
        lv_obj_set_size(randAllBtn, scaled(28), scaled(28));
        lv_obj_add_flag(randAllBtn, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_align(randAllBtn, LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_set_user_data(randAllBtn, (void*)(intptr_t)-1);
        styles.applyMomentaryButton(randAllBtn);
        lv_obj_clear_state(randAllBtn, LV_STATE_CHECKED);
        lv_obj_set_style_bg_color(randAllBtn, lv_color_hex(0x36393E), 0);
        lv_obj_t* randAllDie = uiDiceIcon(randAllBtn, COL_TEXT);
        if (randAllDie != nullptr) lv_obj_center(randAllDie);
        uiAttachTooltip(randAllBtn, "Randomize group count and every filter in the bank");
        lv_obj_add_event_cb(randAllBtn, randomizeAllCb, LV_EVENT_CLICKED, ui);

        if (randomizeCountCb != nullptr) {
            lv_obj_t* randCountBtn = lv_btn_create(header);
            lv_obj_set_size(randCountBtn, scaled(28), scaled(28));
            lv_obj_add_flag(randCountBtn, LV_OBJ_FLAG_IGNORE_LAYOUT);
            lv_obj_align(randCountBtn, LV_ALIGN_RIGHT_MID, -scaled(36), 0);
            lv_obj_set_user_data(randCountBtn, (void*)(intptr_t)-1);
            styles.applyMomentaryButton(randCountBtn);
            lv_obj_clear_state(randCountBtn, LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(randCountBtn, lv_color_hex(0x36393E), 0);
            lv_obj_t* cym = uiCymbalIcon(randCountBtn, COL_TEXT);
            if (cym != nullptr) lv_obj_center(cym);
            uiAttachTooltip(randCountBtn,
                "Randomize only the number of comb filters (adds/removes groups; keeps settings and power)");
            lv_obj_add_event_cb(randCountBtn, randomizeCountCb, LV_EVENT_CLICKED, ui);
        }

        // --- Single visible group card ---
        // Flex COLUMN (same as the DISP stage area): without an explicit
        // layout the card was positioned with a bogus offset and its children
        // rendered detached above it.
        lv_obj_t* stage = lv_obj_create(tab);
        lv_obj_set_size(stage, lv_pct(100), 0);
        lv_obj_set_flex_grow(stage, 1);
        lv_obj_set_style_min_height(stage, scaled(120), 0);
        lv_obj_set_style_bg_opa(stage, 0, 0);
        lv_obj_set_style_border_width(stage, 0, 0);
        lv_obj_set_style_pad_all(stage, 0, 0);
        lv_obj_set_layout(stage, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(stage, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(stage, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_clear_flag(stage, LV_OBJ_FLAG_SCROLLABLE);
        *outCombList = stage;

        // Anywhere-outside-the-dock press closes the dock. Registered on the
        // TAB (not the card, which only sees presses that bubble through it)
        // so the GROUP sub-tab bar and the tab background close it too. The
        // callback reads xyPads[] at event time, so it is registered before
        // populateCombGroups fills the array.
        lv_obj_add_flag(tab, LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_obj_add_event_cb(tab, closeSelectionOnOutsidePressCb, LV_EVENT_PRESSED, xyPads);

        populateCombGroups(btnArea, stage, ui, styles, widgets, labels, fParams, valueCb,
                           nVisible, removeCombCb, randomizeGroupCb, tuningCb, subTabCb,
                           xyPads, groupLPFHPFCb, 0, randomizeGroupTunedCb);

        // The DSP bank always contains ten group slots. Only nVisible/4 groups
        // are represented on screen; any hidden slot that a preset or host
        // state left enabled would keep filtering while every visible group is
        // off. Force those tails off so the visible group count is the real
        // signal path.
        for (int i = nVisible; i < 40; ++i) {
            const uint32_t p = (uint32_t)(PluginCymbals::kParamComb1Enable + i);
            if (fParams[p] > 0.5f) {
                ui->editParameter(p, true);
                ui->setParamValue(p, 0.0f);
                ui->editParameter(p, false);
            }
        }

        // Presses on the tab's own header row (title / ON / + / RANDOM / the
        // GROUP tabs container) must bubble too — they are siblings of `stage`,
        // which populateCombGroups already swept.
        enablePressBubbleRecursive(tab);
    }

    // Builds the GROUP sub-tab buttons and all group cards, showing only the
    // active group. Called at build time and by rebuildCombList.
    static void populateCombGroups(lv_obj_t* subTabBar, lv_obj_t* stage,
                                   AbstractCymbalsUI* ui, UIStyles& styles,
                                   lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                                   lv_event_cb_t valueCb, int nVisible,
                                   lv_event_cb_t removeCombCb, lv_event_cb_t randomizeGroupCb,
                                   lv_event_cb_t tuningCb, lv_event_cb_t subTabCb,
                                   lv_obj_t** xyPads, lv_event_cb_t groupLPFHPFCb,
                                   int activeGroup,
                                   lv_event_cb_t randomizeGroupTunedCb = nullptr) {
        lv_obj_clean(subTabBar);
        lv_obj_clean(stage);

        // The cleans above destroyed every group card and with it every XY pad
        // that xyPads[] still points at. Only the SURVIVING groups re-register
        // their pad below, so without this a shrink (nVisible dropping) leaves
        // dead pointers: closeSelectionOnOutsidePressCb walks all 10 slots and
        // would dereference a freed pad (LV_USE_CHECK_OBJ_VALIDITY is 0 here,
        // so lv_obj_is_valid cannot catch it).
        if (xyPads) {
            for (int i = 0; i < 10; ++i) xyPads[i] = nullptr;
        }

        // Fix #2 (Round 2): clean-then-null the PRIMARY comb block. Both
        // cleans above freed every FILTERS-tab control registered in
        // widgets[]/labels[] (cards, group tabs, zones, group headers), but
        // the build loop below only re-registers the SURVIVING groups:
        // removeCombCallback vacates the old last group's tail params
        // (fNumCombStages -= 4, PluginUI.cpp:2573) and nothing else ever
        // reclaims those slots — so preset load (PluginUI.cpp:1037-1042) /
        // host automation (:773) walked stale non-null entries past the
        // null-only guards (updateLabel :1967, syncParamWidget :354) and
        // wrote through freed objects (LV_USE_CHECK_OBJ_VALIDITY is 0 in
        // this build, so lv_label_set_text dereferences unconditionally).
        // Placed AFTER the cleans so any LV_EVENT_DELETE-time restore is
        // wiped too (deleteStateCb nulls instead of restoring — Fix #1 —
        // and this belt-and-braces any other handler). Comb-owned ranges
        // only:
        //  * kParamCombBankOn is registered by the PERSISTENT tab header
        //    (Stage3Tab::build, outside both cleaned subtrees) — nulling it
        //    would evict a live registration populate never re-registers;
        //  * the mod drawer/canvas bind their comb targets in the SECONDARY
        //    arrays (UIModulationPanel canvasParams), untouched here;
        //  * resonatorDiffusion / combMemoryDamping have no primary
        //    registration anywhere, so there is nothing to clear.
        auto nullRange = [&](uint32_t lo, uint32_t hi) {
            for (uint32_t i = lo; i <= hi && i < (uint32_t)PluginCymbals::kParameterCount; ++i) {
                widgets[i] = nullptr;
                labels[i] = nullptr;
            }
        };
        nullRange(PluginCymbals::kParamComb1Freq, PluginCymbals::kParamCombScale);
        nullRange(PluginCymbals::kParamCombGroup1Cluster, PluginCymbals::kParamCombGroup10Cluster);
        nullRange(PluginCymbals::kParamCombGroup1On, PluginCymbals::kParamCombGroup10On);
        nullRange(PluginCymbals::kParamComb1FilterType, PluginCymbals::kParamComb40FilterType);
        nullRange(PluginCymbals::kParamComb1ModeRate, PluginCymbals::kParamComb40ModeAmount);
        nullRange(PluginCymbals::kParamComb1PartRelPos, PluginCymbals::kParamComb40PartCount);

        const int groupCount = std::max(nVisible / 4, 1);
        for (int g = 0; g < groupCount; ++g) {
            // Browser-style tab: [ON/OFF zone] "GROUP N" [X zone].
            lv_obj_t* btn = lv_btn_create(subTabBar);
            // Six tabs at 136px + the left cluster + dice exceed 1280 and made
            // the header wrap (the dice appeared to drop to a second line).
            const lv_coord_t tabW = (groupCount >= 5) ? scaled(112) : scaled(136);
            lv_obj_set_size(btn, tabW, scaled(32));
            lv_obj_set_style_pad_all(btn, 0, 0);
            lv_obj_set_style_radius(btn, 0, 0);
            lv_obj_set_layout(btn, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            // NO added button styles here. In this LVGL build an ADDED style
            // outranks every local style on the object (btnMain/btnChecked
            // text_color therefore won over the caption colour and produced
            // black-on-black). The group tab is painted entirely by
            // refreshGroupDimming/syncCombGroupVisibility instead; only the
            // pressed feedback is shared (it carries no text colour).
            lv_obj_add_style(btn, &styles.btnPressed, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, COL_PANEL, 0);
            lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
            lv_obj_set_style_bg_grad_dir(btn, LV_GRAD_DIR_NONE, 0);
            lv_obj_set_style_border_width(btn, 0, 0);
            lv_obj_set_style_radius(btn, 0, 0);
            lv_obj_set_style_pad_all(btn, 0, 0);
            lv_obj_set_user_data(btn, reinterpret_cast<void*>((intptr_t)g));
            // Switch on PRESSED/RELEASED/CLICKED; nothing is deleted mid-click.
            lv_obj_add_event_cb(btn, subTabCb, LV_EVENT_PRESSED, ui);
            lv_obj_add_event_cb(btn, subTabCb, LV_EVENT_RELEASED, ui);
            lv_obj_add_event_cb(btn, subTabCb, LV_EVENT_CLICKED, ui);

            // LEFT ZONE: group ON/OFF (per-group enable param).
            const int onParam = PluginCymbals::kParamCombGroup1On + g;
            lv_obj_t* onZone = lv_btn_create(btn);
            lv_obj_set_size(onZone, scaled(26), lv_pct(100));
            lv_obj_set_style_radius(onZone, 0, 0);
            lv_obj_set_style_pad_all(onZone, 0, 0);
            lv_obj_add_flag(onZone, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_user_data(onZone, reinterpret_cast<void*>((intptr_t)onParam));
            lv_obj_add_event_cb(onZone, valueCb, LV_EVENT_VALUE_CHANGED, ui);
            widgets[onParam] = onZone;
            const bool isOn = fParams[onParam] > 0.5f;
            styles.applyPowerZoneToggle(onZone, isOn);
            lv_obj_set_style_bg_color(onZone, COL_HIGHLIGHT, LV_STATE_CHECKED);
            lv_obj_set_style_bg_grad_dir(onZone, LV_GRAD_DIR_NONE, LV_STATE_CHECKED);

            // Tab name (middle).
            char buf[16];
            // Two-line caption on every tab ("GROUP" / number) so the active
            // GROUP 1 does not read on one line while GROUP 2-6 wrap.
            snprintf(buf, sizeof(buf), "GROUP\n%d", g + 1);
            // Plain wrapper: a label parented directly to the checkable tab
            // inherited the button's ADDED CHECKED text colour (COL_BG) even
            // when the group was switched off, leaving black-on-black. The
            // wrapper carries the local colour; tab checked state cannot leak
            // an added style through it.
            lv_obj_t* capBox = lv_obj_create(btn);
            lv_obj_set_size(capBox, 0, lv_pct(100));
            lv_obj_set_flex_grow(capBox, 1);
            lv_obj_set_style_bg_opa(capBox, 0, 0);
            lv_obj_set_style_border_width(capBox, 0, 0);
            lv_obj_set_style_pad_all(capBox, 0, 0);
            lv_obj_clear_flag(capBox, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_clear_flag(capBox, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_color(capBox, COL_TEXT, 0);
            lv_obj_t* lbl = lv_label_create(capBox);
            lv_label_set_text(lbl, buf);
            lv_obj_set_style_text_font(lbl, getScaledSmallFont(), 0);
            lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_width(lbl, lv_pct(100));
            lv_obj_center(lbl);

            // RIGHT ZONE: X (remove). Disabled for the last group; ON always works.
            lv_obj_t* xZone = lv_btn_create(btn);
            lv_obj_set_size(xZone, scaled(26), lv_pct(100));
            lv_obj_set_style_radius(xZone, 0, 0);
            lv_obj_set_style_pad_all(xZone, 0, 0);
            lv_obj_set_style_bg_opa(xZone, LV_OPA_0, 0);
            // Hairline separator between the group name and the X.
            lv_obj_set_style_border_side(xZone, LV_BORDER_SIDE_LEFT, 0);
            lv_obj_set_style_border_width(xZone, 1, 0);
            lv_obj_set_style_border_color(xZone, COL_TEXT, 0);
            lv_obj_set_style_border_opa(xZone, LV_OPA_60, 0);
            lv_obj_set_user_data(xZone, reinterpret_cast<void*>((intptr_t)g));
            lv_obj_t* xLbl = lv_label_create(xZone);
            lv_label_set_text(xLbl, "X");
            lv_obj_set_style_text_font(xLbl, &lv_font_montserrat_14, 0);
            lv_obj_center(xLbl);
            if (removeCombCb != nullptr && groupCount > 1) {
                lv_obj_add_event_cb(xZone, removeCombCb, LV_EVENT_CLICKED, ui);
            } else {
                lv_obj_remove_flag(xZone, LV_OBJ_FLAG_CLICKABLE);
                lv_obj_add_state(xZone, LV_STATE_DISABLED);
                lv_obj_set_style_text_color(xLbl, COL_TEXT_DIM, 0);
            }

            appendStage(stage, ui, styles, widgets, labels, fParams, valueCb,
                        g * 4, removeCombCb, randomizeGroupCb, tuningCb, xyPads,
                        groupLPFHPFCb, randomizeGroupTunedCb);
            if (g != activeGroup) {
                const uint32_t n = lv_obj_get_child_count(stage);
                if (n > 0) lv_obj_add_flag(lv_obj_get_child(stage, n - 1), LV_OBJ_FLAG_HIDDEN);
            }
        }

        // Every press on the FILTER screen now reaches clearSelectionOnOutsidePressCb
        // on the tab (arcs self-exclude via their own stop_bubbling), which is
        // what makes "click outside the dock closes it" work everywhere —
        // group tabs, group header, card background — and not just on the pad.
        enableTabPressBubbling(subTabBar, stage);
        refreshGroupDimming(fParams, subTabBar, stage, activeGroup);
    }

    static void appendStage(lv_obj_t* list, AbstractCymbalsUI* ui, UIStyles& styles,
                            lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                            lv_event_cb_t valueCb, int i, lv_event_cb_t /*removeCombCb*/,
                            lv_event_cb_t randomizeGroupCb, lv_event_cb_t tuningCb,
                            lv_obj_t** xyPads, lv_event_cb_t groupLPFHPFCb = nullptr,
                            lv_event_cb_t randomizeGroupTunedCb = nullptr) {
        int groupIdx = i / 4;

        // Group card. Must be an explicit flex COLUMN (like the nudge cards):
        // with no layout set, LV_SIZE_CONTENT sizing misplaces the children
        // (the header rendered detached ~36px above the card).
        lv_obj_t* card = lv_obj_create(list);
        lv_obj_set_size(card, lv_pct(100), 0);
        lv_obj_set_flex_grow(card, 1);
        lv_obj_add_style(card, &styles.panel, 0);
        lv_obj_set_layout(card, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_row(card, scaled(6), 0);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_user_data(card, reinterpret_cast<void*>((intptr_t)groupIdx));

        // Group header
        CombGroupHeader::Config headerCfg;
        char gbuf[24]; snprintf(gbuf, sizeof(gbuf), "GROUP %d", groupIdx + 1);
        headerCfg.title = gbuf;
        headerCfg.paramLPF = PluginCymbals::kParamComb1Freq + (groupIdx * 4) * 4 + 2;
        headerCfg.paramHPF = PluginCymbals::kParamComb1Freq + (groupIdx * 4) * 4 + 3;
        headerCfg.paramShift = PluginCymbals::kParamCombGroup1Shift + groupIdx;
        headerCfg.paramGain = PluginCymbals::kParamCombGroup1Gain + groupIdx;
        headerCfg.paramSeries = PluginCymbals::kParamCombBank1Series + groupIdx;
        headerCfg.paramCluster = PluginCymbals::kParamCombGroup1Cluster + groupIdx;
        headerCfg.groupIdx = groupIdx;
        headerCfg.valueCb = valueCb;
        headerCfg.tuningCb = tuningCb;
        headerCfg.randomizeCb = randomizeGroupCb;
        headerCfg.randomizeTunedCb = randomizeGroupTunedCb;
        headerCfg.groupLPFHPFCb = groupLPFHPFCb;

        // Ownership: user-allocated, deleted in deleteStateCb on LV_EVENT_DELETE.
        // Created before the header because the header builds the 1-4 selection
        // strip and publishes its buttons into this state.
        auto* st = new CombEditorState();
        st->ui = ui;
        st->groupIdx = groupIdx;
        st->selectedComb = -1;
        st->widgets = widgets;
        headerCfg.selectionCb = selectionButtonCb;
        headerCfg.selectionUserData = st;
        headerCfg.selectionButtons = st->selectionButtons;

        CombGroupHeader::build(card, ui, styles, widgets, labels, fParams, headerCfg);

        // =========================================================
        // XY PAD AREA
        // =========================================================
        lv_obj_t* mainWorkArea = lv_obj_create(card);
        lv_obj_set_size(mainWorkArea, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(mainWorkArea, 1);
        lv_obj_set_style_bg_opa(mainWorkArea, 0, 0);
        lv_obj_set_style_border_width(mainWorkArea, 0, 0);
        lv_obj_set_layout(mainWorkArea, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(mainWorkArea, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(mainWorkArea, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(mainWorkArea, 0, 0);
        lv_obj_set_style_pad_row(mainWorkArea, scaled(8), 0);
        lv_obj_clear_flag(mainWorkArea, LV_OBJ_FLAG_SCROLLABLE);

        // XY first, panel overlays on top-right when a comb circle is touched.
        // The pad grows to absorb the card's spare height (min-height floors
        // it so a short window can never collapse the pad away).
        lv_obj_t* xy = UIXYPad::create(mainWorkArea, groupIdx, ui, styles, false, combCircleTouched, st);
        lv_obj_set_width(xy, lv_pct(100));
        lv_obj_set_flex_grow(xy, 1);
        lv_obj_set_style_min_height(xy, scaled(296), 0);
        lv_obj_add_event_cb(xy, xySelectionSyncCb, LV_EVENT_VALUE_CHANGED, st);
        st->xyPad = xy;

        lv_obj_add_flag(card, LV_OBJ_FLAG_EVENT_BUBBLE);
        lv_obj_add_event_cb(card, clearSelectionOutsideXyCb, LV_EVENT_PRESSED, st);
        lv_obj_add_event_cb(card, deleteStateCb, LV_EVENT_DELETE, st);

        // FIX (dock layout): width covers the widest control row. At 792 the
        // top row could hold the 10 knob columns (10*72 + 9*6 = 774) but the
        // SOLO/ON/PHASE column did not fit next to them, so it got its own
        // full-width row between the two knob rows — a 95px band that read as
        // the gap between them. Widened to 896 (content 884) so the widest
        // possible row, 10 knob columns + the 76px action column
        // (10*72 + 9*6 + 6 + 76 = 856), stays on ONE line for every filter
        // type, and the two knob rows sit adjacent as a single compact block.
        // 952 now, for the 11th column: the comb's feedback knob joined the
        // top row, and 11 columns + the action column need
        // (11*72 + 10*6 + 6 + 76 = 934) of inner width, so 896 would wrap the
        // action column onto a second line and bring the band back.
        // Height is content-sized so the panel always encloses its rows (the
        // old fixed scaled(206) clipped the second row once partials types
        // showed it).
        // Parent the dock to the CARD, not to mainWorkArea. mainWorkArea holds
        // only the XY pad, so a dock parented there could never be dragged
        // above the pad's own top edge — which is exactly the "stuck level
        // with the feedback/frequency graph" symptom. COLOUR already parents
        // its dock to the pane that CONTAINS the group header, so matching that
        // here lets the dock float over the header too.
        lv_obj_add_flag(card, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        lv_obj_t* editor = CombEditorDock::createPanel(card, styles, scaled(1024), LV_SIZE_CONTENT);
        st->panel = editor;
        // Reparent to the display's top layer so the dock can be dragged over
        // the keyboard/spectrum footer, not just inside the filter card.
        if (lv_obj_t* host = uiOverlayRootFor(card)) {
            lv_obj_set_parent(editor, host);
            lv_obj_move_foreground(editor);
            lv_obj_set_user_data(editor, (void*)(intptr_t)kUIFloatDockPayload);
            st->panelDetached = true;
        }
        lv_obj_add_flag(editor, LV_OBJ_FLAG_HIDDEN);
        // Floating overlay dock: flex-skipped, so opening it never overflows or
        // clips the non-scrollable group card. Positioned in combCircleTouched.
        lv_obj_add_flag(editor, LV_OBJ_FLAG_FLOATING);
        // Outside-press close (see closeSelectionOnOutsidePressCb on the tab):
        // the pad carries a pointer to its own dock so the tab-level handler
        // can tell "inside this dock" from "anywhere else on the screen".
        if (auto* pst = static_cast<UIXYPad::State*>(lv_obj_get_user_data(xy)))
            pst->dockPanel = editor;
        CombEditorDock::createHeader(editor,
                         styles,
                         "FILTER",
                         "SAT TYPE",
                         "Tanh\nSoftsign\nArctan\nCubic\nHardClip\nFoldback\nAsymmetric\nSine",
                         combEditorSatTypeCb,
                         st,
                         dragPanelByHeaderCb,
                         editor,
                         &st->titleLabel,
                         &st->satTypeDropdown,
                         "FILTER TYPE",
                         "Comb\nAllpass\nNotch\nResoLP\nResoBP\nResoHP\nPhaser\nFlange\nGlitchPM\nBitcrush\nPop\nStutter\nVowel\nVocal\nTalkbox\nBeam Clamp\nBeam Open\nBeating\nMarimba\nMembrane\nMetal\nPan\nTube",
                         combEditorFilterTypeCb,
                         &st->filterTypeDropdown);

        lv_obj_t* eTop = CombEditorDock::createControlRow(editor, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER);
        // SOLO/ON/PHASE host row: the action column is the LAST child of the
        // top knob row, right of every arc (was its own full-width row, which
        // put a ~95px mostly-empty band between the two knob rows). It is
        // always visible, so it has to live in a row that is always visible.
        lv_obj_t* eActions = eTop;

        const int firstComb = groupIdx * 4;
        const int firstBase = PluginCymbals::kParamComb1Freq + firstComb * 4;
        st->arcFreq = CombEditorDock::createArc(eTop, styles, "Tune", ui->getParamValue(firstBase), 14, combEditorArcSetCb, st);
        st->arcFB = CombEditorDock::createArc(eTop, styles, "FB", ui->getParamValue(firstBase + 1), 15, combEditorArcSetCb, st);
        st->arcQ = CombEditorDock::createArc(eTop, styles, "Q", ui->getParamValue(PluginCymbals::kParamComb1Q + firstComb), 18, combEditorArcSetCb, st);
        st->arcLPF = CombEditorDock::createArc(eTop, styles, "Low Cut", ui->getParamValue(firstBase + 2), 0, combEditorArcSetCb, st);
        st->arcHPF = CombEditorDock::createArc(eTop, styles, "High Cut", ui->getParamValue(firstBase + 3), 1, combEditorArcSetCb, st);
        st->arcScale = CombEditorDock::createArc(eTop, styles, "Scale", ui->getParamValue(PluginCymbals::kParamComb1Scale + firstComb), 2, combEditorArcSetCb, st);
        st->arcMix = CombEditorDock::createArc(eTop, styles, "Mix", ui->getParamValue(PluginCymbals::kParamComb1Mix + firstComb), 3, combEditorArcSetCb, st);
        st->arcSatAmt = CombEditorDock::createArc(eTop, styles, "Sat Amt", ui->getParamValue(PluginCymbals::kParamComb1SatAmount + firstComb), 4, combEditorArcSetCb, st);
        st->arcStressOnset = CombEditorDock::createArc(eTop, styles, "Stress On", ui->getParamValue(PluginCymbals::kParamComb1StressOnset + firstComb), 5, combEditorArcSetCb, st);
        st->arcStressKnee = CombEditorDock::createArc(eTop, styles, "Stress Knee", ui->getParamValue(PluginCymbals::kParamComb1StressKnee + firstComb), 6, combEditorArcSetCb, st);
        st->arcModeRate = CombEditorDock::createArc(eTop, styles, "Mode Rate", ui->getParamValue(PluginCymbals::kParamComb1ModeRate + firstComb), 7, combEditorArcSetCb, st);
        st->arcModeAmount = CombEditorDock::createArc(eTop, styles, "Mode Amt", ui->getParamValue(PluginCymbals::kParamComb1ModeAmount + firstComb), 8, combEditorArcSetCb, st);
        // MODE A/B get their own row: the top row is ROW_WRAP and already at
        // its width budget (13 arc columns + the SOLO stack overflowed the
        // dock by 138px at 1280). The row collapses to zero height while both
        // arcs are hidden, so types without per-type controls pay nothing.
        lv_obj_t* eType = CombEditorDock::createControlRow(editor, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        st->arcModeA = CombEditorDock::createArc(eType, styles, "Mode A", ui->getParamValue(PluginCymbals::kParamComb1ModeA + firstComb), 16, combEditorArcSetCb, st);
        st->arcModeB = CombEditorDock::createArc(eType, styles, "Mode B", ui->getParamValue(PluginCymbals::kParamComb1ModeB + firstComb), 17, combEditorArcSetCb, st);
        // Mode arcs hidden until a selection refresh shows them per filter type.
        CombEditorDock::setArcVisible(st->arcModeRate, false);
        CombEditorDock::setArcVisible(st->arcModeAmount, false);
        CombEditorDock::setArcVisible(st->arcModeA, false);
        CombEditorDock::setArcVisible(st->arcModeB, false);

        // Saike Partials model controls (second row; hidden until a partials
        // type is selected — see refreshEditorFromSelection). The row is
        // LEFT-PACKED at the top row's pitch, not SPACE_BETWEEN: five knobs
        // spread across the dock's full width sat ~136px apart while the ten
        // knobs above them sat 74px apart, so the two rows read as different
        // control surfaces. The model's columns are the same 68px as the top
        // row's, so the pitch matches once the row is packed from the left.
        lv_obj_t* eModel = CombEditorDock::createControlRow(editor, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        st->arcPartRelPos = CombEditorDock::createArcBelow(eModel, styles, "Rel Pos", ui->getParamValue(PluginCymbals::kParamComb1PartRelPos + firstComb), 9, combEditorArcSetCb, st);
        st->arcPartFreqDamp = CombEditorDock::createArcBelow(eModel, styles, "Freq Damp", ui->getParamValue(PluginCymbals::kParamComb1PartFreqDamp + firstComb), 10, combEditorArcSetCb, st);
        st->arcPartStiffness = CombEditorDock::createArcBelow(eModel, styles, "Stiff", ui->getParamValue(PluginCymbals::kParamComb1PartStiffness + firstComb), 11, combEditorArcSetCb, st);
        st->arcPartStiffExp = CombEditorDock::createArcBelow(eModel, styles, "Stiff Exp", ui->getParamValue(PluginCymbals::kParamComb1PartStiffExp + firstComb), 12, combEditorArcSetCb, st);
        st->arcPartCount = CombEditorDock::createArcBelow(eModel, styles, "Parts", ui->getParamValue(PluginCymbals::kParamComb1PartCount + firstComb), 13, combEditorArcSetCb, st);
        CombEditorDock::setArcVisible(st->arcPartRelPos, false);
        CombEditorDock::setArcVisible(st->arcPartFreqDamp, false);
        CombEditorDock::setArcVisible(st->arcPartStiffness, false);
        CombEditorDock::setArcVisible(st->arcPartStiffExp, false);
        CombEditorDock::setArcVisible(st->arcPartCount, false);

        lv_obj_t* soloEnableCol = lv_obj_create(eActions);
        lv_obj_set_size(soloEnableCol, scaled(76), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(soloEnableCol, 0, 0);
        lv_obj_set_style_border_width(soloEnableCol, 0, 0);
        lv_obj_set_style_pad_all(soloEnableCol, 0, 0);
        lv_obj_set_style_pad_row(soloEnableCol, scaled(4), 0);
        lv_obj_set_layout(soloEnableCol, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(soloEnableCol, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(soloEnableCol, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        // The knob row centres this column on the cross axis (crossPlace
        // CENTER), so the old -4 pad_top nudge that compensated for a top
        // aligned column is gone.

        lv_obj_t* soloBtn = lv_btn_create(soloEnableCol);
        lv_obj_set_size(soloBtn, scaled(70), scaled(30));
        styles.applyToggleButton(soloBtn, false); // initial state handled by sync
        lv_obj_t* soloLbl = lv_label_create(soloBtn);
        lv_label_set_text(soloLbl, "SOLO");
        lv_obj_center(soloLbl);
        lv_obj_add_event_cb(soloBtn, combEditorSoloCb, LV_EVENT_VALUE_CHANGED, st);
        st->soloButton = soloBtn;

        const float initEnabled = st->ui->getParamValue(PluginCymbals::kParamComb1Enable + firstComb);
        lv_obj_t* enableBtn = lv_btn_create(soloEnableCol);
        lv_obj_set_size(enableBtn, scaled(70), scaled(30));
        styles.applyToggleButton(enableBtn, initEnabled > 0.5f);
        lv_obj_t* enableLbl = lv_label_create(enableBtn);
        lv_label_set_text(enableLbl, initEnabled > 0.5f ? "ON" : "OFF");
        lv_obj_center(enableLbl);
        lv_obj_add_event_cb(enableBtn, combEditorEnableCb, LV_EVENT_VALUE_CHANGED, st);
        st->enableButton = enableBtn;
        st->enableLabel = enableLbl;

        // PHASE toggle: flips the group's overlay curves between magnitude (dB)
        // and phase (degrees) — most useful for ALLPASS (flat magnitude).
        lv_obj_t* phaseBtn = lv_btn_create(soloEnableCol);
        lv_obj_set_size(phaseBtn, scaled(70), scaled(30));
        styles.applyToggleButton(phaseBtn, false);
        lv_obj_t* phaseLbl = lv_label_create(phaseBtn);
        lv_label_set_text(phaseLbl, "PHASE");
        lv_obj_center(phaseLbl);
        lv_obj_add_event_cb(phaseBtn, combEditorPhaseCb, LV_EVENT_VALUE_CHANGED, st);
        st->phaseToggleButton = phaseBtn;

        st->selectedComb = -1;

        if (groupIdx < 10) xyPads[groupIdx] = xy;
    }

    // Runs at the end of populateCombGroups so every group tab button and every
    // group card (including the dock) carries press bubbling. See
    // enablePressBubbleRecursive.
    static void enableTabPressBubbling(lv_obj_t* subTabBar, lv_obj_t* stage) {
        enablePressBubbleRecursive(subTabBar);
        enablePressBubbleRecursive(stage);
    }

private:
    static lv_obj_t* makeRow(lv_obj_t* parent) {
        return UIWidgets::makeRow(parent);
    }
};

END_NAMESPACE_DISTRHO

#endif // STAGE3_TAB_HPP
