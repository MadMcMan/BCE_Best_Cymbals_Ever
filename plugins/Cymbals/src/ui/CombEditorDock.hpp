#ifndef COMB_EDITOR_DOCK_HPP
#define COMB_EDITOR_DOCK_HPP

#include "UIStyles.hpp"
#include "UICommon.hpp"
#include "UIWidgets.hpp"
#include "UIXYPad.hpp"
#include "CombEditorSelectionController.hpp"
#include <unordered_map>

START_NAMESPACE_DISTRHO

struct CombEditorDock {
    using ParamIndexFn = int(*)(int);

    // True when the object or any ancestor is explicitly hidden. This is the
    // ownership question the floating dock needs: "is my pad's stage/card on
    // screen?" Geometry (lv_obj_is_visible) is unreliable here because a
    // floating overlay is laid out independently of the pad.
    static bool hiddenInTree(lv_obj_t* obj) {
        while (obj != nullptr) {
            if (lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) return true;
            obj = lv_obj_get_parent(obj);
        }
        return false;
    }

    // Last position the user dragged a dock to. Persists across hide/show,
    // group switches, and panel rebuilds (window resize) until the user drags
    // again. Shared by the COMBS and COLOUR docks.
    static inline lv_point_t sSavedDockPos{};
    static inline bool sSavedDockPosSet = false;

    // Registered on the drag header: snapshots the panel position when a drag
    // (or any header release) ends. dragUserData is the panel.
    static void rememberDockPosOnRelease(lv_event_t* e) {
        lv_obj_t* panel = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
        if (!panel) return;
        sSavedDockPos.x = lv_obj_get_x(panel);
        sSavedDockPos.y = lv_obj_get_y(panel);
        sSavedDockPosSet = true;
    }

    // Highest DEFAULT panel y that keeps the dock above the keyboard/spectrum
    // footer. Only the never-dragged placement uses this: once the user drags
    // the dock, their saved position is honoured even over the footer (they
    // must be able to move it there deliberately). The dock is parented to the
    // content root, which also owns the footer.
    static lv_coord_t dockDefaultMaxY(lv_obj_t* parent, lv_coord_t dockH) {
        if (!parent) return 0;
        const lv_coord_t ph = lv_obj_get_height(parent);
        const lv_coord_t reserve = uiFooterHeight();
        return ph > dockH + reserve ? ph - dockH - reserve : 0;
    }

    // Positions the panel at the saved drag location (clamped only to its
    // parent). Returns false when no saved position exists — the caller then
    // applies its default placement. No align style is set, so subsequent
    // drags stick.
    static bool applySavedDockPos(lv_obj_t* panel) {
        if (!panel || !sSavedDockPosSet) return false;
        lv_obj_t* parent = lv_obj_get_parent(panel);
        lv_coord_t x = sSavedDockPos.x;
        lv_coord_t y = sSavedDockPos.y;
        if (parent) {
            const lv_coord_t pw = lv_obj_get_width(parent);
            const lv_coord_t ph = lv_obj_get_height(parent);
            const lv_coord_t w = lv_obj_get_width(panel);
            const lv_coord_t h = lv_obj_get_height(panel);
            x = std::clamp(x, (lv_coord_t)0, pw > w ? pw - w : (lv_coord_t)0);
            y = std::clamp(y, (lv_coord_t)0, ph > h ? ph - h : (lv_coord_t)0);
        }
        // Every comb-parameter write funnels here through the pad sync while a
        // dock knob is being dragged; lv_obj_set_pos has no equality check and
        // would invalidate + mark the parent layout-dirty with the same coords.
        if (lv_obj_get_x(panel) != x || lv_obj_get_y(panel) != y)
            lv_obj_set_pos(panel, x, y);
        return true;
    }

    // Default placement for a dock that has never been dragged: TOP-LEFT of
    // its pad (computed explicitly — lv_obj_align's style is deferred, so
    // reading the position back immediately would capture stale coords). The
    // position is saved on first use, so the controls always appear at the
    // same spot on every group/screen until the user moves them. The caller
    // runs this after the layout is settled (a comb circle touch).
    //
    // Top, not bottom-right: the XY pad is the first flex child and fills the
    // whole card body, so a bottom-anchored dock sat on top of the response
    // curves and read as "the graph covers the frequencies". Anchoring to the
    // top puts the knob row and its Freq/LPF/HPF Hz labels above the graph
    // instead, matching the COLOUR screen's arrangement. Horizontal is
    // unchanged — the dock is near-full-width, so the inset is nominal.
    static void placeDockAtDefault(lv_obj_t* panel, lv_obj_t* anchor = nullptr) {
        if (!panel) return;
        lv_obj_t* parent = lv_obj_get_parent(panel);
        if (!parent) return;
        // The dock lives on the content root so it can float over the graph;
        // the default is computed from the XY pad's ABSOLUTE coords, not the
        // old card-local (10,10). The bottom clamp reserves the footer, so the
        // controls always sit above the keyboard/spectrum.
        lv_coord_t x = (lv_coord_t)scaled(10);
        lv_coord_t y = (lv_coord_t)scaled(10);
        if (anchor != nullptr) {
            lv_area_t a;
            lv_obj_get_coords(anchor, &a);
            x = (lv_coord_t)(a.x1 + scaled(10));
            y = (lv_coord_t)(a.y1 + scaled(10));
        }
        const lv_coord_t pw = lv_obj_get_width(parent);
        const lv_coord_t w = lv_obj_get_width(panel);
        const lv_coord_t h = lv_obj_get_height(panel);
        x = std::clamp(x, (lv_coord_t)0, pw > w ? pw - w : (lv_coord_t)0);
        y = std::clamp(y, (lv_coord_t)0, dockDefaultMaxY(parent, h));
        lv_obj_set_pos(panel, x, y);
        // A content-sized dock can grow after the first position write (the
        // caller shows it as part of the same interaction). Re-measure and
        // re-clamp so the INITIAL placement still clears the footer; a user
        // drag is never passed through here and may sit over the footer.
        lv_obj_update_layout(panel);
        const lv_coord_t settledH = lv_obj_get_height(panel);
        const lv_coord_t settledMaxY = dockDefaultMaxY(parent, settledH);
        if (y > settledMaxY) {
            y = settledMaxY;
            lv_obj_set_pos(panel, x, y);
        }
        if (!sSavedDockPosSet) {
            sSavedDockPos.x = x;
            sSavedDockPos.y = y;
            sSavedDockPosSet = true;
        }
    }

    struct BehaviorParamMap {
        int combCount = 0;
        int exclusiveSoloClearCount = 0;
        ParamIndexFn freqParam = nullptr;
        // The comb's feedback (the t==1 slot of its 4-param group). It is a
        // live DSP parameter - applyCombFamily() calls combBank.setFeedback()
        // - and it had no control until now.
        ParamIndexFn fbParam = nullptr;
        ParamIndexFn qParam = nullptr;
        ParamIndexFn lpfParam = nullptr;
        ParamIndexFn hpfParam = nullptr;
        ParamIndexFn scaleParam = nullptr;
        ParamIndexFn mixParam = nullptr;
        ParamIndexFn satAmountParam = nullptr;
        ParamIndexFn stressOnsetParam = nullptr;
        ParamIndexFn stressKneeParam = nullptr;
        ParamIndexFn satTypeParam = nullptr;
        ParamIndexFn filterTypeParam = nullptr;
        ParamIndexFn soloParam = nullptr;
        ParamIndexFn enableParam = nullptr;
        ParamIndexFn modeRateParam = nullptr;
        ParamIndexFn modeAmountParam = nullptr;
        ParamIndexFn partRelPosParam = nullptr;
        ParamIndexFn partFreqDampParam = nullptr;
        ParamIndexFn partStiffnessParam = nullptr;
        ParamIndexFn partStiffExpParam = nullptr;
        ParamIndexFn partCountParam = nullptr;
        // Per-filter-type MODE A/B (meaning depends on the FILTER TYPE).
        ParamIndexFn modeAParam = nullptr;
        ParamIndexFn modeBParam = nullptr;
    };

    struct ArcDragState {
        lv_point_t startPoint{0, 0};
        int startValue = 0;
        bool active = false;
    };

    template <typename State>
    static int getTargetMask(State* st) {
        if (!st) return 0;
        int mask = 0;
        if (st->xyPad) mask = UIXYPad::getSelectedMask(st->xyPad);

        if (mask == 0 && st->selectedComb >= 0) {
            const int local = std::clamp(st->selectedComb - st->groupIdx * 4, 0, 3);
            mask = (1 << local);
        }
        return mask;
    }

    template <typename State>
    static void updateSelectionTitle(State* st) {
        if (!st || !st->titleLabel) return;
        const int mask = getTargetMask(st);
        CombEditorSelectionController::setTitleFromMask(st->titleLabel, mask, st->groupIdx * 4 + 1);
        if (st->ui) {
            const bool bankOn = st->isFinal
                ? (st->ui->getParamValue(PluginCymbals::kParamFinalCombBankOn) >= 0.5f)
                : (st->ui->getParamValue(PluginCymbals::kParamCombBankOn) >= 0.5f);
            if (!bankOn) {
                // The per-stage ON can be lit while the whole bank is bypassed;
                // say so in the dock instead of leaving a working-looking panel.
                std::string t = lv_label_get_text(st->titleLabel);
                t += "  -  BANK OFF";
                lv_label_set_text(st->titleLabel, t.c_str());
            }
        }
    }

    // Fix B (steal-then-null): the mod-target registration in
    // refreshEditorFromSelection OVERWRITES widgets[p] with a dock arc —
    // stealing e.g. the group header's LPF/HPF slot (Stage3Tab/StageFinalTab
    // register their header arcs there at build). Every teardown path used
    // to write nullptr back into those slots, so closing the dock (or
    // destroying the editor State) left the REAL owner unregistered for the
    // rest of the session: its mod ring, cable endpoint and hit-testing went
    // dead until a full rebuild. Registration now saves the evicted owner in
    // State::savedWidgets[] and this hands the slot back.
    //
    // LVGL teardown order is arbitrary, so the restore only runs when the
    // slot is still ours (a live arc registered in arcWhichMap), vacant, or
    // pointing at a dead object — a live FOREIGN owner (e.g. a rebuilt
    // header's new arc re-claiming the slot meanwhile) is never evicted.
    //
    // Fix #1 exception: deleteStateCb (Stage3Tab/StageFinalTab), which fires
    // while the card/combPane subtree is itself being deleted, does NOT call
    // this restore — it nulls the slots instead, because every saved owner
    // dies in that same deletion and lv_obj_is_valid cannot detect an owner
    // queued to die in this very pass. Restore is only correct on the rebind
    // and dock-close paths, whose owners are still alive.
    template <typename State>
    static void releaseModTargetBindings(State* st) {
        if (!st) return;
        if (st->widgets) {
            for (int k = 0; k < st->modParamCount; ++k) {
                const uint32_t p = st->modParams[k];
                if (p && p < PluginCymbals::kParameterCount) {
                    lv_obj_t* cur = st->widgets[p];
                    const bool slotIsOurs = !cur || !lv_obj_is_valid(cur) ||
                                            arcWhichMap().find(cur) != arcWhichMap().end();
                    if (slotIsOurs) {
                        lv_obj_t* prev = st->savedWidgets[k];
                        st->widgets[p] = (prev && lv_obj_is_valid(prev)) ? prev : nullptr;
                    }
                }
                st->savedWidgets[k] = nullptr;
            }
        }
        st->modParamCount = 0;
    }

    template <typename State>
    static void refreshEditorFromSelection(State* st, const BehaviorParamMap& map) {
        if (!st || !st->ui || map.combCount <= 0) return;
        updateSelectionTitle(st);

        const int c = std::clamp(st->selectedComb, 0, map.combCount - 1);
        const float lpf = st->ui->getParamValue(map.lpfParam(c));
        const float hpf = st->ui->getParamValue(map.hpfParam(c));
        const float scale = st->ui->getParamValue(map.scaleParam(c));
        const float mix = st->ui->getParamValue(map.mixParam(c));
        const float satAmt = st->ui->getParamValue(map.satAmountParam(c));
        const float stressOnset = st->ui->getParamValue(map.stressOnsetParam(c));
        const float stressKnee = st->ui->getParamValue(map.stressKneeParam(c));
        const int satType = (int)st->ui->getParamValue(map.satTypeParam(c));
        const int filterType = (int)st->ui->getParamValue(map.filterTypeParam(c));
        const float solo = st->ui->getParamValue(map.soloParam(c));
        const float enabled = st->ui->getParamValue(map.enableParam(c));

        if (st->arcFreq && map.freqParam) setArcNormalizedValue(st->arcFreq, st->ui->getParamValue(map.freqParam(c)));
        if (st->arcFB && map.fbParam) setArcNormalizedValue(st->arcFB, st->ui->getParamValue(map.fbParam(c)));
        if (st->arcQ && map.qParam) {
            setArcNormalizedValue(st->arcQ, st->ui->getParamValue(map.qParam(c)));
            st->ui->updateParamLabel(map.qParam(c));
        }
        if (st->arcLPF) setArcNormalizedValue(st->arcLPF, lpf);
        if (st->arcHPF) setArcNormalizedValue(st->arcHPF, hpf);
        if (st->arcScale) setArcNormalizedValue(st->arcScale, scale);
        if (st->arcMix) setArcNormalizedValue(st->arcMix, mix);
        if (st->arcSatAmt) setArcNormalizedValue(st->arcSatAmt, satAmt);
        if (st->arcStressOnset) setArcNormalizedValue(st->arcStressOnset, stressOnset);
        if (st->arcStressKnee) setArcNormalizedValue(st->arcStressKnee, stressKnee);
        if (st->satTypeDropdown) lv_dropdown_set_selected(st->satTypeDropdown, satType);
        if (st->filterTypeDropdown) lv_dropdown_set_selected(st->filterTypeDropdown, filterType);

        // Mode-specific controls, visible only for the filter types that use them:
        // MODE RATE drives FLANGE/PHASER LFOs and the TALKBOX vowel sweep;
        // MODE AMT is the ALLPASS dry-wet and the FLANGE wet tap.
        if (map.modeRateParam && st->arcModeRate) setArcNormalizedValue(st->arcModeRate, st->ui->getParamValue(map.modeRateParam(c)));
        if (map.modeAmountParam && st->arcModeAmount) setArcNormalizedValue(st->arcModeAmount, st->ui->getParamValue(map.modeAmountParam(c)));
        if (map.modeAParam && st->arcModeA) setArcNormalizedValue(st->arcModeA, st->ui->getParamValue(map.modeAParam(c)));
        if (map.modeBParam && st->arcModeB) setArcNormalizedValue(st->arcModeB, st->ui->getParamValue(map.modeBParam(c)));
        const bool rateRelevant = (filterType == CombFilterBank::FT_FLANGE ||
                                   filterType == CombFilterBank::FT_PHASER ||
                                   filterType == CombFilterBank::FT_TALKBOX ||
                                   CombFilterBank::isPartialType(filterType));
        const bool amtRelevant = (filterType == CombFilterBank::FT_ALLPASS ||
                                  filterType == CombFilterBank::FT_FLANGE ||
                                  CombFilterBank::isPartialType(filterType));
        setArcVisible(st->arcModeRate, rateRelevant);
        setArcVisible(st->arcModeAmount, amtRelevant);
        // Q only exists in the biquad/formant paths. On COMB (and the other
        // non-biquad modes) there is no Q to hear or draw, so leaving the knob
        // visible made a real control look broken. Hide it there.
        const bool qRelevant = (filterType == CombFilterBank::FT_NOTCH ||
                                filterType == CombFilterBank::FT_RESO_LP ||
                                filterType == CombFilterBank::FT_RESO_BP ||
                                filterType == CombFilterBank::FT_RESO_HP ||
                                filterType == CombFilterBank::FT_PHASER ||
                                filterType == CombFilterBank::FT_ALLPASS ||
                                filterType == CombFilterBank::FT_VOWEL ||
                                filterType == CombFilterBank::FT_VOCAL ||
                                filterType == CombFilterBank::FT_TALKBOX);
        setArcVisible(st->arcQ, qRelevant);
        // FLANGE does not read the comb frequency at all: its delay is the
        // fixed 2-12 ms LFO sweep (CombFilterBank FT_FLANGE), so the response
        // chart cannot move when Freq turns. Hide the knob instead of leaving
        // a control that provably does nothing.
        setArcVisible(st->arcFreq, filterType != CombFilterBank::FT_FLANGE);

        // Per-filter-type MODE A/B. Both are real parameters (host-automatable
        // and modulation-matrix targets); only the controls that a type
        // actually reads are shown, with a label matching the meaning in
        // CombFilterBank.
        const char* labelA = nullptr;
        const char* labelB = nullptr;
        switch (filterType) {
            case CombFilterBank::FT_FLANGE:    labelA = "Delay";   labelB = "Sweep"; break;
            case CombFilterBank::FT_PHASER:    labelA = "Depth";   labelB = "Mix";   break;
            case CombFilterBank::FT_GLITCH_PM: labelA = "PM Dep"; labelB = "PM Rat"; break;
            case CombFilterBank::FT_BITCRUSH:  labelA = "Bits";    labelB = "Rate";  break;
            case CombFilterBank::FT_POP:       labelA = "Level";   labelB = "Rate";  break;
            case CombFilterBank::FT_STUTTER:   labelA = "Window";  labelB = "Rate";  break;
            case CombFilterBank::FT_VOWEL:     labelA = "Spread";  break;
            case CombFilterBank::FT_VOCAL:     labelA = "Spread";  labelB = "Pitch"; break;
            case CombFilterBank::FT_TALKBOX:   labelA = "Spread";  labelB = "Morph"; break;
            default: break;
        }
        auto setArcTitle = [](lv_obj_t* arc, const char* text) {
            if (arc == nullptr) return;
            lv_obj_t* knobCont = lv_obj_get_parent(arc);
            lv_obj_t* cont = knobCont ? lv_obj_get_parent(knobCont) : nullptr;
            lv_obj_t* title = cont ? lv_obj_get_child(cont, 0) : nullptr;
            if (title && text) lv_label_set_text(title, text);
        };
        setArcVisible(st->arcModeA, labelA != nullptr);
        setArcVisible(st->arcModeB, labelB != nullptr);
        setArcTitle(st->arcModeA, labelA);
        setArcTitle(st->arcModeB, labelB);

        // Saike Partials model controls — only for FT_PARTIAL_* types. Mode
        // Amt = damp, Mode Rate = inharmonicity; the arcs below are the rest
        // of the Saike slider set (rel_position, freq_damp, stiffness,
        // stiffness_exp, partials count).
        const bool partRelevant = CombFilterBank::isPartialType(filterType);
        if (map.partRelPosParam && st->arcPartRelPos) setArcNormalizedValue(st->arcPartRelPos, st->ui->getParamValue(map.partRelPosParam(c)));
        if (map.partFreqDampParam && st->arcPartFreqDamp) setArcNormalizedValue(st->arcPartFreqDamp, st->ui->getParamValue(map.partFreqDampParam(c)));
        if (map.partStiffnessParam && st->arcPartStiffness) setArcNormalizedValue(st->arcPartStiffness, st->ui->getParamValue(map.partStiffnessParam(c)));
        if (map.partStiffExpParam && st->arcPartStiffExp) setArcNormalizedValue(st->arcPartStiffExp, st->ui->getParamValue(map.partStiffExpParam(c)));
        if (map.partCountParam && st->arcPartCount) setArcNormalizedValue(st->arcPartCount, st->ui->getParamValue(map.partCountParam(c)));
        setArcVisible(st->arcPartRelPos, partRelevant);
        setArcVisible(st->arcPartFreqDamp, partRelevant);
        setArcVisible(st->arcPartStiffness, partRelevant);
        setArcVisible(st->arcPartStiffExp, partRelevant);
        setArcVisible(st->arcPartCount, partRelevant);

        if (st->phaseToggleButton) {
            const bool phaseOn = UIXYPad::getPhaseView(st->xyPad);
            if (phaseOn) lv_obj_add_state(st->phaseToggleButton, LV_STATE_CHECKED);
            else lv_obj_clear_state(st->phaseToggleButton, LV_STATE_CHECKED);
            UIStyles::setToggleTick(st->phaseToggleButton, phaseOn, false);
        }

        if (st->soloButton) {
            const bool soloOn = solo > 0.5f;
            if (soloOn) lv_obj_add_state(st->soloButton, LV_STATE_CHECKED);
            else lv_obj_clear_state(st->soloButton, LV_STATE_CHECKED);
            UIStyles::setToggleTick(st->soloButton, soloOn, false);
        }
        if (st->enableButton) {
            const bool enOn = enabled > 0.5f;
            if (enOn) lv_obj_add_state(st->enableButton, LV_STATE_CHECKED);
            else lv_obj_clear_state(st->enableButton, LV_STATE_CHECKED);
            UIStyles::setToggleTick(st->enableButton, enOn, false);
        }
        if (st->enableLabel) lv_label_set_text(st->enableLabel, enabled > 0.5f ? "ON" : "OFF");

        // Mod-target registration: bind the routable dock arcs to the selected
        // comb's params (user_data = param index) so cable-drop + mod rings
        // work on the dock. partCount/satType/filterType/solo/enable are
        // discrete and stay unregistered.
        if (st->widgets) {
            // Fix B: hand previously-stolen slots back to their original
            // owners before rebinding (the selection may have moved to a
            // different comb/header in the meantime).
            releaseModTargetBindings(st);
            auto reg = [&](lv_obj_t* arc, ParamIndexFn fn) {
                if (!arc || !fn) return;
                const uint32_t p = (uint32_t)fn(c);
                lv_obj_set_user_data(arc, (void*)(intptr_t)p);
                // Fix B: remember whoever owns the slot right now (usually
                // the header's arc) so releaseModTargetBindings can restore
                // it when this binding goes away.
                lv_obj_t* prev = st->widgets[p];
                st->widgets[p] = arc;
                if (st->modParamCount < 24) {
                    st->savedWidgets[st->modParamCount] = prev;
                    st->modParams[st->modParamCount++] = p;
                }
            };
            reg(st->arcFreq, map.freqParam);
            reg(st->arcFB, map.fbParam);
            reg(st->arcQ, map.qParam);
            reg(st->arcLPF, map.lpfParam);
            reg(st->arcHPF, map.hpfParam);
            reg(st->arcScale, map.scaleParam);
            reg(st->arcMix, map.mixParam);
            reg(st->arcSatAmt, map.satAmountParam);
            reg(st->arcStressOnset, map.stressOnsetParam);
            reg(st->arcStressKnee, map.stressKneeParam);
            reg(st->arcModeRate, map.modeRateParam);
            reg(st->arcModeAmount, map.modeAmountParam);
            reg(st->arcModeA, map.modeAParam);
            reg(st->arcModeB, map.modeBParam);
            reg(st->arcPartRelPos, map.partRelPosParam);
            reg(st->arcPartFreqDamp, map.partFreqDampParam);
            reg(st->arcPartStiffness, map.partStiffnessParam);
            reg(st->arcPartStiffExp, map.partStiffExpParam);
        }
    }

    template <typename State>
    static void applyArcToSelection(State* st, const BehaviorParamMap& map, lv_obj_t* arc, int which, float value) {
        if (!st || !st->ui || map.combCount <= 0) return;
        if (st->selectedComb < 0) return;

        const int mask = getTargetMask(st);
        if (mask == 0) return;
        updateSelectionTitle(st);

        for (int local = 0; local < 4; ++local) {
            if ((mask & (1 << local)) == 0) continue;
            const int c = std::clamp(st->groupIdx * 4 + local, 0, map.combCount - 1);

            int p = map.scaleParam(c);
            if (which == 14) p = map.freqParam(c);
            else if (which == 15) p = map.fbParam(c);
            else if (which == 18) p = map.qParam(c);
            else if (which == 0) p = map.lpfParam(c);
            else if (which == 1) p = map.hpfParam(c);
            else if (which == 2) p = map.scaleParam(c);
            else if (which == 3) p = map.mixParam(c);
            else if (which == 4) p = map.satAmountParam(c);
            else if (which == 5) p = map.stressOnsetParam(c);
            else if (which == 6) p = map.stressKneeParam(c);
            else if (which == 7) p = map.modeRateParam(c);
            else if (which == 8) p = map.modeAmountParam(c);
            else if (which == 9) p = map.partRelPosParam(c);
            else if (which == 10) p = map.partFreqDampParam(c);
            else if (which == 11) p = map.partStiffnessParam(c);
            else if (which == 12) p = map.partStiffExpParam(c);
            else if (which == 13) p = map.partCountParam(c);
            else if (which == 16) p = map.modeAParam(c);
            else if (which == 17) p = map.modeBParam(c);

            st->ui->editParameter(p, true);
            st->ui->setParamValue(p, value);
            st->ui->editParameter(p, false);
        }

        setArcNormalizedValue(arc, value);
        if (st->xyPad) lv_obj_invalidate(st->xyPad);
    }

    template <typename State>
    static void applySoloToSelection(State* st, const BehaviorParamMap& map, float value) {
        if (!st || !st->ui || map.combCount <= 0) return;
        const int mask = getTargetMask(st);
        if (mask == 0) return;

        if (value > 0.5f) {
            const int clearCount = std::max(0, map.exclusiveSoloClearCount);
            for (int c = 0; c < clearCount; ++c) {
                const int p = map.soloParam(c);
                st->ui->editParameter(p, true);
                st->ui->setParamValue(p, 0.0f);
                st->ui->editParameter(p, false);
            }
        }

        for (int local = 0; local < 4; ++local) {
            if ((mask & (1 << local)) == 0) continue;
            const int c = std::clamp(st->groupIdx * 4 + local, 0, map.combCount - 1);
            const int p = map.soloParam(c);
            st->ui->editParameter(p, true);
            st->ui->setParamValue(p, value);
            st->ui->editParameter(p, false);
        }

        if (st->xyPad) lv_obj_invalidate(st->xyPad);
    }

    template <typename State>
    static void applyEnableToSelection(State* st, const BehaviorParamMap& map, float value) {
        if (!st || !st->ui || map.combCount <= 0) return;
        if (st->enableLabel) lv_label_set_text(st->enableLabel, value > 0.5f ? "ON" : "OFF");

        const int mask = getTargetMask(st);
        if (mask == 0) return;

        for (int local = 0; local < 4; ++local) {
            if ((mask & (1 << local)) == 0) continue;
            const int c = std::clamp(st->groupIdx * 4 + local, 0, map.combCount - 1);

            const int ep = map.enableParam(c);
            st->ui->editParameter(ep, true);
            st->ui->setParamValue(ep, value);
            st->ui->editParameter(ep, false);
            // The dock's ON is also mirrored by the header's numbered enable
            // pills (registered in widgets[]). setParamValue alone bypasses
            // the UI callback, so push the same value into that button; without
            // this the modal said ON while the number pill stayed dark.
            st->ui->syncParamWidget((uint32_t)ep, value);

            if (value < 0.5f) {
                const int sp = map.soloParam(c);
                st->ui->editParameter(sp, true);
                st->ui->setParamValue(sp, 0.0f);
                st->ui->editParameter(sp, false);
            }
        }

        if (st->xyPad) lv_obj_invalidate(st->xyPad);
    }

    template <typename State>
    static void applySatTypeToSelection(State* st, const BehaviorParamMap& map, int satType) {
        if (!st || !st->ui || map.combCount <= 0) return;
        if (st->selectedComb < 0) return;

        const int mask = getTargetMask(st);
        if (mask == 0) return;
        updateSelectionTitle(st);

        for (int local = 0; local < 4; ++local) {
            if ((mask & (1 << local)) == 0) continue;
            const int c = std::clamp(st->groupIdx * 4 + local, 0, map.combCount - 1);
            const int p = map.satTypeParam(c);
            st->ui->editParameter(p, true);
            st->ui->setParamValue(p, (float)satType);
            st->ui->editParameter(p, false);
        }

        if (st->xyPad) lv_obj_invalidate(st->xyPad);
    }

    template <typename State>
    static void applyFilterTypeToSelection(State* st, const BehaviorParamMap& map, int filterType) {
        if (!st || !st->ui || map.combCount <= 0) return;
        if (st->selectedComb < 0) return;

        const int mask = getTargetMask(st);
        if (mask == 0) return;
        updateSelectionTitle(st);

        for (int local = 0; local < 4; ++local) {
            if ((mask & (1 << local)) == 0) continue;
            const int c = std::clamp(st->groupIdx * 4 + local, 0, map.combCount - 1);
            const int p = map.filterTypeParam(c);
            st->ui->editParameter(p, true);
            st->ui->setParamValue(p, (float)filterType);
            st->ui->editParameter(p, false);
        }

        // Discrete switch: snap the response curves (no one-pole morph) and
        // force an immediate redraw. Without the explicit invalidate the chart
        // could keep its previous type until some unrelated repaint, which is
        // exactly the "changed FILTER TYPE but chart didn't move" report.
        if (st->xyPad) {
            UIXYPad::resetCurveSmoothing(st->xyPad);
            lv_obj_invalidate(st->xyPad);
        }
        // The dropdown path must refresh the dock: mode-arc visibility and
        // values depend on the selected filter type (setParamValue updated the
        // UI fParams synchronously, so the refresh reads fresh values).
        refreshEditorFromSelection(st, map);
    }

    template <typename State>
    static void syncEditorFromXySelection(State* st, const BehaviorParamMap& map) {
        if (!st || !st->panel || !st->xyPad) return;

        const int mask = CombEditorSelectionController::syncPanelVisibilityAndTitle(
            st->panel,
            st->titleLabel,
            st->xyPad,
            st->groupIdx * 4 + 1);

        if (mask == 0) {
            st->selectedComb = -1;
            // Dock closed: release the mod-target bindings so no phantom drop
            // targets or rings survive — Fix B restores each slot's original
            // owner instead of just nulling it.
            releaseModTargetBindings(st);
            return;
        }

        // The dock is a floating overlay on the content root, so the stage
        // switch cannot hide it through its parent. It must stay hidden while
        // its owning pad is not on screen: invalidateCombVisuals() sends a
        // VALUE_CHANGED to the pad on every filter/comb parameter change, and
        // without this gate that revoked the stage-switch hide and dropped the
        // dock over whatever page was open (reported: FILTERS 1+2+3+4 floating
        // over FX CHAIN's FINAL TOUCHES / MASTER ADSR controls).
        if (st->xyPad && hiddenInTree(st->xyPad)) {
            lv_obj_add_flag(st->panel, LV_OBJ_FLAG_HIDDEN);
            return;
        }

        // The footer is a later sibling of the content-root dock and therefore
        // paints above it. Raise the dock every time it is shown, otherwise a
        // drag toward the keyboard/spectrum moves it BEHIND the footer and the
        // user cannot see (or grab) it there.
        lv_obj_move_foreground(st->panel);

        // Lasso auto-open: the panel was never positioned (only
        // combCircleTouched places it). A FLOATING panel without an explicit
        // position renders at the parent's origin, covering the pad and group
        // card — visibly broken at every window scale. Position it now using
        // the same rule as a circle touch: saved drag location if any, else
        // the default bottom-right placement.
        if (!applySavedDockPos(st->panel))
            placeDockAtDefault(st->panel);

        if (st->selectedComb < 0) {
            for (int i = 0; i < 4; ++i) {
                if (mask & (1 << i)) {
                    st->selectedComb = st->groupIdx * 4 + i;
                    break;
                }
            }
        }

        refreshEditorFromSelection(st, map);
    }

    static std::unordered_map<lv_obj_t*, ArcDragState>& dragStates() {
        static std::unordered_map<lv_obj_t*, ArcDragState> s;
        return s;
    }

    static void arcDragCb(lv_event_t* e) {
        lv_obj_t* arc = static_cast<lv_obj_t*>(lv_event_get_target(e));
        const lv_event_code_t code = lv_event_get_code(e);

        if (code == LV_EVENT_DELETE) {
            dragStates().erase(arc);
            arcWhichMap().erase(arc);
            return;
        }

        lv_indev_t* indev = lv_indev_get_act();
        if (!indev) return;

        if (code == LV_EVENT_PRESSED) {
            lv_point_t p;
            lv_indev_get_point(indev, &p);
            ArcDragState& st = dragStates()[arc];
            st.startPoint = p;
            st.startValue = lv_arc_get_value(arc);
            st.active = true;
            lv_event_stop_bubbling(e);
            return;
        }

        if (code == LV_EVENT_PRESSING) {
            auto it = dragStates().find(arc);
            if (it == dragStates().end() || !it->second.active) return;

            lv_point_t p;
            lv_indev_get_point(indev, &p);
            const int dx = p.x - it->second.startPoint.x;
            const int dy = p.y - it->second.startPoint.y;
            const float travel = (float)dx - (float)dy; // Up/right increase.
            // Shift = fine drag, same 10x precision as the shared kit knobs.
            const float fine = UIWidgets::isFineDragActive() ? 0.1f : 1.0f;
            const int next = std::clamp(
                (int)std::lround((float)it->second.startValue + travel * 2.2f * fine), 0, 1000);

            if (next != lv_arc_get_value(arc)) {
                lv_arc_set_value(arc, next);
                lv_obj_send_event(arc, LV_EVENT_VALUE_CHANGED, nullptr);
            }
            lv_event_stop_bubbling(e);
            return;
        }

        if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
            auto it = dragStates().find(arc);
            if (it != dragStates().end()) it->second.active = false;
            lv_event_stop_bubbling(e);
        }
    }

    static void setArcVisible(lv_obj_t* arc, bool visible) {
        if (!arc) return;
        // arc is a child of knobCont, which is a child of the outer cont
        // (nameLbl + knobCont + valLbl) — hide the whole column.
        lv_obj_t* knobCont = lv_obj_get_parent(arc);
        lv_obj_t* cont = knobCont ? lv_obj_get_parent(knobCont) : nullptr;
        if (!cont) return;
        if (visible) lv_obj_clear_flag(cont, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
    }

    static lv_obj_t* createPanel(lv_obj_t* parent, UIStyles& /*styles*/, lv_coord_t width, lv_coord_t height) {
        lv_obj_t* panel = lv_obj_create(parent);
        lv_obj_set_size(panel, width, height);
        lv_obj_set_style_bg_color(panel, COL_BG, 0);
        // Opaque: at bg_opa 228 the XY pad's grid lines, response curves and
        // per-comb Hz captions showed through the dock and collided with the
        // Freq/LPF/HPF value labels. The dock is a control surface, not a
        // scrim, so it must fully occlude what it covers.
        lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(panel, COL_HIGHLIGHT, 0);
        lv_obj_set_style_border_width(panel, 1, 0);
        lv_obj_set_style_border_opa(panel, 190, 0);
        lv_obj_set_style_radius(panel, scaled(14), 0);
        lv_obj_set_style_shadow_width(panel, scaled(18), 0);
        lv_obj_set_style_shadow_spread(panel, scaled(2), 0);
        lv_obj_set_style_shadow_opa(panel, 90, 0);
        lv_obj_set_style_shadow_color(panel, COL_SHADOW, 0);
        // Row gaps and the outer inset are the dock's dead-space budget: the
        // FILTER modal used to read as three stacked islands (knob row,
        // SOLO/ON/PHASE row, model knob row) separated by full-height gaps.
        // Rows are now adjacent (6 -> 4) and the inset 8 -> 6.
        lv_obj_set_style_pad_all(panel, scaled(6), 0);
        lv_obj_set_style_pad_row(panel, scaled(4), 0);
        lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
        return panel;
    }

    static void createHeader(lv_obj_t* panel,
                             UIStyles& styles,
                             const char* titleText,
                             const char* satLabelText,
                             const char* satOptions,
                             lv_event_cb_t satCb,
                             void* satUserData,
                             lv_event_cb_t dragCb,
                             void* dragUserData,
                             lv_obj_t** outTitle,
                             lv_obj_t** outSatDropdown,
                             const char* filterLabelText,
                             const char* filterOptions,
                             lv_event_cb_t filterCb,
                             lv_obj_t** outFilterDropdown) {
        lv_obj_t* header = lv_obj_create(panel);
        lv_obj_set_size(header, lv_pct(100), scaled(34));
        lv_obj_set_style_bg_opa(header, 0, 0);
        lv_obj_set_style_border_width(header, 0, 0);
        lv_obj_set_style_pad_all(header, 0, 0);
        lv_obj_set_layout(header, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
        if (dragCb) {
            lv_obj_add_flag(header, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(header, dragCb, LV_EVENT_PRESSING, dragUserData);
            // Keep the dock where the user left it: snapshot the position when
            // a drag ends. dragUserData is the panel.
            lv_obj_add_event_cb(header, rememberDockPosOnRelease, LV_EVENT_RELEASED, dragUserData);
        }

        lv_obj_t* title = lv_label_create(header);
        lv_label_set_text(title, titleText);
        lv_obj_add_style(title, &styles.title, 0);

        lv_obj_t* satCont = lv_obj_create(header);
        lv_obj_set_size(satCont, scaled(224), scaled(32));
        lv_obj_set_style_bg_opa(satCont, 0, 0);
        lv_obj_set_style_border_width(satCont, 0, 0);
        lv_obj_set_style_pad_all(satCont, 0, 0);
        lv_obj_set_layout(satCont, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(satCont, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(satCont, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(satCont, scaled(8), 0);
        lv_obj_clear_flag(satCont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t* satLbl = lv_label_create(satCont);
        lv_obj_add_style(satLbl, &styles.label, 0);
        lv_obj_set_style_text_color(satLbl, COL_TEXT_DIM, 0);
        lv_obj_set_width(satLbl, scaled(100)); // enough for "FILTER TYPE" at the scaled label font
        lv_label_set_long_mode(satLbl, LV_LABEL_LONG_CLIP);
        lv_label_set_text(satLbl, satLabelText);

        lv_obj_t* satDd = lv_dropdown_create(satCont);
        lv_dropdown_set_options(satDd, satOptions);
        lv_obj_set_width(satDd, scaled(112));
        lv_obj_set_height(satDd, scaled(30));
        lv_obj_add_style(satDd, &styles.sliderMain, 0);
        // Long option lists must scroll (sanctioned exemption); panels stay locked.
        uiEnableDropdownListScroll(satDd);
            lv_obj_add_event_cb(satDd, satCb, LV_EVENT_VALUE_CHANGED, satUserData);
        uiFixDropdownPopup(satDd);

        lv_obj_t* filterCont = lv_obj_create(header);
        lv_obj_set_size(filterCont, scaled(224), scaled(32));
        lv_obj_set_style_bg_opa(filterCont, 0, 0);
        lv_obj_set_style_border_width(filterCont, 0, 0);
        lv_obj_set_style_pad_all(filterCont, 0, 0);
        lv_obj_set_layout(filterCont, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(filterCont, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(filterCont, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(filterCont, scaled(8), 0);
        lv_obj_clear_flag(filterCont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t* filterLbl = lv_label_create(filterCont);
        lv_obj_add_style(filterLbl, &styles.label, 0);
        lv_obj_set_style_text_color(filterLbl, COL_TEXT_DIM, 0);
        lv_obj_set_width(filterLbl, scaled(100));
        lv_label_set_long_mode(filterLbl, LV_LABEL_LONG_CLIP);
        lv_label_set_text(filterLbl, filterLabelText);

        lv_obj_t* filterDd = lv_dropdown_create(filterCont);
        lv_dropdown_set_options(filterDd, filterOptions);
        lv_obj_set_width(filterDd, scaled(112));
        lv_obj_set_height(filterDd, scaled(30));
        lv_obj_add_style(filterDd, &styles.sliderMain, 0);
        // Long option lists must scroll (sanctioned exemption); panels stay locked.
        uiEnableDropdownListScroll(filterDd);
            lv_obj_add_event_cb(filterDd, filterCb, LV_EVENT_VALUE_CHANGED, satUserData);
        uiFixDropdownPopup(filterDd);
        if (outTitle) *outTitle = title;
        if (outSatDropdown) *outSatDropdown = satDd;
        if (outFilterDropdown) *outFilterDropdown = filterDd;
    }

    // A knob row inside the dock. ROW_WRAP keeps an over-wide row from
    // computing negative inter-item gaps (LVGL justifies per wrapped line, so
    // a wrapped line always fits). justify/crossPlace let a caller fill the row
    // width instead of leaving a dead band at its right edge — LV_FLEX_ALIGN_START
    // keeps the previous left-packed look for callers that want it.
    static lv_obj_t* createControlRow(lv_obj_t* panel,
                                      lv_flex_align_t justify = LV_FLEX_ALIGN_START,
                                      lv_flex_align_t crossPlace = LV_FLEX_ALIGN_START) {
        lv_obj_t* row = lv_obj_create(panel);
        lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(row, 0, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_layout(row, LV_LAYOUT_FLEX);
        // FIX ("Freq column half off-screen"): with plain ROW + SPACE_BETWEEN,
        // content wider than the row (e.g. the Mode arcs becoming visible)
        // makes LVGL compute a NEGATIVE inter-item gap that accumulates and
        // pushes children into negative x, clipping them at the panel's left
        // second line instead of drifting off-panel.
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_flex_align(row, justify, crossPlace, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_gap(row, scaled(6), 0);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        return row;
    }

    // FIX (dock width overflow): full-width END-aligned action row hosting
    // the SOLO/ON/PHASE stack. The stack used to be a child of the arcs
    // control row; worst-case content there (10 arc columns @64px + this
    // 76px column) exceeded the dock width whenever FLANGE/FT_PARTIAL_*
    // showed both Mode arcs, so the stack wrapped to a floating second line
    // whose position jumped with filter type. A dedicated row below keeps
    // placement stable; definite content-sized height encloses its buttons.
    static lv_obj_t* createActionRow(lv_obj_t* panel) {
        lv_obj_t* row = lv_obj_create(panel);
        lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(row, 0, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_layout(row, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        return row;
    }

    static lv_obj_t* createArc(lv_obj_t* parent,
                               UIStyles& styles,
                               const char* name,
                               float init,
                               int which,
                               lv_event_cb_t cb,
                               void* userData) {
        // Uniform knob size (user call): match the rail's 44px spec so the
        // COMBS/FILTER dock knobs read identically to IMPULSES. Columns
        // size to the full mod-ring diameter (arcSize + 28) as before.
        UIWidgets::ArcVisualSpec compactSpec = UIWidgets::smallArcVisualSpec();
        compactSpec.arcSize = scaled(44);
        compactSpec.containerW = scaled(68);
        compactSpec.containerH = scaled(104);
        const lv_coord_t ringPad = scaled(28);
        const lv_coord_t knobSize = (lv_coord_t)(compactSpec.arcSize + ringPad);
        const lv_coord_t contW = std::max(compactSpec.containerW, knobSize);
        // Name + knob + value ONLY. This used to add ringPad on top of
        // containerH, but the kit's ArcVisualSpec already budgets the 28px
        // ring allowance inside containerH (and knobCont below is sized to
        // the full knobSize ring square). The extra +28 therefore became dead
        // space at the BOTTOM of every arc column (LV_FLEX_ALIGN_START), i.e.
        // 28px per row of dead height — the wide gap between the two knob rows.
        const lv_coord_t contH = (lv_coord_t)compactSpec.containerH;

        lv_obj_t* cont = lv_obj_create(parent);
        lv_obj_set_size(cont, contW, contH);
        lv_obj_set_style_bg_opa(cont, 0, 0);
        lv_obj_set_style_border_width(cont, 0, 0);
        lv_obj_set_style_pad_all(cont, 0, 0);
        lv_obj_set_style_pad_row(cont, 1, 0);
        lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* nameLbl = lv_label_create(cont);
        lv_obj_add_style(nameLbl, &styles.label, 0);
        lv_obj_set_style_text_font(nameLbl, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(nameLbl, COL_TEXT_DIM, 0);
        lv_obj_set_width(nameLbl, contW);
        lv_obj_set_style_text_align(nameLbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_long_mode(nameLbl, LV_LABEL_LONG_CLIP);
        lv_label_set_text(nameLbl, name);
        lv_obj_set_style_margin_bottom(nameLbl, compactSpec.labelMarginBottom, 0);

        // Use shared knob builder for arc
        lv_obj_t* arc = UIWidgets::createSimpleArcWithDrag(cont, compactSpec.arcSize, init, styles,
                                                           arcDragCb, nullptr);
        // Ring containment exactly like the kit's createArcEx: enlarge the
        // returned arc's parent container to the full ring square and re-center
        // the arc inside it, so the ring no longer paints outside its own
        // container (and thus never escapes the dock row/panel).
        {
            lv_obj_t* knobCont = lv_obj_get_parent(arc);
            lv_obj_set_size(knobCont, knobSize, knobSize);
            lv_obj_center(arc); // style ALIGN_CENTER persists through the resize
        }

        lv_obj_t* valLbl = lv_label_create(cont);
        lv_obj_add_style(valLbl, &styles.label, 0);
        lv_obj_set_style_text_font(valLbl, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(valLbl, COL_TEXT, 0);
        lv_obj_set_style_margin_top(valLbl, compactSpec.valueMarginTop, 0);
        lv_obj_set_width(valLbl, contW);
        lv_obj_set_style_text_align(valLbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_long_mode(valLbl, LV_LABEL_LONG_CLIP);
        // Fix A.2: `which` (0..14) is a LOCAL payload that collides with the
        // param-index range — leave user_data unset (raw 0, rejected by
        // repoint's `raw > 0` guard) so a repoint pass can never claim
        // widgets[which] = this arc before registration. The which-index
        // reader is arcWhichMap() below; mod-target registration overwrites
        // user_data with the real param index afterwards.
        lv_obj_set_user_data(arc, nullptr);
        arcWhichMap()[arc] = which;
        setArcNormalizedValue(arc, init);

            lv_obj_add_event_cb(arc, cb, LV_EVENT_VALUE_CHANGED, userData);
        return arc;
    }

    // which-index lookup by arc (user_data starts UNSET — Fix A.2 — and is
    // repurposed for the param index by the mod-target registration):
    // parallels dragStates() lifecycle.
    static std::unordered_map<lv_obj_t*, int>& arcWhichMap() {
        static std::unordered_map<lv_obj_t*, int> m;
        return m;
    }

    // A second-row arc: knob FIRST, then the name, then the value. The
    // default column order is name -> knob -> value, which on the dock's
    // SECOND row puts the name directly under the first row's value - the
    // two bands read as one dense block. Putting the name under the knob
    // separates the rows: row 1 reads name/knob/value, row 2 reads
    // knob/name/value with its name in the gap between the rows.
    static lv_obj_t* createArcBelow(lv_obj_t* parent,
                                    UIStyles& styles,
                                    const char* name,
                                    float init,
                                    int which,
                                    lv_event_cb_t cb,
                                    void* userData) {
        lv_obj_t* arc = createArc(parent, styles, name, init, which, cb, userData);
        if (!arc) return arc;
        // The column is name -> knob -> value; move the name after the knob.
        lv_obj_t* cont = lv_obj_get_parent(arc);          // the knob container
        if (cont == nullptr) return arc;
        lv_obj_t* col = lv_obj_get_parent(cont);           // the column
        if (col == nullptr) return arc;
        lv_obj_t* nameLbl = lv_obj_get_child(col, 0);
        if (nameLbl == nullptr) return arc;
        lv_obj_t* valLbl = lv_obj_get_child(col, 2);
        if (valLbl == nullptr) return arc;
        // Re-insert the name between the knob container and the value label.
        lv_obj_move_to_index(nameLbl, 1);
        return arc;
    }

    static void setArcNormalizedValue(lv_obj_t* arc, float normalized) {
        if (!arc) return;
        // setArcValueInstant (not ...AndSyncVisual) also rotates the pointer:
        // plain lv_arc_set_value only moves the arc angles — LVGL never emits
        // VALUE_CHANGED on programmatic sets, so the pointer stayed frozen
        // while the fill updated.
        //
        // INSTANT, not animated, because this runs per pointer move: the XY pad
        // fires LV_EVENT_VALUE_CHANGED on every move (Stage3Tab.hpp:656 ->
        // xySelectionSyncCb -> refreshEditorFromSelection -> here, ~13 arcs).
        // With the animated setter each move restarted a 200ms sweep on all 13
        // knobs, so they chased the finger and never settled.
        UIWidgets::setArcValueInstant(arc, (int)(normalized * 1000.0f));
        // arc is a child of knobCont, which is a child of the outer cont (nameLbl + knobCont + valLbl).
        lv_obj_t* knobCont = lv_obj_get_parent(arc);
        lv_obj_t* cont = knobCont ? lv_obj_get_parent(knobCont) : nullptr;
        lv_obj_t* valLbl = (cont && lv_obj_get_child_cnt(cont) >= 3) ? lv_obj_get_child(cont, 2) : nullptr;
        if (!valLbl) return;
        char buf[24];
        // which: 0=LPF(Hz), 1=HPF(Hz), 2=Scale(%), 3=Mix(%), 4=SatAmt(%), 5=StressOnset(%),
        // 6=StressKnee(%), 7=ModeRate(Hz), 8=ModeAmt(%), 9=RelPos(%), 10=FreqDamp(dB-decade),
        // 11=Stiffness(10^x), 12=StiffExp(exponent), 13=Partials(count), 14=Freq(Hz),
        // 15=FB(feedback)
        const auto it = arcWhichMap().find(arc);
        const int which = (it != arcWhichMap().end()) ? it->second : -1;
        if (which == 0 || which == 1 || which == 14) {
            // Cutoff: map normalized 0-1 → 20 Hz - 20 kHz logarithmic
            float hz = DSPUtils::normToHz( normalized);
            if (hz >= 9950.0f)      snprintf(buf, sizeof(buf), "%.0f kHz", hz / 1000.0f);
            else if (hz >= 995.0f)  snprintf(buf, sizeof(buf), "%.1f kHz", hz / 1000.0f);
            else if (hz >= 100.0f)  snprintf(buf, sizeof(buf), "%.0f Hz", hz);
            else                    snprintf(buf, sizeof(buf), "%.1f Hz", hz);
        } else if (which == 7) {
            // Mode rate: normalized 0-1 → 0.05-8 Hz logarithmic
            float hz = 0.05f * std::pow(160.0f, normalized);
            if (hz >= 9.95f)        snprintf(buf, sizeof(buf), "%.1f Hz", hz);
            else if (hz >= 0.995f)  snprintf(buf, sizeof(buf), "%.2f Hz", hz);
            else                    snprintf(buf, sizeof(buf), "%.3f Hz", hz);
        } else if (which == 9) {
            // Relative position: 0.0001..0.999
            snprintf(buf, sizeof(buf), "%.0f%%", normalized * 100.0f);
        } else if (which == 10) {
            // Freq damp: -6..-1 (10^x), display the exponent
            snprintf(buf, sizeof(buf), "%.2f", -6.0f + normalized * 5.0f);
        } else if (which == 11) {
            // Stiffness: 2..6 (10^x)
            snprintf(buf, sizeof(buf), "%.1f", 2.0f + normalized * 4.0f);
        } else if (which == 12) {
            // Stiffness exponent: 1..3
            snprintf(buf, sizeof(buf), "%.2f", 1.0f + normalized * 2.0f);
        } else if (which == 13) {
            // Partials count: 16..64
            snprintf(buf, sizeof(buf), "%.0f", 16.0f + normalized * 48.0f);
        } else if (which == 15) {
            // Feedback: 0..0.998, and the comb's default is 0.985 - so a plain
            // percent would read "98%" and look pinned at the top of its
            // travel. Show the RESONANCE the feedback produces instead: a
            // single-tap comb with feedback r resonates with Q = 1/(1-r), and
            // Q is what the ear reads as ring. The same quantity the DSP
            // derives (q = 0.7 + fb*4.3 for the resonant types) is what the
            // knob is really setting.
            const float r = std::clamp(normalized, 0.0f, 0.998f);
            const float q = 1.0f / std::max(1.0f - r, 1.0e-3f);
            if (q >= 10.0f) snprintf(buf, sizeof(buf), "Q %.0f", q);
            else            snprintf(buf, sizeof(buf), "Q %.1f", q);
        } else {
            snprintf(buf, sizeof(buf), "%.0f%%", normalized * 100.0f);
        }
        lv_label_set_text(valLbl, buf);
    }
};

END_NAMESPACE_DISTRHO

#endif // COMB_EDITOR_DOCK_HPP
