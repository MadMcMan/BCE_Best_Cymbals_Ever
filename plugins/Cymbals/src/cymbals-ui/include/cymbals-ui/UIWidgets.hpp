#ifndef UI_WIDGETS_HPP
#define UI_WIDGETS_HPP

#include "cymbals-ui/UIStyles.hpp"
#include "cymbals-ui/FFTAnalyzer.hpp"
#include "cymbals-ui/UICommon.hpp"
#include <cstdio>
#include <cstring>

START_NAMESPACE_DISTRHO

struct UIWidgets {
    struct ArcVisualSpec {
        lv_coord_t containerW = 92;
        lv_coord_t containerH = 104;
        lv_coord_t arcSize = 68;
        // Transparent margin around the face reserved for the modulation
        // ring. Compact grids (two-row FX slots) trade it down; keep a few px
        // or an active teal ring is clipped.
        lv_coord_t ringPad = 28;
        lv_coord_t labelMarginBottom = 1;
        lv_coord_t valueMarginTop = 1;
        // Functional color coding: when hueOverride is set
        // the value arc takes the section hue. arcHue is the lv_color_t the
        // override resolves to.
        bool hueOverride = false;
        lv_color_t arcHue{};
        // Extra width the TITLE label may use beyond containerW. The caller sets
        // this to its row gap: cells in a packed row are narrower than the text
        // they must show, but the gap beside them is empty, so a centred title
        // can borrow it without colliding with its neighbour. 0 = cell width.
        lv_coord_t titleExtraW = 0;
    };

    static ArcVisualSpec normalArcVisualSpec() noexcept {
        ArcVisualSpec spec{};
        spec.containerW = scaled(74);
        // Honest heights: knobCont is arcSize + ringPad(28), plus title line
        // (~16, may WRAP to 2 lines) + value line (~16) + margins.
        spec.containerH = scaled(38) + scaled(54) + scaled(28); // 120 @ 1.0
        spec.arcSize = scaled(54);
        spec.labelMarginBottom = 1;
        spec.valueMarginTop = 1;
        return spec;
    }

    static ArcVisualSpec smallArcVisualSpec() noexcept {
        ArcVisualSpec spec{};
        spec.containerW = scaled(56);
        spec.containerH = scaled(38) + scaled(36) + scaled(28); // 102 @ 1.0
        spec.arcSize = scaled(36);
        spec.labelMarginBottom = 1;
        spec.valueMarginTop = 1;
        return spec;
    }

    static ArcVisualSpec smallWideTitleArcVisualSpec() noexcept {
        ArcVisualSpec spec = smallArcVisualSpec();
        spec.containerW = scaled(70);
        return spec;
    }

    static ArcVisualSpec smallWideTitleNoValueArcVisualSpec() noexcept {
        ArcVisualSpec spec = smallWideTitleArcVisualSpec();
        spec.containerW = scaled(64);
        spec.containerH = scaled(52); // no value label: title+knob only
        spec.arcSize = scaled(30);
        return spec;
    }

    static ArcVisualSpec compactDispersionArcVisualSpec() noexcept {
        ArcVisualSpec spec{};
        spec.containerW = scaled(58);
        spec.containerH = scaled(72) + scaled(28); // 100 @ 1.0
        spec.arcSize = scaled(40);
        spec.labelMarginBottom = 0;
        spec.valueMarginTop = 0;
        return spec;
    }

    static ArcVisualSpec microArcVisualSpec() noexcept {
        ArcVisualSpec spec{};
        spec.containerW = scaled(44);
        spec.containerH = scaled(38) + scaled(30) + scaled(28); // 96 @ 1.0
        spec.arcSize = scaled(30);
        spec.labelMarginBottom = 0;
        spec.valueMarginTop = 0;
        return spec;
    }

    static lv_obj_t* createKnobTitleLabel(lv_obj_t* parent, const char* title,
                                          UIStyles& styles, lv_coord_t width,
                                          lv_label_long_mode_t longMode = LV_LABEL_LONG_DOT);  // one line ALWAYS: a wrapped title would push its knob down

    static lv_obj_t* createCompactControlColumn(lv_obj_t* parent, lv_coord_t width,
                                                lv_coord_t height = LV_SIZE_CONTENT,
                                                lv_coord_t rowPad = 0);

    static lv_obj_t* createCompactDropdown(lv_obj_t* parent, UIStyles& styles,
                                           lv_coord_t width, lv_coord_t height = LV_SIZE_CONTENT);

    static lv_obj_t* createCompactLabeledDropdown(lv_obj_t* parent, int paramIndex,
                                                  const char* name, const char* options,
                                                  AbstractCymbalsUI* ui, UIStyles& styles,
                                                  lv_obj_t** widgets, float* fParams,
                                                  lv_event_cb_t callback,
                                                  lv_coord_t width = scaled(196),
                                                  lv_coord_t dropdownH = scaled(24),
                                                  lv_coord_t rowPad = 0);

    static lv_obj_t* createCompactValueLabel(lv_obj_t* parent, UIStyles& styles,
                                             lv_coord_t width, lv_coord_t marginTop = 1);

    static lv_obj_t* createPanel(lv_obj_t* parent, const char* title, int32_t w, int32_t h, UIStyles& styles) {
        lv_obj_t* panel = lv_obj_create(parent);
        lv_obj_add_style(panel, &styles.panel, 0);
        lv_obj_set_size(panel, w, h);
        lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* lbl = lv_label_create(panel);
        lv_label_set_text(lbl, title);
        lv_obj_add_style(lbl, &styles.title, 0);
        lv_obj_set_style_margin_bottom(lbl, scaled(5), 0);

        return panel;
    }

    static void createArc(lv_obj_t* parent, int paramIndex, const char* name, 
                            AbstractCymbalsUI* ui, UIStyles& styles, 
                            lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                            lv_event_cb_t callback);

    static void createArcEx(lv_obj_t* parent, int paramIndex, const char* name,
                            AbstractCymbalsUI* ui, UIStyles& styles,
                            lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                            lv_event_cb_t callback, const ArcVisualSpec& spec,
                            bool showValue = true);

    // Creates a mini arc knob with full visual treatment (cap, shading, needle)
    // for use in EQ editor and other compact layouts.
    // The arcSize parameter determines the visual size; typical values are 36-48.
    // Creates a simple arc knob with canonical styles and drag behavior.
    // Used for inline arc creation where the caller manages the container.
    // Returns the arc widget which must be stored by caller.
    static lv_obj_t* createSimpleArcWithDrag(lv_obj_t* parent, lv_coord_t arcSize,
                                            float initialValue, UIStyles& styles,
                                            lv_event_cb_t dragCallback = nullptr,
                                            void* dragUserData = nullptr);

    // Finds the teal modulation sweep ring in a knob container (scans children
    // for the user_data sentinel -2; robust to child ordering).
    static lv_obj_t* findModRing(lv_obj_t* knobCont);

    // Returns true when the given arc widget has an active drag gesture in progress.
    // Used by valueChangedCallback to skip per-frame gesture wrapping while arcDragCb owns the gesture.
    static bool isArcInGesture(lv_obj_t* arc);

    // FINE DRAG (shift-drag): when active, arcDragCb scales pointer travel down
    // by kFineDragFactor so the whole range needs ~10x the distance, giving
    // precise dialling. The host owns the modifier state - the UI layer tracks
    // shift (onKeyboard/onMouse) and publishes it here, because LVGL's drag
    // callback never sees a DGL event and so cannot read ev.mod itself.
    // Make an object's scrollbar DRAGGABLE.
    //
    // LVGL 9 draws scrollbars as indicators only: there is no scrollbar object
    // and no press/drag handling for one anywhere in the core (lv_obj.c only
    // DRAWS it; v8 had a real draggable scrollbar child and v9 removed it). So
    // grabbing the bar and pulling does nothing, which reads as a broken list.
    // This restores the gesture: a press landing inside the vertical bar starts a
    // drag, and motion maps the pointer's position along the track to the scroll
    // offset - absolute positioning, the way a scrollbar is expected to behave.
    static void attachScrollbarDrag(lv_obj_t* scrollable);

    static void setFineDragActive(bool active) noexcept;
    static bool isFineDragActive() noexcept;

    // Attach the canonical arc drag behavior used across the app.
    static void attachArcDrag(lv_obj_t* arc, AbstractCymbalsUI* ui);

    // Programmatically set an arc value and resync the shared knob face/needle.
    static void setArcValueAndSyncVisual(lv_obj_t* arc, int value);

    // Same visual sync, NO sweep. For callers that stream values faster than
    // the 200ms animation - a per-pointer-move pad drag or an automation
    // refresh. With the animated setter those callers would restart a sweep on
    // every move, so the knob chases the finger and never settles.
    static void setArcValueInstant(lv_obj_t* arc, int value);

    // Returns the current shared knob face rotation angle for a shared arc, or -1 if unavailable.
    static int getArcVisualAngle(lv_obj_t* arc);

    // Optional per-arc discrete snapping. Return true-snapped value (0..1000).
    // Default nullptr = no snapping. Cymbals installs a hook mirroring Airwindows logic.
    // ponytail: extra AbstractCymbalsUI* param so hook can read slot type without global state.
    using DiscreteSnapFn = int(*)(AbstractCymbalsUI* ui, int paramIndex, int rawValue01k, void* userData);
    static void setDiscreteSnap(DiscreteSnapFn fn, void* userData);
    static DiscreteSnapFn getDiscreteSnap() noexcept;
    static void* getDiscreteSnapUserData() noexcept;

    static lv_obj_t* createDropdown(lv_obj_t* parent, int paramIndex, const char* name, 
                                 const char* options, AbstractCymbalsUI* ui, UIStyles& styles,
                                 lv_obj_t** widgets, float* fParams, lv_event_cb_t callback) {
        lv_obj_t* cont = lv_obj_create(parent);
        lv_obj_set_size(cont, scaled(280), scaled(76));
        lv_obj_set_style_bg_opa(cont, 0, 0);
        lv_obj_set_style_border_width(cont, 0, 0);
        lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(cont, scaled(6), 0);
        lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* lbl = lv_label_create(cont);
        lv_label_set_text(lbl, name);
        lv_obj_add_style(lbl, &styles.title, 0);  // group-header tier (bright, tracked)

        lv_obj_t* dropdown = lv_dropdown_create(cont);
        lv_dropdown_set_options(dropdown, options);
        lv_obj_set_width(dropdown, scaled(260));
        lv_obj_set_height(dropdown, scaled(40));
        lv_obj_set_style_pad_top(dropdown, scaled(7), 0);
        lv_obj_set_style_pad_bottom(dropdown, scaled(7), 0);
        lv_obj_set_scrollbar_mode(dropdown, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(dropdown, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_style(dropdown, &styles.compactSelectMain, 0);  // raised machined face
        // Popup list: app palette, not the LVGL default theme
        lv_obj_t* list = lv_dropdown_get_list(dropdown);
        if (list != nullptr) {
            lv_obj_add_style(list, &styles.compactSelectListMain, LV_PART_MAIN);
            lv_obj_add_style(list, &styles.compactSelectListSelected, LV_PART_SELECTED);
        }
        lv_dropdown_set_selected(dropdown, (int)fParams[paramIndex]);
        
        lv_obj_set_user_data(dropdown, reinterpret_cast<void*>((intptr_t)paramIndex));
        lv_obj_add_event_cb(dropdown, callback, LV_EVENT_VALUE_CHANGED, ui);
        
        if (widgets) widgets[paramIndex] = dropdown;
        return dropdown;
    }
    
    // Spectrum Analyzer - creates a custom display for FFT visualization.
    // barObjects receives the per-band bars; peakDotOut (optional) receives a
    // small "output LED" the caller drives while audio flows.
    static lv_obj_t* createSpectrumAnalyzer(lv_obj_t* parent, int32_t w, int32_t h, UIStyles& styles, lv_obj_t** barObjects = nullptr, lv_obj_t** peakDotOut = nullptr);

    static lv_obj_t* makeRow(lv_obj_t* parent);

    // Uniform screen title strip: machined-plate bar with the screen title left,
    // a flex spacer, and room for screen-level controls appended by the caller.
    // height: LV_SIZE_CONTENT with min-height scaled(30); styles.panel background,
    // radius scaled(6), pad_hor scaled(10), pad_ver scaled(4); flex ROW, align
    // CENTER. Children: title label (styles.title) with user_data (void*)(intptr_t)-1,
    // then a flex-grow spacer with user_data (void*)(intptr_t)-1. The -1 sentinels
    // keep sub-tab switcher loops (which filter children by user_data) from
    // mistaking the title/spacer for a sub-tab button.
    static lv_obj_t* createScreenStrip(lv_obj_t* parent, UIStyles& styles, const char* title);
};

END_NAMESPACE_DISTRHO

#endif // UI_WIDGETS_HPP
