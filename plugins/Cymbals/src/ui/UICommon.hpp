#pragma once
// Shim — UICommon now lives in deps/cymbals-ui
#include "cymbals-ui/UICommon.hpp"
#include <cstdint>

// Footer (keyboard + spectrum) height in design pixels. The floating filter
// docks are parented to the same content root, so their placement must reserve
// this strip: the controls belong above the keyboard/spectrum, never over it.
inline constexpr int kUIFooterHeight = 84;
inline lv_coord_t uiFooterHeight() { return scaled(kUIFooterHeight); }

// ============================================================================
// Plugin-local palette additions (shared cymbals-ui repo is read-only).
// All lv_color_hex(...) definitions stay macros here, per repo convention.
// ============================================================================

// XY-pad disabled comb rings: selected vs unselected.
#define COL_RING_DISABLED      lv_color_hex(0x6F747B)
#define COL_RING_DISABLED_DIM  lv_color_hex(0x4A4E54)

// ============================================================================
// USER RULE: nothing in this UI ever scrolls — not tabs, not lists, not even
// FX CHAIN. Every container created anywhere under plugins/Cymbals/src goes
// through this immediately after creation (lv_obj_create defaults to
// SCROLLABLE + auto scrollbar in LVGL 9). Content must instead FIT its panel;
// tests/test_ui_no_scroll.py enforces the source-level invariant.
// ============================================================================
inline void uiLockScroll(lv_obj_t* obj) {
    if (!obj) return;
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
}

// ============================================================================
// Text input vs the PROCESS-GLOBAL LVGL default group.
//
// lv_textarea is group-def, so LVGL adds it to lv_group_get_default() when it
// is created. That default is one group for the whole PROCESS: it belongs to
// whichever editor was constructed last, and an editor closing resets it to
// NULL (lv_group_delete clears it). In a host that keeps several plugin
// editors alive (Bitwig does), a textarea can therefore join ANOTHER editor's
// group, or no group at all. Clicking it still focuses it in that group, but
// keystrokes are delivered to the keypad indev's own group, so typing does
// nothing.
//
// Bind every textarea to the KEYPAD indev of its OWN display at creation; the
// indev/display pair is per editor, so this cannot cross instances.
// ============================================================================
inline lv_group_t* uiKeypadGroupForOwnDisplay(lv_obj_t* obj) {
    if (obj == nullptr) return nullptr;
    lv_display_t* const disp = lv_obj_get_display(obj);
    if (disp == nullptr) return nullptr;
    for (lv_indev_t* indev = lv_indev_get_next(nullptr); indev != nullptr;
         indev = lv_indev_get_next(indev)) {
        if (lv_indev_get_type(indev) != LV_INDEV_TYPE_KEYPAD) continue;
        if (lv_indev_get_display(indev) != disp) continue;
        return lv_indev_get_group(indev);
    }
    return nullptr;
}

// Small dice glyph built from real LVGL objects (no icon font dependency):
// a rounded square with five pips. Used for every "randomize" button.
inline lv_obj_t* uiDiceIcon(lv_obj_t* parent, lv_color_t face = COL_TEXT) {
    if (parent == nullptr) return nullptr;
    lv_obj_t* die = lv_obj_create(parent);
    lv_obj_remove_style_all(die);
    lv_obj_set_size(die, scaled(14), scaled(14));
    lv_obj_set_style_radius(die, scaled(3), 0);
    lv_obj_set_style_bg_color(die, face, 0);
    lv_obj_set_style_bg_opa(die, LV_OPA_COVER, 0);
    lv_obj_clear_flag(die, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(die, LV_OBJ_FLAG_CLICKABLE);
    static const int kPips[5][2] = {{3,3},{10,3},{7,7},{3,10},{10,10}};
    for (const auto& pip : kPips) {
        lv_obj_t* dot = lv_obj_create(die);
        lv_obj_remove_style_all(dot);
        lv_obj_set_size(dot, scaled(2), scaled(2));
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(dot, COL_PANEL_DARK, 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        lv_obj_set_pos(dot, scaled(pip[0]), scaled(pip[1]));
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_CLICKABLE);
    }
    return die;
}

// Small cymbal glyph for the structural "randomize filter count" button:
// shallow oval plate, centre bell and stand. Built from real LVGL objects so
// it needs no icon font.
inline lv_obj_t* uiCymbalIcon(lv_obj_t* parent, lv_color_t face = COL_TEXT) {
    if (parent == nullptr) return nullptr;
    lv_obj_t* root = lv_obj_create(parent);
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, scaled(14), scaled(14));
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t* plate = lv_obj_create(root);
    lv_obj_remove_style_all(plate);
    lv_obj_set_size(plate, scaled(14), scaled(7));
    lv_obj_align(plate, LV_ALIGN_TOP_MID, 0, scaled(1));
    lv_obj_set_style_radius(plate, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(plate, face, 0);
    lv_obj_set_style_bg_opa(plate, LV_OPA_COVER, 0);
    lv_obj_clear_flag(plate, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(plate, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t* bell = lv_obj_create(root);
    lv_obj_remove_style_all(bell);
    lv_obj_set_size(bell, scaled(4), scaled(4));
    lv_obj_align(bell, LV_ALIGN_TOP_MID, 0, scaled(2));
    lv_obj_set_style_radius(bell, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(bell, COL_PANEL_DARK, 0);
    lv_obj_set_style_bg_opa(bell, LV_OPA_COVER, 0);
    lv_obj_clear_flag(bell, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(bell, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t* stand = lv_obj_create(root);
    lv_obj_remove_style_all(stand);
    lv_obj_set_size(stand, scaled(2), scaled(5));
    lv_obj_align(stand, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(stand, face, 0);
    lv_obj_set_style_bg_opa(stand, LV_OPA_COVER, 0);
    lv_obj_clear_flag(stand, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(stand, LV_OBJ_FLAG_CLICKABLE);
    return root;
}

inline void uiBindTextInputToOwnIndev(lv_obj_t* area) {
    if (area == nullptr) return;
    lv_group_t* const group = uiKeypadGroupForOwnDisplay(area);
    if (group != nullptr && lv_obj_get_group(area) != group)
        lv_group_add_obj(group, area);
    // LVGL's generic pointer click-focus can lose the race when the press is
    // consumed by a modal/parent. Every text field therefore claims keypad
    // focus itself on press/click; the group is the editor's own indev group,
    // so this cannot cross plugin instances.
    lv_obj_add_event_cb(area, [](lv_event_t* e) {
        lv_obj_t* self = static_cast<lv_obj_t*>(lv_event_get_target(e));
        if (self == nullptr) return;
        lv_group_t* g = uiKeypadGroupForOwnDisplay(self);
        if (g != nullptr && lv_group_get_focused(g) != self)
            lv_group_focus_obj(self);
    }, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(area, [](lv_event_t* e) {
        lv_obj_t* self = static_cast<lv_obj_t*>(lv_event_get_target(e));
        if (self == nullptr) return;
        lv_group_t* g = uiKeypadGroupForOwnDisplay(self);
        if (g != nullptr && lv_group_get_focused(g) != self)
            lv_group_focus_obj(self);
    }, LV_EVENT_CLICKED, nullptr);
}
// Top layer of the DISPLAY that owns `anchor`. Modals/tooltips must parent
// here, not to lv_layer_top() (which is the PROCESS-GLOBAL default display's
// layer): with two editors open, a modal could otherwise appear on the other
// editor's window or on no visible one.
inline lv_obj_t* uiLayerTopFor(lv_obj_t* anchor) {
    lv_display_t* const disp = anchor != nullptr ? lv_obj_get_display(anchor) : nullptr;
    return disp != nullptr ? lv_display_get_layer_top(disp) : lv_layer_top();
}

// Floating overlays that live on the display top layer (the comb-editor docks)
// tag their container with this user_data so stage/sub-tab switches can hide
// them; a layer child is NOT a descendant of the panel it belongs to, so it
// would otherwise stay painted over whatever screen is shown next.
// The UI is a fixed 1280x720 canvas scaled by a transform on its content
// root. Objects parented to lv_layer_top() sit OUTSIDE that transform, so at
// any zoom != 100% they render at design size (tiny at 188%). Overlays that
// must cover the header/footer should therefore be parented to the content
// root and raised above its children instead.
inline lv_obj_t* uiOverlayRootFor(lv_obj_t* anchor) {
    lv_obj_t* root = anchor;
    while (root != nullptr) {
        lv_obj_t* parent = lv_obj_get_parent(root);
        if (parent == nullptr) break;
        if (lv_obj_get_parent(parent) == nullptr) return root;  // direct child of the screen
        root = parent;
    }
    return nullptr;
}

inline constexpr intptr_t kUIFloatDockPayload = -5;

inline void uiHideFloatingDocks(lv_obj_t* anchor) {
    // Docks live on the transformed content root (see uiOverlayRootFor).
    lv_obj_t* const host = uiOverlayRootFor(anchor);
    if (host == nullptr) return;
    for (uint32_t i = 0; i < lv_obj_get_child_count(host); ++i) {
        lv_obj_t* c = lv_obj_get_child(host, i);
        if ((intptr_t)lv_obj_get_user_data(c) == kUIFloatDockPayload)
            lv_obj_add_flag(c, LV_OBJ_FLAG_HIDDEN);
    }
    // Backward-compat: older layer-top docks (if any) are hidden too.
    lv_obj_t* const layer = uiLayerTopFor(anchor);
    if (layer && layer != host) {
        for (uint32_t i = 0; i < lv_obj_get_child_count(layer); ++i) {
            lv_obj_t* c = lv_obj_get_child(layer, i);
            if ((intptr_t)lv_obj_get_user_data(c) == kUIFloatDockPayload)
                lv_obj_add_flag(c, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

// ============================================================================
// Non-parameter widget payload guard — audit fix A.2 (repoint poisoning).
//
// BUG: repointWidgetsRecursive()/syncCombWidgetsRecursive() (PluginUI.cpp)
// treat any arc/slider/dropdown/button/switch whose user_data satisfies
// `0 < raw < kParameterCount` as a parameter-bound control. Several controls
// inside repointed subtrees carry small LOCAL payloads instead: the comb
// group header's RAND button (group index), its tune buttons
// ((preset << 8) | group), the DISP macro buttons (curve type) and the nudge
// card X buttons (card index), plus a comb dock arc before registration
// (`which`, 0..14). Those payloads collide with real parameter indexes, so a
// sub-tab switch could re-register e.g. widgets[3] = "the RAND button" —
// mod rings, cables, host sync and knob hit-tests then drove the wrong
// control for the rest of the session.
//
// FIX: encode the payload above any kParameterCount value as
// (payload + 1) << 24. The +1 makes the legitimate payload -1 encode to raw
// 0, which the existing `raw > 0` guard already rejects; every other payload
// lands at >= 1<<24, far past kParameterCount (~2.3k), so the guard fails
// outright. Readers must pass the raw value through uiDecodeNonParamPayload
// to recover the original payload. Payloads must stay small — the low 24
// bits hold payload + 1.
// ============================================================================
inline constexpr intptr_t kUiNonParamPayloadShift = 24;

inline intptr_t uiEncodeNonParamPayload(int payload) {
    return ((intptr_t)(payload + 1)) << kUiNonParamPayloadShift;
}

inline int uiDecodeNonParamPayload(intptr_t raw) {
    return (int)(raw >> kUiNonParamPayloadShift) - 1;
}

// ============================================================================
// Scaled-canvas input mapping. applyContentFit() fits the fixed 1280x720
// content into the real window via an ancestor transform scale on
// fContentRoot, so raw indev points (window pixels) only line up with
// TRANSFORMED geometry. Any handler that maps a pointer position onto
// object-local coordinates must go through these instead of subtracting
// lv_obj_get_coords() directly (the class of bug the EQ band hit-test had:
// raw point minus UNSCALED coords missed every node at fit != 1).
// ============================================================================
// Window-space rect of obj on screen, with all ancestor transforms applied
// (what the raw indev point must be compared against).
inline void uiObjTransformedArea(lv_obj_t* obj, lv_area_t* out) {
    lv_obj_get_coords(obj, out);
    lv_obj_get_transformed_area(obj, out, LV_OBJ_POINT_TRANSFORM_FLAG_RECURSIVE);
}
// Inverse-map a raw indev point (window pixels) into obj-local UNSCALED
// coordinates — the space lv_obj_get_width/height(), canvas drawing, and
// plot math live in. lv_obj_transform_point(INVERSE) undoes the ancestor
// fit transform but keeps the absolute origin, so subtract the object's
// untransformed coords to land in local space.
inline lv_point_t uiObjLocalPoint(lv_obj_t* obj, const lv_point_t& p) {
    lv_point_t q = p;
    lv_obj_transform_point(obj, &q,
        (lv_obj_point_transform_flag_t)(LV_OBJ_POINT_TRANSFORM_FLAG_RECURSIVE |
                                         LV_OBJ_POINT_TRANSFORM_FLAG_INVERSE));
    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);
    q.x -= coords.x1;
    q.y -= coords.y1;
    return q;
}
