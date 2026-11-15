#ifndef UI_KEYBOARD_HPP
#define UI_KEYBOARD_HPP

#include "UICommon.hpp"

START_NAMESPACE_DISTRHO

// Compact playable keyboard for the bottom bar. Keys are ~1/3 the height of
// the old 150px ones so the footer stays slim on every tab while remaining
// comfortably tappable (>= 40px tall).
struct UIKeyboard {
    // keyH: the playable keybed height (piano fills the footer's height).
    // Explicit rather than pct: pct heights through the nested non-layout
    // footer containers resolve against stale sizes (measured 7px keys).
    static void create(lv_obj_t* parent, AbstractCymbalsUI* ui, lv_event_cb_t callback, lv_coord_t keyH) {
        // Two octaves (C3-C5). White keys are a bit narrower so they fit next
        // to the analyzer. Key heights follow the container height.
        const lv_coord_t whiteW = scaled(40);
        const lv_coord_t blackW = scaled(24);
        const lv_coord_t blackH = (keyH * 64) / 100;   // black keys ~64% of white
        const lv_coord_t gap    = scaled(4);
        const lv_coord_t pitch  = whiteW + gap;   // white key stride
        const int keyCount = 15;  // two octaves of white keys

        lv_obj_t* keysCont = lv_obj_create(parent);
        lv_obj_set_size(keysCont, pitch * keyCount - gap, keyH);
        lv_obj_set_style_bg_opa(keysCont, 0, 0);
        lv_obj_set_style_border_width(keysCont, 0, 0);
        // Never scroll, never clip: the keybed is a fixed instrument surface.
        lv_obj_clear_flag(keysCont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(keysCont, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
        lv_obj_clear_flag(keysCont, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
        lv_obj_set_scrollbar_mode(keysCont, LV_SCROLLBAR_MODE_OFF);
        // Let keys overhang the logical container instead of being cut off.
        lv_obj_add_flag(keysCont, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        // Inert surface: containers are clickable by default, but keysCont must
        // never become the pressed object — a drag that pauses over a gap would
        // move the press onto the container and its RELEASED would go nowhere,
        // hanging the note. Keys are separate clickables and stay unaffected.
        lv_obj_clear_flag(keysCont, LV_OBJ_FLAG_CLICKABLE);

        // Enable flex layout for white keys
        lv_obj_set_layout(keysCont, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(keysCont, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(keysCont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_gap(keysCont, gap, 0);

        // White keys: two octaves (C3-C5).
        uint8_t whiteNotes[] = {60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77, 79, 81, 83, 84};
        const char* whiteLabels[] = {"C3", "D3", "E3", "F3", "G3", "A3", "B3",
                                     "C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"};

        for (int i = 0; i < keyCount; ++i) {
            lv_obj_t* btn = lv_button_create(keysCont);
            lv_obj_set_size(btn, whiteW, keyH);
            lv_obj_set_style_bg_color(btn, COL_KEYBOARD_BTN, 0);
            lv_obj_set_style_bg_color(btn, COL_KEYBOARD_BTN_PRESSED, LV_STATE_PRESSED);
            lv_obj_set_style_radius(btn, scaled(4), 0);
            lv_obj_set_style_border_width(btn, 1, 0);
            lv_obj_set_style_border_color(btn, COL_KEYBOARD_BORDER, 0);
            lv_obj_set_style_shadow_width(btn, 0, 0);

            lv_obj_t* lbl = lv_label_create(btn);
            lv_label_set_text(lbl, whiteLabels[i]);
            // Explicit small face + tracked near-black ink: at 1280 the
            // default theme face let labels spill toward the key gaps and
            // read as misaligned (critic round 1). 12px tracked text sits
            // unambiguously inside its own 40px white key.
            lv_obj_set_style_text_font(lbl, getScaledSmallFont(), 0);
            lv_obj_set_style_text_color(lbl, lv_color_hex(0x0E1116), 0);
            lv_obj_set_style_text_letter_space(lbl, scaled(1), 0);
            lv_obj_align(lbl, LV_ALIGN_BOTTOM_MID, 0, scaled(-8));  // clear the window-bottom clip

            lv_obj_set_user_data(btn, (void*)(intptr_t)whiteNotes[i]);
            lv_obj_add_event_cb(btn, callback, LV_EVENT_ALL, ui);
        }

        // Black keys sit on the white-key boundaries across both octaves.
        struct BlackKey { uint8_t note; int afterWhite; const char* lbl; };
        BlackKey blackKeys[] = {
            {61, 0, "C#"}, {63, 1, "D#"},
            {66, 3, "F#"}, {68, 4, "G#"}, {70, 5, "A#"},
            {73, 7, "C#"}, {75, 8, "D#"},
            {78, 10, "F#"}, {80, 11, "G#"}, {82, 12, "A#"}
        };

        for (int i = 0; i < 10; ++i) {
            lv_obj_t* btn = lv_button_create(keysCont);
            lv_obj_set_size(btn, blackW, blackH);
            lv_obj_add_flag(btn, LV_OBJ_FLAG_IGNORE_LAYOUT);
            // Center the black key on the gap between the two adjacent white keys.
            lv_obj_set_pos(btn, (blackKeys[i].afterWhite + 1) * pitch - gap / 2 - blackW / 2, 0);

            // Visible dark-gray face (black-key read): subtle
            // vertical sheen + lighter top edge so the keys read against the
            // dark footer (critic round 7).
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x2E3238), 0);
            lv_obj_set_style_bg_grad_color(btn, lv_color_hex(0x1C1F23), 0);
            lv_obj_set_style_bg_grad_dir(btn, LV_GRAD_DIR_VER, 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(0x3A3E44), 0);
            lv_obj_set_style_border_width(btn, 1, 0);
            lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_TOP, 0);
            lv_obj_set_style_bg_color(btn, COL_KEYBOARD_BLACK_PRESSED, LV_STATE_PRESSED);
            lv_obj_set_style_radius(btn, scaled(2), 0);
            lv_obj_set_style_shadow_width(btn, 0, 0);
            // No note labels on black keys (whites keep them) — reduces noise
            // at the footer's keybed scale (critic round 5).
            lv_obj_set_user_data(btn, (void*)(intptr_t)blackKeys[i].note);
            lv_obj_add_event_cb(btn, callback, LV_EVENT_ALL, ui);
        }
    }
};

END_NAMESPACE_DISTRHO

#endif // UI_KEYBOARD_HPP
