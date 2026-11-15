#ifndef UI_EQ_SECTION_HPP
#define UI_EQ_SECTION_HPP

#include "UIWidgets.hpp"
#include "UIEQEditor.hpp"
#include <cstddef>
START_NAMESPACE_DISTRHO

struct UIEQSection {
    enum class ControlType {
        Arc,
        Dropdown,
    };

    struct HeaderControlSpec {
        int param;
        const char* label;
        ControlType type = ControlType::Arc;
        const char* options = nullptr;
    };

    struct BuildOptions {
        const char* titleText = nullptr;
        int eqBaseParam = 0;
        lv_coord_t areaHeight = LV_SIZE_CONTENT;
        bool fillHeight = false;
        bool grow = false;
        bool cardStyle = true;
        lv_coord_t headerHeight = 120;
        lv_flex_align_t headerMainAlign = LV_FLEX_ALIGN_END;
        bool compactHeaderArcs = false;
        lv_obj_t** outTopBar = nullptr;  // EQ editor's Mix/Tilt/Pivot top strip
    };

    static lv_obj_t* create(lv_obj_t* parent,
                            AbstractCymbalsUI* ui,
                            UIStyles& styles,
                            lv_obj_t** widgets,
                            lv_obj_t** labels,
                            float* fParams,
                            lv_event_cb_t valueCb,
                            lv_event_cb_t dropdownCb,
                            const BuildOptions& options,
                            const HeaderControlSpec* controlSpecs,
                            std::size_t controlCount) {
        lv_obj_t* area = lv_obj_create(parent);
        lv_obj_set_width(area, lv_pct(100));
        if (options.fillHeight)
            lv_obj_set_height(area, lv_pct(100));
        else
            lv_obj_set_height(area, options.areaHeight);

        if (options.grow)
            lv_obj_set_flex_grow(area, 1);

        if (options.cardStyle) {
            lv_obj_set_style_bg_color(area, COL_PANEL, 0);
            lv_obj_set_style_bg_opa(area, LV_OPA_COVER, 0);
            lv_obj_set_style_border_color(area, COL_ACCENT, 0);
            lv_obj_set_style_border_width(area, 2, 0);
            lv_obj_set_style_radius(area, 8, 0);
            lv_obj_set_style_pad_all(area, 12, 0);
        } else {
            // IMPULSES-style transparent section: the knob strip sits directly
            // on the panel, so the default-theme 20px padding is pure dead
            // band above the Mix/Tilt/Pivot titles — and the theme 2px border
            // draws a stray rect around a section that must be borderless.
            // Zeroed (batch 5); bg transparency is pre-existing, keep it.
            lv_obj_set_style_bg_opa(area, 0, 0);
            lv_obj_set_style_pad_all(area, 0, 0);
            lv_obj_set_style_pad_row(area, 0, 0);
            lv_obj_set_style_border_width(area, 0, 0);
        }

        lv_obj_set_layout(area, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(area, LV_FLEX_FLOW_COLUMN);
        lv_obj_clear_flag(area, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(area, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t* header = lv_obj_create(area);
        // LV_SIZE_CONTENT: knob containers grew (mod-ring clearance), so a
        // fixed headerHeight clipped arcs and value labels. The header now
        // wraps its tallest child exactly.
        lv_obj_set_size(header, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_min_height(header, options.headerHeight, 0);
        lv_obj_set_style_bg_opa(header, 0, 0);
        lv_obj_set_style_border_width(header, 0, 0);
        lv_obj_set_style_pad_all(header, 0, 0);
        lv_obj_set_style_pad_column(header, 8, 0);
        lv_obj_set_style_pad_row(header, 0, 0);
        lv_obj_set_layout(header, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(header, options.headerMainAlign, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

        if (options.titleText && options.titleText[0] != '\0') {
            lv_obj_t* title = lv_label_create(header);
            lv_label_set_text(title, options.titleText);
            lv_obj_add_style(title, &styles.title, 0);
        }

        for (std::size_t i = 0; i < controlCount; ++i) {
            const HeaderControlSpec& spec = controlSpecs[i];
            if (spec.type == ControlType::Dropdown) {
                lv_obj_t* cont = UIWidgets::createCompactControlColumn(header, 92, 82);

                lv_obj_t* lbl = UIWidgets::createKnobTitleLabel(cont, spec.label, styles, 92);
                lv_obj_set_style_margin_bottom(lbl, 1, 0);

                lv_obj_t* dd = UIWidgets::createCompactDropdown(cont, styles, 84);
                lv_dropdown_set_options(dd, (spec.options && spec.options[0] != '\0') ? spec.options : "1P\n2P");
                lv_dropdown_set_selected(dd, (uint16_t)fParams[spec.param]);
                lv_obj_set_user_data(dd, reinterpret_cast<void*>((intptr_t)spec.param));
                lv_obj_add_event_cb(dd, dropdownCb, LV_EVENT_VALUE_CHANGED, ui);
                uiEnableDropdownListScroll(dd);
                uiFixDropdownPopup(dd);

                widgets[spec.param] = dd;
                continue;
            }

            if (options.compactHeaderArcs) {
                UIWidgets::ArcVisualSpec compact = UIWidgets::smallArcVisualSpec();
                compact.labelMarginBottom = 0;
                compact.valueMarginTop = 0;
                compact.containerH = scaled(72);  // packed 36 rhythm: 15 + 40 + 15 + 2 slack
                UIWidgets::createArcEx(header,
                                       spec.param,
                                       spec.label,
                                       ui,
                                       styles,
                                       widgets,
                                       labels,
                                       fParams,
                                       valueCb,
                                       compact);
                // Packed rhythm, universal with the rail + card stacks: zero
                // column pad, then layout-independent top-pin into a 40px box.
                lv_obj_t* mkc = (widgets[spec.param] != nullptr) ? lv_obj_get_parent(widgets[spec.param]) : nullptr;
                lv_obj_t* mcol = (mkc != nullptr) ? lv_obj_get_parent(mkc) : nullptr;
                if (mcol != nullptr) lv_obj_set_style_pad_row(mcol, scaled(0), 0);
                uiPackKnob(widgets[spec.param], compact.arcSize, scaled(0), scaled(40));
            } else {
                const UIWidgets::ArcVisualSpec normal = UIWidgets::normalArcVisualSpec();
                UIWidgets::createArcEx(header,
                                       spec.param,
                                       spec.label,
                                       ui,
                                       styles,
                                       widgets,
                                       labels,
                                       fParams,
                                       valueCb,
                                       normal);
            }
            // Plugin-local cap material (softened bands; knob logic stays kit).
        }

        UIEQEditor::create(area, options.eqBaseParam, ui, styles, header, widgets, 0,
                           options.outTopBar);
        return area;
    }
};

END_NAMESPACE_DISTRHO

#endif // UI_EQ_SECTION_HPP
