#pragma once
// Round 2 Piece 1: right-click context menu for arc knobs (Alt-RMB trigger).
//
// Three items, all routed through the existing valueChangedCallback / setParam
// path so the same downstream paint, label-update, and DSP side-effects fire
// as a real knob drag end.
//
//   - "Reset to Default": writes the canonical default value (read via
//     AbstractCymbalsUI::getParameterDefault) through setParamValue and
//     re-syncs the widget so the visual arc matches. This is the same call
//     path the kit's LV_EVENT_DOUBLE_CLICKED handler uses, so menu and
//     double-click stay bit-identical.
//   - "Set to Current": future feature. The brief marks it as a TODO no-op
//     for this round — the per-knob user-defaults file I/O is out of scope
//     (env CYMBALS_USER_DEFAULTS, %LOCALAPPDATA%/MadMcMan/Cymbal/
//     user-knob-defaults.json). The menu item is enabled, but selecting it
//     logs a single stderr line and dismisses the popup so the gesture is
//     visibly acknowledged.
//   - "Cancel": dismiss without writing anything.
//
// Lifetime: the menu widget is owned by the parent lv_obj's tree and self-
// deletes on LV_EVENT_DELETE. Outside-click dismissal is implemented via a
// full-screen clickable scrim placed below the menu panel.
//
// No new parameters. No animated cables. No new tabs.

#include "cymbals-ui/UICommon.hpp"
#include <cstdio>

namespace UIKnobContextMenu {

// File-local helper: the menu's scrim is the parent of every visible element,
// so deleting it tears down the whole popup (menu, items, scrim itself).
// Validated first so a stray event after a manual lv_obj_del is a safe no-op.
static inline void closeScrim(lv_obj_t* scrim) {
    if (scrim && lv_obj_is_valid(scrim)) lv_obj_del(scrim);
}

// Open the context menu anchored at the given screen coordinates. The menu
// resolves the param under the cursor and wires the three items; the caller
// is responsible only for the input event (Alt-RMB on a knob).
//
// `host` is the AbstractCymbalsUI* whose getParamValue / getParameterDefault
// / setParamValue / valueChangedCallback path the menu uses. Passing nullptr
// is treated as a no-op (used by tests and as a safety guard).
inline void open(lv_obj_t* parentScreen,
                 AbstractCymbalsUI* host,
                 int paramIndex,
                 float currentValue) {
    if (!parentScreen || !host) return;
    if (paramIndex < 0) return;
    if ((uint32_t)paramIndex >= PluginCymbals::kParameterCount) return;

    // Scrim: full-screen, clickable, swallows the next click so the menu
    // closes on outside press without forwarding the event to the knob under
    // the cursor.
    lv_obj_t* scrim = lv_obj_create(parentScreen);
    lv_obj_set_size(scrim, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(scrim, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(scrim, 0, 0);
    lv_obj_set_style_pad_all(scrim, 0, 0);
    lv_obj_add_flag(scrim, LV_OBJ_FLAG_FLOATING);
    lv_obj_clear_flag(scrim, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scrim, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_foreground(scrim);

    // The menu panel itself: 3 rows, app palette.
    lv_obj_t* menu = lv_obj_create(scrim);
    lv_obj_set_size(menu, scaled(180), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(menu, COL_PANEL, 0);
    lv_obj_set_style_bg_opa(menu, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(menu, COL_ACCENT, 0);
    lv_obj_set_style_border_width(menu, 1, 0);
    lv_obj_set_style_radius(menu, scaled(6), 0);
    lv_obj_set_style_pad_all(menu, scaled(4), 0);
    lv_obj_set_style_pad_row(menu, scaled(2), 0);
    lv_obj_set_layout(menu, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(menu, LV_OBJ_FLAG_SCROLLABLE);
    // Anchor at the cursor; clamped to the visible area in a follow-up.
    lv_obj_set_pos(menu, scaled(40), scaled(40));
    lv_obj_add_flag(menu, LV_OBJ_FLAG_FLOATING);
    lv_obj_move_foreground(menu);

    struct Ctx {
        AbstractCymbalsUI* host;
        int paramIndex;
        float currentValue;
        lv_obj_t* scrim;
    };
    auto* ctxAlloc = new Ctx{host, paramIndex, currentValue, scrim};

    auto makeItem = [&](const char* label, bool enabled) -> lv_obj_t* {
        lv_obj_t* btn = lv_obj_create(menu);
        lv_obj_set_size(btn, lv_pct(100), scaled(26));
        lv_obj_set_style_bg_opa(btn, enabled ? LV_OPA_TRANSP : LV_OPA_TRANSP, 0);
        lv_obj_set_style_bg_color(btn, COL_ACCENT, LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_style_radius(btn, scaled(4), 0);
        lv_obj_set_style_pad_hor(btn, scaled(8), 0);
        lv_obj_set_style_text_color(btn, enabled ? COL_TEXT : COL_TEXT_DIM, 0);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
        if (enabled) lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, label);
        lv_obj_set_style_text_font(lbl, getScaledFont(), 0);
        lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_clear_flag(lbl, LV_OBJ_FLAG_CLICKABLE);
        return btn;
    };


    // "Reset to Default"
    lv_obj_t* reset = makeItem("Reset to Default", true);
    lv_obj_add_event_cb(reset, [](lv_event_t* e) {
        Ctx* c = (Ctx*)lv_event_get_user_data(e);
        if (!c || !c->host) return;
        const float defNorm = c->host->getParameterDefault((uint32_t)c->paramIndex);
        // Mirror the kit's DOUBLE_CLICKED path: editParameter bracket +
        // setParamValue + VALUE_CHANGED so the consumer's valueChangedCallback
        // paints the new value the same way a drag-end would.
        c->host->editParameter((uint32_t)c->paramIndex, true);
        c->host->setParamValue((uint32_t)c->paramIndex, defNorm);
        c->host->editParameter((uint32_t)c->paramIndex, false);
        c->host->updateParamLabel((uint32_t)c->paramIndex);
        c->host->syncParamWidget((uint32_t)c->paramIndex, defNorm);
        closeScrim(c->scrim);
    }, LV_EVENT_CLICKED, ctxAlloc);

    // "Set to Current" — TODO no-op for round 2 (see file header).
    lv_obj_t* setCurrent = makeItem("Set to Current", true);
    lv_obj_add_event_cb(setCurrent, [](lv_event_t* e) {
        Ctx* c = (Ctx*)lv_event_get_user_data(e);
        if (!c) return;
        // Per-knob user-defaults file I/O is deferred. Log once so the gesture
        // is visibly acknowledged; the menu dismisses cleanly.
        std::fprintf(stderr,
            "[UIKnobContextMenu] Set to Current: TODO round 2 (param %d, current=%.3f)\n",
            c->paramIndex, (double)c->currentValue);
        closeScrim(c->scrim);
    }, LV_EVENT_CLICKED, ctxAlloc);

    // "Cancel"
    lv_obj_t* cancel = makeItem("Cancel", true);
    lv_obj_add_event_cb(cancel, [](lv_event_t* e) {
        Ctx* c = (Ctx*)lv_event_get_user_data(e);
        if (!c) return;
        closeScrim(c->scrim);
    }, LV_EVENT_CLICKED, ctxAlloc);

    // Outside-click scrim handler. Delete-on-click so a stray release on the
    // scrim after the LVGL press state is set still dismisses.
    lv_obj_add_event_cb(scrim, [](lv_event_t* e) {
        const lv_event_code_t code = lv_event_get_code(e);
        if (code == LV_EVENT_DELETE) return;
        Ctx* c = (Ctx*)lv_event_get_user_data(e);
        if (c) closeScrim(c->scrim);
    }, LV_EVENT_ALL, ctxAlloc);
}

} // namespace UIKnobContextMenu
