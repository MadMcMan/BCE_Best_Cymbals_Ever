#pragma once
// Local wrapper — canonical implementation lives in cymbals-ui.
// See deps/cymbals-ui/include/cymbals-ui/UIWidgets.hpp
#include "cymbals-ui/UIWidgets.hpp"
#include <cstring>

// Footer spectrum analyzer fit: the kit's dB-grid furniture is positioned
// for a ~100px box (grid lines at y=25/50/75), but the footer gives the
// analyzer ~63px, so the bottom grid line spills past the container (audit
// HARD clip, and a lost grid line on screen). Re-seat the three grid lines
// at quarters of the ACTUAL content height; the baseline glow stays where the
// kit put it. Kit-private-safe: furniture lines are matched structurally
// (plain lv_obj + IGNORE_LAYOUT + 1px tall), never by child index.
inline void uiFitAnalyzerFurniture(lv_obj_t* analyzer) {
    if (analyzer == nullptr || !lv_obj_is_valid(analyzer)) return;
    lv_obj_update_layout(analyzer);
    const int32_t H = lv_obj_get_content_height(analyzer);
    if (H <= 0) return;
    const lv_obj_class_t* baseCls = lv_obj_get_class(analyzer);
    lv_obj_t* grid[3] = {nullptr, nullptr, nullptr};
    int n = 0;
    const uint32_t cnt = lv_obj_get_child_count(analyzer);
    for (uint32_t i = 0; i < cnt && n < 3; ++i) {
        lv_obj_t* c = lv_obj_get_child(analyzer, i);
        if (c == nullptr || !lv_obj_is_valid(c)) continue;
        if (!lv_obj_has_flag(c, LV_OBJ_FLAG_IGNORE_LAYOUT)) continue;
        if (lv_obj_get_height(c) != 1) continue;
        if (lv_obj_get_class(c) != baseCls) continue;
        grid[n++] = c;
    }
    if (n < 3) return;
    for (int k = 0; k < 3; ++k)
        lv_obj_align(grid[k], LV_ALIGN_TOP_MID, 0, (H * (k + 1)) / 4);
}


// Dropdown popups at zoom != 1.0 land misaligned: lv_dropdown re-parents its
// list to the SCREEN on open (lv_dropdown.c), outside the fit transform on
// the design-canvas wrapper, so LVGL positions/sizes the popup in unscaled
// canvas coords. Re-host each opening list on the transformed wrapper instead:
// LVGL's own align math then runs in design space and the fit transform scales
// the popup onto its anchor for free. (One bubbled LV_EVENT_READY catch on the
// wrapper cannot work: event bubbling defaults OFF in LVGL, so every dropdown
// carries its own READY handler — one uiFixDropdownPopup() call per creation
// site.) With no zoom transform the handler is a no-op (stock LVGL path).
inline lv_obj_t*& uiOpenDropdownListSlot();
inline void uiFixDropdownPopupCb(lv_event_t* e) {
    if (lv_event_get_code(e) != LV_EVENT_READY) return;
    lv_obj_t* target = static_cast<lv_obj_t*>(lv_event_get_target(e));
    if (target == nullptr) return;
    const lv_obj_class_t* cl = lv_obj_get_class(target);
    if (cl == nullptr || cl->name == nullptr || std::strcmp(cl->name, "lv_dropdown") != 0) return;
    lv_obj_t* list = lv_dropdown_get_list(target);
    if (list == nullptr || !lv_obj_is_valid(list)) return;
    // Mousewheel scroll: the LVGL encoder mousewheel indev never applies its
    // deltas to the popup list, so the widget's onScroll routes wheel deltas
    // directly to it. Register the just-opened list (hidden = closed) and cap
    // the popup to a scrollable viewport so long option sets scroll instead of
    // running off-screen.
    uiOpenDropdownListSlot() = list;
    lv_obj_t* scr = lv_obj_get_screen(target);
    if (scr == nullptr) return;
    lv_obj_t* top = target;
    while (top != nullptr && lv_obj_get_parent(top) != scr) top = lv_obj_get_parent(top);
    if (top == nullptr || top == target) return;
    if (lv_obj_get_style_transform_scale_x(top, LV_PART_MAIN) == LV_SCALE_NONE &&
        lv_obj_get_style_transform_scale_y(top, LV_PART_MAIN) == LV_SCALE_NONE) return;
    if (lv_obj_get_parent(list) != top) {
        lv_obj_set_parent(list, top);
        lv_obj_move_to_index(list, -1);
    }
    // Scrollable viewport: never let the popup claim more than ~45% of the
    // design height — beyond that it scrolls (the sanctioned surface).
    const lv_coord_t designH = scaled(640);
    if (lv_obj_get_height(list) > designH) {
        lv_obj_set_height(list, designH);
        lv_obj_set_scroll_dir(list, LV_DIR_VER);
        lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);
    UIWidgets::attachScrollbarDrag(list);
        UIWidgets::attachScrollbarDrag(list);
    }
}
// The dropdown popup list currently open (for mousewheel scrolling). Callers:
// uiFixDropdownPopupCb sets it on open; the widget's onScroll scrolls it when
// visible; a hidden list means the popup closed. LVGL shows one popup at a time.
inline lv_obj_t*& uiOpenDropdownListSlot() {
    static lv_obj_t* s = nullptr;
    return s;
}
// The plugin-picker ("SELECT PLUGIN") list currently open, for the same reason
// the dropdown slot exists: DPF's mousewheel is an LVGL ENCODER indev, and an
// encoder never scrolls an arbitrary list on its own. Every list the user is
// allowed to wheel must therefore be registered here and driven by onScroll.
// Cleared when the picker closes (the list dies with the modal, so the slot is
// validity-checked at use).
inline lv_obj_t*& uiOpenPickerListSlot() {
    static lv_obj_t* s = nullptr;
    return s;
}
// Programmatic scroll for the OPEN dropdown popup list only (the sanctioned
// scroll surface). Lives with the other dropdown sanction (helper pattern in
// UIWidgets.hpp, outside the no-scroll hard scope) so call sites only name
// it; this is what the widget's onScroll drives with the DPF wheel delta.
inline void uiDropdownScrollBy(lv_obj_t* list, int dy_px) {
    if (list == nullptr || !lv_obj_is_valid(list)) return;
    if (!lv_obj_is_visible(list)) return;
    // Bounded: clamps to content extent so the wheel stops at the last option
    // instead of scrolling past it. lv_obj_scroll_by() is unbounded, so an
    // over-scroll leaves dead space (and can push options out of the clip).
    lv_obj_scroll_by_bounded(list, 0, dy_px, LV_ANIM_OFF);
}
// Kill a mod ring's moving dot. The ring is an lv_arc whose live dot is the
// arc's LV_PART_KNOB part, moved per tick by lv_arc_set_value() in
// PluginUI::updateModRings() so it tracks base+delta inside the excursion
// line. The ring is the intended "solid line only" rendering everywhere, so
// blank the KNOB part and let those per-tick updates keep running invisibly.
//
// WHY style and not geometry: LVGL 9 sizes the arc knob rect in
// get_knob_area() (lv_arc.c:995-1017) from arc_width(LV_PART_INDICATOR) plus
// LV_PART_KNOB pads — it NEVER reads the KNOB width/height styles the kit
// sets, so `width/height = 6px` is inert. The ring leaves arc_width
// (INDICATOR) unset (its stroke is TRANSP anyway) and LV_STYLE_ARC_WIDTH is
// not LV_STYLE_PROP_FLAG_INHERITABLE (lv_style.c:111), so there is no MAIN
// fallback: the value falls through to the auto-applied default theme, whose
// arc_indic width is LV_DPX_CALC(dpi, 15) (lv_theme_default.c:399, applied
// to every arc INDICATOR at :1004-1007). Unpacked knobs therefore drew a
// ~15px blob instead of a 6px dot — the "giant dot" riding the arc on every
// screen except the packed rail/EQ knobs, which already called this out via
// the inline pair this helper replaces.
//
// KNOB opa+border is also what suppresses the draw itself: get_knob_inv_area
// (lv_arc.c:920-935) early-outs on TRANSP bg with no border/outline/shadow,
// and lv_draw_rect paints nothing.
inline void uiKillModRingDot(lv_obj_t* ring) {
    if (ring == nullptr || !lv_obj_is_valid(ring)) return;
    lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_border_width(ring, 0, LV_PART_KNOB);
}

// Knob pack: reclaim knobCont's internal ring slack with spacing universal
// across knob sizes, and restyle the mod ring in the same pass. createArcEx
// centers an arcSize arc in an arcSize+28 box (14px dead air top AND bottom);
// card columns build an arc-exact box with no slack. TOP_MID align + explicit
// y offsets (later layouts keep them; bare set_pos does not survive layout —
// pinned by test), with ALL offsets from arcSizePx + kit constexpr insets:
// fully layout-independent, so it applies mid-build when every coord still
// reads 0. Ring (childless) resized to arcSize+6, centered on the arc, KNOB
// dot made transparent+borderless (per-tick updates keep running invisibly;
// the solid full-270 MAIN line is untouched).
inline void uiPackKnob(lv_obj_t* arc, lv_coord_t arcSizePx, lv_coord_t arcTopPx, lv_coord_t kcHPx) {
    if (arc == nullptr || !lv_obj_is_valid(arc)) return;
    lv_obj_t* kc = lv_obj_get_parent(arc);
    if (kc == nullptr || !lv_obj_is_valid(kc)) return;
    if (arcSizePx <= 0 || kcHPx <= 0) return;
    const int inset = (arcSizePx < scaled(40)) ? UIStyles::compactKnobGeometry().capInset
                                              : UIStyles::normalKnobGeometry().capInset;
    const lv_coord_t capY = (lv_coord_t)(arcTopPx + inset / 2);
    const lv_coord_t ringSize = (lv_coord_t)(arcSizePx + 6);
    const lv_coord_t ringY = (lv_coord_t)(arcTopPx + (arcSizePx - ringSize) / 2);
    for (uint32_t i = 0; i < lv_obj_get_child_count(kc); ++i) {
        lv_obj_t* ch = lv_obj_get_child(kc, i);
        if (ch == nullptr || !lv_obj_is_valid(ch)) continue;
        lv_obj_set_align(ch, LV_ALIGN_TOP_MID);
        if (ch == arc) {
            lv_obj_set_y(ch, arcTopPx);
        } else if (lv_obj_get_child_count(ch) > 0) {
            lv_obj_set_y(ch, capY);
        } else {
            lv_obj_set_y(ch, ringY);
            lv_obj_set_size(ch, ringSize, ringSize);
            uiKillModRingDot(ch);
        }
    }
    lv_obj_set_height(kc, kcHPx);
}
inline void uiFixDropdownPopup(lv_obj_t* dd) {
    if (dd == nullptr) return;
    lv_obj_add_event_cb(dd, uiFixDropdownPopupCb, LV_EVENT_READY, nullptr);
}
// Dropdown popup lists are the ONE sanctioned scroll surface in the UI:
// option lists (Airwindows 500+, targets 2000+) physically cannot fit any
// viewport, so the list scrolls vertically with an auto scrollbar while
// every panel/tab/card stays locked (see test_ui_no_scroll.py). The LVGL
// default already allows it; this only guarantees the state explicitly so
// future uiLockScroll-style sweeps cannot silently re-lock a list. Call
// once per dropdown right after creation/styling.
inline void uiEnableDropdownListScroll(lv_obj_t* dd) {
    if (dd == nullptr) return;
    lv_obj_t* list = lv_dropdown_get_list(dd);
    if (list == nullptr) return;
    lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);
}

// Long-press tooltips for icon-only / cryptic controls (no global help
// system exists). Text must be a string literal (stored, never freed).
// One tip at a time; auto-hides after 2.5s, on release, or when the owner
// is deleted. The trailing click after a long-press is swallowed via
// lv_indev_wait_release, so tips never toggle/delete/zoom by accident.
// Positioned from the owner's TRANSFORMED area: correct at any zoom.
namespace {
struct UiTipState { lv_obj_t* tip = nullptr; lv_timer_t* hide = nullptr; };
inline UiTipState& uiTipState() { static UiTipState s; return s; }
inline void uiTipHide(lv_timer_t* t = nullptr) {
    UiTipState& s = uiTipState();
    if (s.hide) { lv_timer_del(s.hide); s.hide = nullptr; }
    if (s.tip && lv_obj_is_valid(s.tip)) lv_obj_del(s.tip);
    s.tip = nullptr;
    (void)t;
}
inline void uiTipShow(lv_obj_t* target, const char* text, bool swallowClick) {
    if (!text || !target || !lv_obj_is_valid(target)) return;
    uiTipHide();
    UiTipState& s = uiTipState();
    // Parent to the layer of the display that owns the target, not the
    // process-global lv_layer_top(): with several editors alive the tip would
    // otherwise be created on another editor's display.
    lv_display_t* tipDisp = lv_obj_get_display(target);
    lv_obj_t* tip = lv_label_create(tipDisp ? lv_display_get_layer_top(tipDisp) : lv_layer_top());
    lv_label_set_text(tip, text);
    lv_obj_set_style_text_font(tip, getScaledSmallFont(), 0);
    lv_obj_set_style_text_color(tip, COL_TEXT, 0);
    lv_obj_set_style_bg_color(tip, COL_PANEL_DARK, 0);
    lv_obj_set_style_bg_opa(tip, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(tip, COL_ACCENT, 0);
    lv_obj_set_style_border_width(tip, 1, 0);
    lv_obj_set_style_radius(tip, scaled(4), 0);
    lv_obj_set_style_pad_hor(tip, scaled(8), 0);
    lv_obj_set_style_pad_ver(tip, scaled(6), 0);
    lv_obj_set_width(tip, scaled(240));
    lv_label_set_long_mode(tip, LV_LABEL_LONG_WRAP);
    lv_obj_clear_flag(tip, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(tip, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(tip, LV_SCROLLBAR_MODE_OFF);
    lv_obj_update_layout(tip);
    lv_area_t a;
    lv_obj_get_coords(target, &a);
    lv_obj_get_transformed_area(target, &a, LV_OBJ_POINT_TRANSFORM_FLAG_RECURSIVE);
    lv_display_t* disp = lv_obj_get_display(target);
    const int32_t scrW = disp ? (int32_t)lv_display_get_horizontal_resolution(disp) : 1280;
    const int32_t tw = lv_obj_get_width(tip);
    const int32_t th = lv_obj_get_height(tip);
    int32_t tx = a.x1;
    if (tx + tw > scrW) tx = scrW - tw - 4;
    if (tx < 0) tx = 0;
    int32_t ty = a.y1 - th - 6;
    if (ty < 0) ty = a.y2 + 6;
    lv_obj_set_pos(tip, tx, ty);
    // Scale the tip with the window fit (same class as the dropdown-popup
    // bug): layer_top is unscaled screen space. Identity at 1.0, pivot
    // top-left so the tip grows down-right from the anchor.
    lv_obj_t* scr = lv_obj_get_screen(target);
    lv_obj_t* top = target;
    while (top && lv_obj_get_parent(top) != scr) top = lv_obj_get_parent(top);
    int32_t qx = 256, qy = 256;
    if (top && top != target) {
        qx = lv_obj_get_style_transform_scale_x(top, LV_PART_MAIN);
        qy = lv_obj_get_style_transform_scale_y(top, LV_PART_MAIN);
        if (qx <= 0) qx = 256;
        if (qy <= 0) qy = 256;
    }
    lv_obj_set_style_transform_scale_x(tip, qx, 0);
    lv_obj_set_style_transform_scale_y(tip, qy, 0);
    lv_obj_set_style_transform_pivot_x(tip, 0, 0);
    lv_obj_set_style_transform_pivot_y(tip, 0, 0);
    s.tip = tip;
    s.hide = lv_timer_create(uiTipHide, 2500, nullptr);
    if (s.hide) lv_timer_set_repeat_count(s.hide, 1);
    // Swallow the trailing click so a long-press never toggles, deletes or
    // zooms by accident. Hold-to-drag owners pass swallowClick=false: their
    // PRESSING stream must survive the long-press or slow drags die.
    if (swallowClick) {
        lv_indev_t* ind = lv_indev_active();
        if (ind) lv_indev_wait_release(ind);
    }
}
inline void uiTipDispatch(lv_event_t* e, bool swallowClick) {
    const lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_LONG_PRESSED) {
        uiTipShow(static_cast<lv_obj_t*>(lv_event_get_target(e)),
                  static_cast<const char*>(lv_event_get_user_data(e)),
                  swallowClick);
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST ||
               code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_DELETE) {
        uiTipHide();
    }
}
inline void uiTipEvent(lv_event_t* e) { uiTipDispatch(e, true); }
inline void uiTipEventHoldable(lv_event_t* e) { uiTipDispatch(e, false); }
}  // namespace
inline void uiAttachTooltip(lv_obj_t* obj, const char* text) {
    if (!obj || !text) return;
    lv_obj_add_event_cb(obj, uiTipEvent, LV_EVENT_ALL, (void*)text);
}
inline void uiAttachTooltipHoldable(lv_obj_t* obj, const char* text) {
    if (!obj || !text) return;
    lv_obj_add_event_cb(obj, uiTipEventHoldable, LV_EVENT_ALL, (void*)text);
}
// Micro tier with a REAL distinction (kit getScaledMicroFont == small == 12):
// chips/hints step one size below the small tier at every zoom.
inline const lv_font_t* uiMicroFont() {
    if (gUIScale >= 1.5f) return &lv_font_montserrat_14;
    if (gUIScale >= 1.2f) return &lv_font_montserrat_12;
    return &lv_font_montserrat_10;
}
