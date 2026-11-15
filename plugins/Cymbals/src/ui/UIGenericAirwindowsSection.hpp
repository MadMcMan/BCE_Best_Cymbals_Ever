#ifndef UI_GENERIC_AIRWINDOWS_SECTION_HPP
#define UI_GENERIC_AIRWINDOWS_SECTION_HPP

#include "UIWidgets.hpp"
#include "../PluginCymbals.hpp"
#include "airwindows/AirwindowsCommon.hpp"
#include <algorithm>
#include <string>
#include <vector>

START_NAMESPACE_DISTRHO

class UIGenericAirwindowsSection {
public:
    struct Config {
        int typeParamIndex;
        int firstParamIndex;
        const char* title;
        const char* options;
        typedef AirwindowsSpec (*GetSpecFn)(int);
        typedef const char* (*GetDescFn)(int);
        GetSpecFn getSpec;
        GetDescFn getDesc;
    };

    static void setControlName(int controlIndex, int firstParamIndex, const char* name, lv_obj_t** widgets, const char* units = nullptr) {
        int idx = firstParamIndex + controlIndex;
        lv_obj_t* arc = widgets[idx];
        if (!arc) return;
        lv_obj_t* knobCont = lv_obj_get_parent(arc);
        if (!knobCont) return;
        lv_obj_t* mainCont = lv_obj_get_parent(knobCont);
        if (!mainCont) return;
        lv_obj_t* nameLbl = lv_obj_get_child(mainCont, 0);
        if (nameLbl && lv_obj_check_type(nameLbl, &lv_label_class)) {
            if (name && units && units[0] != '\0') {
                // Format as "Name (unit)"
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%s (%s)", name, units);
                lv_label_set_text(nameLbl, buffer);
            } else {
                lv_label_set_text(nameLbl, name ? name : "");
            }
        }
    }

    static void updateControlVisibility(int type, const Config& config, lv_obj_t** widgets, lv_obj_t** labels) {
        lv_obj_t* descLabel = labels[config.typeParamIndex];
        if (descLabel) {
            lv_label_set_text(descLabel, config.getDesc(type));
        }

        AirwindowsSpec spec = config.getSpec(type);

        for (int i = 0; i < 10; ++i) {
            int idx = config.firstParamIndex + i;
            lv_obj_t* arc = widgets[idx];
            if (!arc) continue;
            lv_obj_t* knobCont = lv_obj_get_parent(arc);
            if (!knobCont) continue;
            lv_obj_t* mainCont = lv_obj_get_parent(knobCont);
            if (!mainCont) continue;

            if (type > 0 && i < spec.count) {
                const char* n = (i < spec.count) ? spec.names[i] : nullptr;
                const char* u = (i < spec.count) ? spec.units[i] : nullptr;
                if (n == nullptr || n[0] == '\0') {
                    char fallback[16];
                    snprintf(fallback, sizeof(fallback), "Param %d", i + 1);
                    setControlName(i, config.firstParamIndex, fallback, widgets, u);
                } else {
                    setControlName(i, config.firstParamIndex, n, widgets, u);
                }
                lv_obj_remove_flag(mainCont, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(mainCont, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    static lv_obj_t* build(lv_obj_t* parent, const Config& config, AbstractCymbalsUI* ui, UIStyles& styles,
                         lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                         lv_event_cb_t rollerCb, lv_event_cb_t valueCb) {
        
        lv_obj_t* panel = lv_obj_create(parent);
        lv_obj_set_size(panel, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_add_style(panel, &styles.panel, 0);
        lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

        // Header Row for Title and Drag Handle
        lv_obj_t* header = lv_obj_create(panel);
        lv_obj_set_size(header, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(header, 0, 0);
        lv_obj_set_style_border_width(header, 0, 0);
        lv_obj_set_style_pad_all(header, 0, 0);
        lv_obj_set_layout(header, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* titleLbl = lv_label_create(header);
        lv_label_set_text(titleLbl, config.title);
        lv_obj_add_style(titleLbl, &styles.title, 0);
        lv_obj_set_flex_grow(titleLbl, 1);

        // Drag handle for reordering (left of title)
        lv_obj_t* dragHandle = lv_obj_create(header);
        lv_obj_set_size(dragHandle, 40, 28);
        lv_obj_set_style_bg_color(dragHandle, COL_ACCENT_DIM, 0);
        lv_obj_set_style_border_width(dragHandle, 0, 0);
        lv_obj_set_style_radius(dragHandle, 4, 0);
        lv_obj_add_flag(dragHandle, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_pad_all(dragHandle, 0, 0);

        lv_obj_t* gripLabel = lv_label_create(dragHandle);
        lv_label_set_text(gripLabel, LV_SYMBOL_BARS);  // ≡≡≡ icon
        lv_obj_set_style_text_color(gripLabel, COL_TEXT, 0);
        lv_obj_center(gripLabel);

        // Attach drag callback
        lv_obj_add_event_cb(dragHandle, ui->getDragCallback(), LV_EVENT_ALL, ui);

        // Position title to right of drag handle
        lv_obj_move_to_index(titleLbl, 1);

        // Top Row: Selector and Description
        lv_obj_t* topRow = lv_obj_create(panel);
        lv_obj_set_size(topRow, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(topRow, 0, 0);
        lv_obj_set_style_border_width(topRow, 0, 0);
        lv_obj_set_style_pad_all(topRow, 0, 0);
        lv_obj_set_layout(topRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(topRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(topRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(topRow, LV_OBJ_FLAG_SCROLLABLE);

        // Algorithm dropdown (Left)
        lv_obj_t* dropdown = UIWidgets::createDropdown(topRow, config.typeParamIndex, "Algorithm",
                                  config.options, ui, styles, widgets, fParams, rollerCb);
        uiEnableDropdownListScroll(dropdown);
        uiFixDropdownPopup(dropdown);
        
        // Description label (Right)
        lv_obj_t* descCont = lv_obj_create(topRow);
        lv_obj_set_size(descCont, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(descCont, 1);
        lv_obj_set_style_bg_opa(descCont, 0, 0);
        lv_obj_set_style_border_width(descCont, 0, 0);
        lv_obj_set_style_pad_all(descCont, 0, 0);
        lv_obj_set_layout(descCont, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(descCont, LV_FLEX_FLOW_COLUMN);
        lv_obj_clear_flag(descCont, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* descLabel = lv_label_create(descCont);
        lv_label_set_text(descLabel, "");
        lv_obj_set_width(descLabel, lv_pct(100));
        lv_label_set_long_mode(descLabel, LV_LABEL_LONG_WRAP);
        lv_obj_add_style(descLabel, &styles.label, 0);
        lv_obj_set_style_text_color(descLabel, COL_TEXT_DIM, 0);
        
        if (labels) labels[config.typeParamIndex] = descLabel;

        // Params: 10 knobs total.
        lv_obj_t* grid = lv_obj_create(panel);
        lv_obj_set_size(grid, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(grid, 0, 0);
        lv_obj_set_style_border_width(grid, 0, 0);
        lv_obj_set_style_pad_all(grid, 0, 0);
        lv_obj_set_layout(grid, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(grid, 4, 0);
        lv_obj_set_style_pad_row(grid, 4, 0);
        lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

        for (int i = 0; i < 10; ++i) {
            UIWidgets::createArc(grid, config.firstParamIndex + i,
                                 "", ui, styles, widgets, labels, fParams, valueCb);
        }

        return panel;
    }
};

END_NAMESPACE_DISTRHO

#endif // UI_GENERIC_AIRWINDOWS_SECTION_HPP
