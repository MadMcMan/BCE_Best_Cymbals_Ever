#pragma once

#include "lvgl.h"
#include "UICommon.hpp"
#include "UIStyles.hpp"
#include "UIWidgets.hpp"
#include "../airwindows/AirSuiteUniversal.hpp"
#include "../fx/InternalFxMeta.hpp"
#include "../fx/VendoredFxMeta.hpp"
#include "../airwindows/AirwindowsCommon.hpp"
#include "../PluginCymbals.hpp"
#include "../surge/SurgeFx.hpp"
#include <cstdio>

START_NAMESPACE_DISTRHO

// Parameter metadata for a slot.
//
// The generated universal spec table does not cover every registered type:
// scripts/gen_airwindows.py explicitly SKIPS AirSuiteDynamics ("non-uniform
// Impl"), so DYN algorithms (ButterComp2 et al.) fall through to
// `default: return AirwindowsSpec()` and the card showed just the WET knob
// (reported). Re-running the generator does not close that gap. Fall back to
// the WinVST-extracted parameter table, which IS generated for every type.
inline int fxSlotParamCount(AirSuiteUniversal::Type type, AirwindowsSpec& suppressed) {
    suppressed = AirSuiteUniversal::getSuppressedSpec(type);
    // The WinVST-extracted table is the algorithm's complete native parameter
    // list. The generated spec table is sometimes a shortened/legacy view
    // (Air4 lists three controls while the algorithm has four), so prefer the
    // extracted count and let getParamDisplayName() hide an internal wet knob.
    const int extracted = std::clamp(AirSuiteUniversal::getExtractedParamCount(type), 0, 13);
    if (extracted > 0) return extracted;
    return suppressed.count;
}

inline const char* fxSlotParamName(AirSuiteUniversal::Type type, int index) {
    const char* name = AirSuiteUniversal::getParamDisplayName(type, index, true);
    return (name != nullptr && name[0] != '\0') ? name : nullptr;
}

// Unified source-agnostic views. Airwindows keeps ids < kBaseType; Surge ids
// start at CymbalsSurgeFx::kBaseType and share the same 13 slot params.
inline bool fxTypeIsSurge(int typeId) {
    return typeId >= CymbalsSurgeFx::kBaseType && !CymbalsInternalFx::isInternalId(typeId)
        && !CymbalsVendoredFx::isVendoredId(typeId);
}
inline bool fxTypeIsInternal(int typeId) {
    return CymbalsInternalFx::isInternalId(typeId);
}
inline bool fxTypeIsVendored(int typeId) {
    return CymbalsVendoredFx::isVendoredId(typeId);
}
inline bool fxTypeIsNone(int typeId) {
    return typeId == (int)AirSuiteUniversal::NONE;
}
inline int fxSurfaceParamCount(int typeId) {
    if (fxTypeIsVendored(typeId))
        return CymbalsVendoredFx::parameterCount(CymbalsVendoredFx::indexOf(typeId));
    if (fxTypeIsInternal(typeId))
        return CymbalsInternalFx::parameterCount(CymbalsInternalFx::indexOf(typeId));
    if (fxTypeIsSurge(typeId))
        return CymbalsSurgeFx::parameterCount(typeId - CymbalsSurgeFx::kBaseType);
    AirwindowsSpec suppressed{};
    return fxSlotParamCount((AirSuiteUniversal::Type)typeId, suppressed);
}
inline const char* fxSurfaceParamName(int typeId, int index) {
    if (fxTypeIsVendored(typeId))
        return CymbalsVendoredFx::parameterName(CymbalsVendoredFx::indexOf(typeId), index);
    if (fxTypeIsInternal(typeId))
        return CymbalsInternalFx::parameterName(CymbalsInternalFx::indexOf(typeId), index);
    if (fxTypeIsSurge(typeId))
        return CymbalsSurgeFx::parameterName(typeId - CymbalsSurgeFx::kBaseType, index);
    return fxSlotParamName((AirSuiteUniversal::Type)typeId, index);
}
inline const char* fxSurfaceName(int typeId) {
    if (fxTypeIsVendored(typeId))
        return CymbalsVendoredFx::name(CymbalsVendoredFx::indexOf(typeId));
    if (fxTypeIsInternal(typeId))
        return CymbalsInternalFx::name(CymbalsInternalFx::indexOf(typeId));
    if (fxTypeIsSurge(typeId))
        return CymbalsSurgeFx::algorithmName(typeId - CymbalsSurgeFx::kBaseType);
    const auto& identity = AirSuiteUniversal::getRuntimeIdentity((AirSuiteUniversal::Type)typeId);
    return identity.displayName ? identity.displayName : "";
}
inline const char* fxSurfaceCategory(int typeId) {
    if (fxTypeIsVendored(typeId))
        return CymbalsVendoredFx::category(CymbalsVendoredFx::indexOf(typeId));
    if (fxTypeIsInternal(typeId))
        return CymbalsInternalFx::category(CymbalsInternalFx::indexOf(typeId));
    if (fxTypeIsSurge(typeId))
        return CymbalsSurgeFx::algorithmCategory(typeId - CymbalsSurgeFx::kBaseType);
    const auto& identity = AirSuiteUniversal::getRuntimeIdentity((AirSuiteUniversal::Type)typeId);
    return identity.categoryName ? identity.categoryName : "";
}

// Builds (or rebuilds) the slot card's knob grid for `type`.
//
// The grid is chosen from the LIVE algorithm, not from the empty slot: a card
// is created while the slot is EMPTY (one cell), and the type is applied later
// via updatePlugin. Sizing the row once at creation left a selected 10-param
// algorithm laid out with the empty slot's big single-row cells - knobs ran off
// the card and the second row never existed (reported on ToTape9/ToTape8).
// updatePlugin now clears and repopulates through this same function.
inline void populateFxArcRow(lv_obj_t* arcRow, int typeIdx, int wetIdx,
                             int typeId, AbstractCymbalsUI* ui,
                             UIStyles& styles, lv_obj_t** widgets, lv_obj_t** labels,
                             float* fParams, lv_event_cb_t valueCb) {
    if (arcRow == nullptr) return;
    constexpr int kFxArcGapPx = 6;
    constexpr int kFxArcRowAvailPx = 395;  // design px measured off a 1280 render

    lv_obj_clean(arcRow);
    lv_obj_set_layout(arcRow, LV_LAYOUT_FLEX);
    lv_obj_set_style_pad_all(arcRow, 0, 0);
    lv_obj_set_style_pad_row(arcRow, 0, 0);
    lv_obj_set_style_pad_column(arcRow, scaled(kFxArcGapPx), 0);
    lv_obj_set_width(arcRow, lv_pct(100));
    // Take the card's leftover height and spread the wrapped knob rows through
    // it. The FX page gives each slot card more height than two compact rows
    // need; previously the content hugged the top and left a dead band below.
    lv_obj_set_height(arcRow, 0);
    lv_obj_set_flex_grow(arcRow, 1);
    // CENTER, not START: the six-cell grid is capped at the compact cell width,
    // so a wider slot card used to leave all of its slack on the right. The
    // controls now sit with equal left/right margins.
    lv_obj_set_flex_align(arcRow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_obj_set_user_data(arcRow, (void*)(intptr_t)0xAA);  // "arc row" sentinel

    const int paramCount = fxTypeIsNone(typeId) ? 0 : fxSurfaceParamCount(typeId);
    const int visibleArcs = std::max(1, paramCount + 1);   // params + WET
    const int arcRows = visibleArcs > 9 ? 2 : 1;
    const int perRow = (visibleArcs + arcRows - 1) / arcRows;
    lv_obj_set_flex_flow(arcRow, arcRows > 1 ? LV_FLEX_FLOW_ROW_WRAP : LV_FLEX_FLOW_ROW);
    // Separate the two wrapped rows from each other while keeping the title and
    // value tight to the cap inside each cell.
    lv_obj_set_style_pad_row(arcRow, arcRows > 1 ? scaled(4) : 0, 0);

    UIWidgets::ArcVisualSpec arcSpec = UIWidgets::smallArcVisualSpec();
    const int availPx = scaled(kFxArcRowAvailPx);
    const int gapPx = scaled(kFxArcGapPx);
    // 6px safety margin: hairline/padding rounding otherwise clips the last
    // cell by ~5px (measured by the layout audit).
    const int cellW = std::clamp((availPx - (perRow - 1) * gapPx - scaled(6)) / perRow,
                                 scaled(30), UIWidgets::smallArcVisualSpec().containerW);
    arcSpec.containerW = cellW;
    // Two-row algorithms get the freed vertical space: the 20px cap left the
    // knobs visibly smaller than the one-row cells. 22px is the largest cap
    // that still clears the title/value lines inside the card height.
    arcSpec.arcSize = std::clamp(cellW * 36 / 56, scaled(16),
                                 arcRows > 1 ? scaled(22) : scaled(36));
    // Pull the title/value toward the cap with NEGATIVE margins rather than
    // shrinking ringPad: knobCont must stay arcSize+28 for the one-row mod ring
    // (and arcSize+4 for the deliberately compact two-row cells), so the
    // closeness has to come from the text boxes overlapping the transparent
    // ring reserve.
    arcSpec.labelMarginBottom = arcRows > 1 ? -scaled(2) : -scaled(6);
    arcSpec.valueMarginTop = arcSpec.labelMarginBottom;
    if (arcRows > 1) {
        arcSpec.ringPad = scaled(4);
        arcSpec.containerH = scaled(36) + arcSpec.arcSize;
    } else {
        arcSpec.ringPad = scaled(28);
        arcSpec.containerH = scaled(38) + arcSpec.arcSize + scaled(28);
    }
    // No title overhang: with titleExtraW the label borrowed the right-hand
    // gap and its centred text sat half a cell-gap right of the knob. A
    // zero extra width centres the title over its own cell; long Airwindows
    // names ellipsise inside the cell instead.
    arcSpec.titleExtraW = 0;

    // One uniform compact tier for every FX knob title and value. The default
    // adaptive title face stepped up for short names (Left/Right) while long
    // names (Feedback/Crossfeed) stepped down, so a two-row card read with two
    // different label sizes.
    auto compactFxLabels = [&](int p) {
        lv_obj_t* w = widgets[p];
        if (!w) return;
        lv_obj_t* knobCont = lv_obj_get_parent(w);
        lv_obj_t* col = knobCont ? lv_obj_get_parent(knobCont) : nullptr;
        if (col) {
            lv_obj_t* title = lv_obj_get_child(col, 0);
            if (title) {
                lv_obj_set_style_text_font(title, uiMicroFont(), 0);
                lv_obj_set_style_text_letter_space(title, 0, 0);
            }
        }
        if (labels[p]) lv_obj_set_style_text_font(labels[p], uiMicroFont(), 0);
    };

    for (int i = 0; i < 13; ++i) {
        const int paramIdx = typeIdx + 1 + i;
        const char* name = (!fxTypeIsNone(typeId) && i < paramCount)
                               ? fxSurfaceParamName(typeId, i) : nullptr;
        const bool visible = (name != nullptr);
        UIWidgets::createArcEx(arcRow, paramIdx, visible ? name : "P", ui, styles,
                               widgets, labels, fParams, valueCb, arcSpec, true);
        compactFxLabels(paramIdx);
        if (!visible && widgets[paramIdx] != nullptr) {
            lv_obj_t* cont = lv_obj_get_parent(lv_obj_get_parent(widgets[paramIdx]));
            if (cont != nullptr) lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
        }
    }
    UIWidgets::createArcEx(arcRow, wetIdx, "Wet", ui, styles,
                           widgets, labels, fParams, valueCb, arcSpec, true);
    compactFxLabels(wetIdx);

    if (fxTypeIsNone(typeId)) lv_obj_add_flag(arcRow, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_clear_flag(arcRow, LV_OBJ_FLAG_HIDDEN);
}

class UIModularSlotSection {
public:
    // Build the slot panel into `parent`. Returns the panel object.
    // slot is 0-indexed (0..5).
    static lv_obj_t* build(
        lv_obj_t* parent,
        int slot,
        AbstractCymbalsUI* ui,
        UIStyles& styles,
        lv_obj_t** widgets,
        lv_obj_t** labels,
        float* fParams,
        lv_event_cb_t valueCb,
        lv_event_cb_t dragCb
    );

    // Call when slot type changes (from parameterChanged).
    // Rebuilds arc visibility + labels.
    static void updatePlugin(
        lv_obj_t* panel,
        int newType,
        AbstractCymbalsUI* ui,
        UIStyles& styles,
        lv_obj_t** widgets,
        lv_obj_t** labels,
        float* fParams,
        lv_event_cb_t valueCb
    );

    // Helper: given a panel, extract the slot index (stored as user_data).
    // Returns 0-indexed slot (0..5).
    static int getSlotIndex(lv_obj_t* panel) {
        intptr_t data = (intptr_t)lv_obj_get_user_data(panel);
        return (int)(data) - 1;  // 1-indexed → 0-indexed
    }

private:
    // Event handler for the "×" remove button
    static void onRemoveClicked(lv_event_t* e);
    // Event handler for the "+ ADD PLUGIN" button
    static void onAddClicked(lv_event_t* e);
};

// ============================================================================
// INLINE IMPLEMENTATION
// ============================================================================

inline lv_obj_t* UIModularSlotSection::build(
    lv_obj_t* parent,
    int slot,
    AbstractCymbalsUI* ui,
    UIStyles& styles,
    lv_obj_t** widgets,
    lv_obj_t** labels,
    float* fParams,
    lv_event_cb_t valueCb,
    lv_event_cb_t dragCb
) {
    using P = PluginCymbals::Parameters;
    const int typeIdx   = P::kParamSlot1Type + slot * 16;
    const int wetIdx    = typeIdx + 14;
    const int bypassIdx = typeIdx + 15;

    const int currentTypeId = (int)std::lround(fParams[typeIdx]);
    const bool hasAnyFx = !fxTypeIsNone(currentTypeId);

    // 1. Outer panel
    lv_obj_t* panel = lv_obj_create(parent);
    lv_obj_add_style(panel, &styles.panel, 0);
    // Tighter than the kit's 10px plate inset: six plates plus the FX tail
    // must share the fixed non-scrolling viewport, and the 4px recovered per
    // side is invisible on these stacked cards.
    lv_obj_set_style_pad_all(panel, scaled(4), 0);
    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    // The panel can be TALLER than its content (the FX page gives the slot
    // grid the leftover height), so the content is centred in the card
    // instead of hugging the top: an empty slot then reads as a deliberate
    // drop target rather than a card with a hole under it.
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_width(panel, lv_pct(100));
    lv_obj_set_height(panel, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(panel, 0);
    lv_obj_set_user_data(panel, (void*)(intptr_t)(slot + 1));  // 1-indexed for retrieval
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    // 1a. Chain-flow tap: the serial chain position as a vertical azure
    // plug on the panel's LEFT edge — the upstream card's arrow lands on it
    // (and SLOT 1 gets the "chain start" back-tap), so the six plates read as
    // one connected serial bus. Brightened to LV_OPA_70 so the stub pair
    // across each 8px row gap reads as a continuous connector.
    lv_obj_t* flowTap = lv_obj_create(panel);
    lv_obj_remove_style_all(flowTap);
    lv_obj_set_size(flowTap, scaled(6), lv_pct(100));
    lv_obj_set_pos(flowTap, 0, 0);
    lv_obj_set_style_bg_color(flowTap, COL_HIGHLIGHT, 0);
    lv_obj_set_style_bg_opa(flowTap, (slot == 0) ? LV_OPA_90 : LV_OPA_70, 0);
    lv_obj_set_style_radius(flowTap, 0, 0);
    lv_obj_clear_flag(flowTap, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(flowTap, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_flag(flowTap, LV_OBJ_FLAG_HIDDEN);  // decorative blue rail (reported as awful)
    // 2. Header row (required 3-level drag hierarchy: handle → headerRow → panel)
    lv_obj_t* headerRow = UIWidgets::makeRow(panel);
    lv_obj_set_width(headerRow, lv_pct(100));
    // Fixed compact header: two-row knob grids need every vertical pixel.
    lv_obj_set_height(headerRow, scaled(24));
    lv_obj_set_flex_grow(headerRow, 0);
    lv_obj_set_flex_align(headerRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // The whole header is a drag handle: LVGL's hit test only targets
    // CLICKABLE objects, so without this flag presses land on the panel body.
    lv_obj_add_flag(headerRow, LV_OBJ_FLAG_CLICKABLE);
    // Deterministic gaps between the seven header members (makeRow leaves
    // pad_column at 0; without this the chip/grip/pill/labels would touch).
    lv_obj_set_style_pad_column(headerRow, scaled(6), 0);

    // 2a. Chain-position chip: two-digit serial index ("01".."06") — this
    // card's place in the chain (top row 1-3, bottom row 4-6). Accent when
    // a plugin is loaded, neutral chrome when empty. Non-clickable so header
    // drags pass through.
    lv_obj_t* slotChip = lv_obj_create(headerRow);
    lv_obj_set_size(slotChip, scaled(34), scaled(16));
    lv_obj_set_style_bg_color(slotChip, COL_PANEL_DARK, 0);
    lv_obj_set_style_bg_opa(slotChip, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(slotChip, scaled(4), 0);
    lv_obj_set_style_border_width(slotChip, 1, 0);
    lv_obj_set_style_pad_all(slotChip, scaled(2), 0);
    lv_obj_clear_flag(slotChip, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t* chipLbl = lv_label_create(slotChip);
    lv_obj_center(chipLbl);
    lv_obj_set_style_text_font(chipLbl, getScaledMicroFont(), 0);
    char slotChipText[16];
    snprintf(slotChipText, sizeof(slotChipText), "%02d", slot + 1);
    lv_label_set_text(chipLbl, slotChipText);
    if (!hasAnyFx) {
        lv_obj_set_style_text_color(chipLbl, COL_TEXT_DIM, 0);
        lv_obj_set_style_border_color(slotChip, COL_BORDER, 0);
        lv_obj_set_style_border_opa(slotChip, LV_OPA_40, 0);
    } else {
        lv_obj_set_style_text_color(chipLbl, COL_HIGHLIGHT, 0);
        lv_obj_set_style_border_color(slotChip, COL_HIGHLIGHT, 0);
        lv_obj_set_style_border_opa(slotChip, LV_OPA_70, 0);
    }

    // 2b. Drag handle button (grip control for reliable drag in LVGL 9)
    lv_obj_t* dragHandle = lv_btn_create(headerRow);
    lv_obj_set_size(dragHandle, scaled(36), scaled(22));
    styles.applyMomentaryButton(dragHandle);
    // The grip glyph fills the whole 36px box — theme padding would clip it.
    lv_obj_set_style_pad_all(dragHandle, 0, 0);
    lv_obj_add_flag(dragHandle, LV_OBJ_FLAG_PRESS_LOCK);

    // Child label with drag glyph (inherits the button's text color)
    lv_obj_t* dragGlyph = lv_label_create(dragHandle);
    lv_label_set_text(dragGlyph, LV_SYMBOL_MINUS LV_SYMBOL_MINUS LV_SYMBOL_MINUS);
    lv_obj_center(dragGlyph);
    uiAttachTooltipHoldable(dragHandle, "Drag to reorder");
    
    // Register drag events on the grip AND the whole header row, so a drag
    // can start anywhere on the header (buttons inside keep their own
    // handlers; a plain click never reorders because the 10px threshold
    // never trips). The callback walks up to the panel from any target.
    if (dragCb != nullptr) {
        lv_obj_add_event_cb(dragHandle, dragCb, LV_EVENT_PRESSED,   (void*)ui);
        lv_obj_add_event_cb(dragHandle, dragCb, LV_EVENT_PRESSING,  (void*)ui);
        lv_obj_add_event_cb(dragHandle, dragCb, LV_EVENT_RELEASED,  (void*)ui);
        lv_obj_add_event_cb(dragHandle, dragCb, LV_EVENT_PRESS_LOST, (void*)ui);
        lv_obj_add_event_cb(headerRow, dragCb, LV_EVENT_PRESSED,   (void*)ui);
        lv_obj_add_event_cb(headerRow, dragCb, LV_EVENT_PRESSING,  (void*)ui);
        lv_obj_add_event_cb(headerRow, dragCb, LV_EVENT_RELEASED,  (void*)ui);
        lv_obj_add_event_cb(headerRow, dragCb, LV_EVENT_PRESS_LOST, (void*)ui);
        lv_obj_add_event_cb(panel, dragCb, LV_EVENT_DELETE, (void*)ui);
    }

    // 2c. Bypass toggle button
    lv_obj_t* bypassBtn = lv_btn_create(headerRow);
    lv_obj_set_size(bypassBtn, scaled(60), scaled(22));
    lv_obj_add_flag(bypassBtn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_user_data(bypassBtn, (void*)(intptr_t)bypassIdx);
    lv_obj_t* bypassLbl = lv_label_create(bypassBtn);
    lv_obj_center(bypassLbl);
    lv_obj_add_event_cb(bypassBtn, valueCb, LV_EVENT_VALUE_CHANGED, (void*)ui);
    widgets[bypassIdx] = bypassBtn;
    labels[bypassIdx] = bypassLbl;

    if (!hasAnyFx) {
        // Empty slot: hide the bypass pill entirely. Registered in
        // labels[bypassIdx], it otherwise receives updateLabel()'s generic
        // "%.2f" fallback — the red "0.00" box the round-8 critic flagged as a
        // dead meter. Nothing on an empty card needs it; updatePlugin clears
        // the HIDDEN flag the moment a plugin lands on the slot.
        lv_obj_add_flag(bypassBtn, LV_OBJ_FLAG_HIDDEN);
        // Install the SAME toggle styles the populated branch uses, even while
        // hidden. A runtime-populated card (empty at build, algorithm chosen
        // later) otherwise has no local CHECKED style, and LVGL's default
        // theme paints a checked lv_button with its SECONDARY colour — the red
        // "ON" pill (reported). The disabled look below still dims it.
        styles.applyToggleButton(bypassBtn, false);
        lv_label_set_text(bypassLbl, "OFF");
        lv_obj_set_style_text_color(bypassLbl, COL_TEXT_DIM, 0);
        lv_obj_set_style_text_font(bypassLbl, getScaledSmallFont(), 0);
        lv_obj_set_style_border_color(bypassBtn, COL_BORDER, 0);
        lv_obj_set_style_border_width(bypassBtn, 1, 0);
        lv_obj_set_style_border_opa(bypassBtn, LV_OPA_40, 0);
        lv_obj_add_state(bypassBtn, LV_STATE_DISABLED);
        lv_obj_clear_flag(bypassBtn, LV_OBJ_FLAG_CLICKABLE);
    } else {
        styles.applyToggleButton(bypassBtn, fParams[bypassIdx] < 0.5f);
        lv_label_set_text(bypassLbl, "ON");
        lv_obj_set_style_text_color(bypassLbl, lv_color_white(), 0);
        lv_obj_set_style_text_font(bypassLbl, getScaledSmallFont(), 0);
        uiAttachTooltip(bypassBtn, "Bypass slot");
        // Solid amber when active (btnChecked has a vertical gradient otherwise)
        lv_obj_set_style_bg_grad_dir(bypassBtn, LV_GRAD_DIR_NONE, LV_STATE_CHECKED);
    }

    // 2d. Algorithm name label (flex-grow to fill space)
    lv_obj_t* algoLabel = lv_label_create(headerRow);
    lv_obj_add_style(algoLabel, &styles.label, 0);
    lv_obj_set_flex_grow(algoLabel, 1);
    // Ellipsis, never wrap: a long algorithm name must not grow the header
    // (the FX chain page is a fixed, non-scrolling viewport).
    lv_label_set_long_mode(algoLabel, LV_LABEL_LONG_DOT);
    {
        const char* algoName = hasAnyFx ? fxSurfaceName(currentTypeId) : "";
        lv_label_set_text(algoLabel, algoName);
        // Long names ("Dual Delay") get the micro tier so they stay on one
        // line in the narrow card header; short names keep the small face.
        const bool longName = algoName != nullptr && algoName[8] != '\0';
        lv_obj_set_style_text_font(algoLabel, longName ? uiMicroFont() : getScaledSmallFont(), 0);
    }
    labels[typeIdx] = algoLabel;  // stored so updatePlugin can reach it

    // 2e. Category badge
    lv_obj_t* catLabel = lv_label_create(headerRow);
    lv_obj_add_style(catLabel, &styles.labelSmall, 0);
    lv_obj_set_style_text_color(catLabel, COL_TEXT_DIM, 0);
    lv_obj_set_width(catLabel, scaled(64));
    lv_label_set_text(catLabel, hasAnyFx ? fxSurfaceCategory(currentTypeId) : "");

    // 2f. Remove [×] button
    lv_obj_t* removeBtn = lv_btn_create(headerRow);
    lv_obj_set_size(removeBtn, scaled(32), scaled(22));
    styles.applyMomentaryButton(removeBtn);
    lv_obj_set_style_pad_all(removeBtn, 0, 0);
    lv_obj_set_user_data(removeBtn, (void*)(intptr_t)typeIdx);
    lv_obj_t* removeLbl = lv_label_create(removeBtn);
    lv_label_set_text(removeLbl, LV_SYMBOL_CLOSE);
    lv_obj_center(removeLbl);
    uiAttachTooltip(removeBtn, "Remove plugin");
    lv_obj_add_event_cb(removeBtn, onRemoveClicked, LV_EVENT_CLICKED, (void*)ui);

    // 2g. Header flow marker: a small dim chevron at the header's right end.
    // The authoritative signal is the card-edge plug pair below (out-stub →
    // next card's in-tap); this keeps the header strip echoing the same flow
    // language so the card reads as a chain node even at a glance.
    lv_obj_t* flowArrow = lv_label_create(headerRow);
    lv_obj_set_style_text_font(flowArrow, getScaledFont(), 0);
    lv_obj_set_style_text_color(flowArrow, COL_HIGHLIGHT, 0);
    lv_obj_set_style_text_opa(flowArrow, LV_OPA_70, 0);
    lv_obj_clear_flag(flowArrow, LV_OBJ_FLAG_SCROLLABLE);
    lv_label_set_text(flowArrow, LV_SYMBOL_RIGHT);

    // 2h. Card-edge flow stub: this card's serial OUT (right edge, bright —
    // the last slot feeds the FX TAIL). Meets the next card's flowTap across
    // the 4px row gap, drawing a continuous connector.
    lv_obj_t* flowStub = lv_obj_create(panel);
    lv_obj_remove_style_all(flowStub);
    const bool isLast = (slot == 5);
    lv_obj_set_size(flowStub, scaled(6), lv_pct(100));
    lv_obj_set_pos(flowStub, lv_pct(100), 0);
    lv_obj_set_style_bg_color(flowStub, COL_HIGHLIGHT, 0);
    lv_obj_set_style_bg_opa(flowStub, isLast ? LV_OPA_90 : LV_OPA_70, 0);
    lv_obj_set_style_radius(flowStub, 0, 0);
    lv_obj_clear_flag(flowStub, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(flowStub, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_flag(flowStub, LV_OBJ_FLAG_HIDDEN);  // decorative blue rail (reported as awful)

    // 3. Empty row — the whole card body is the click target. "EMPTY" reads
    // big and dim (chrome, not accent) with a subdued "ADD" chip below it:
    // a quiet insert point, not a loud CTA. The arc row stays hidden so no
    // meter chrome competes.
    lv_obj_t* emptyRow = UIWidgets::makeRow(panel);
    lv_obj_set_width(emptyRow, lv_pct(100));
    // Fill the card body: the empty slot is one big add target, with EMPTY /
    // ADD centred in the leftover height instead of hugging the header.
    lv_obj_set_height(emptyRow, 0);
    lv_obj_set_flex_grow(emptyRow, 1);
    lv_obj_set_flex_align(emptyRow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_user_data(emptyRow, (void*)(intptr_t)slot);  // slot index for picker
    lv_obj_set_style_bg_opa(emptyRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(emptyRow, 0, 0);
    lv_obj_set_style_pad_all(emptyRow, scaled(4), 0);
    lv_obj_set_style_pad_row(emptyRow, scaled(3), 0);
    uiLockScroll(emptyRow);
    lv_obj_add_flag(emptyRow, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(emptyRow, onAddClicked, LV_EVENT_CLICKED, (void*)ui);

    // "EMPTY" in big chrome — unmistakable, not a placeholder. The old 40%
    // of a dim colour made the slot read as a VOID rather than an affordance
    // (round-3 critic: "all six slots show EMPTY in dark grey text that
    // barely contrasts with the background"), so the label now sits at a
    // legible tier and the ADD chip keeps a clear outline.
    lv_obj_t* emptyLbl = lv_label_create(emptyRow);
    lv_obj_set_style_text_font(emptyLbl, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(emptyLbl, COL_TEXT, 0);
    lv_obj_set_style_text_opa(emptyLbl, LV_OPA_70, 0);
    lv_obj_set_style_text_letter_space(emptyLbl, scaled(2), 0);
    lv_label_set_text(emptyLbl, "EMPTY");
    lv_obj_clear_flag(emptyLbl, LV_OBJ_FLAG_SCROLLABLE);

    // Subdued "ADD" chip — hairline outline, dim accent, micro caps.
    lv_obj_t* addChip = lv_label_create(emptyRow);
    lv_obj_set_style_text_color(addChip, COL_HIGHLIGHT, 0);
    lv_obj_set_style_text_opa(addChip, LV_OPA_80, 0);
    lv_obj_set_style_text_font(addChip, getScaledMicroFont(), 0);
    lv_obj_set_style_text_letter_space(addChip, scaled(1), 0);
    lv_obj_set_style_bg_opa(addChip, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(addChip, COL_HIGHLIGHT, 0);
    lv_obj_set_style_border_width(addChip, 1, 0);
    lv_obj_set_style_border_opa(addChip, LV_OPA_70, 0);
    lv_obj_set_style_radius(addChip, scaled(3), 0);
    lv_obj_set_style_pad_hor(addChip, scaled(5), 0);
    lv_obj_set_style_pad_ver(addChip, scaled(1), 0);
    lv_label_set_text(addChip, "ADD");
    lv_obj_clear_flag(addChip, LV_OBJ_FLAG_SCROLLABLE);

    if (hasAnyFx) {
        lv_obj_add_flag(emptyRow, LV_OBJ_FLAG_HIDDEN);
    }

    // 4. Param arc row (10 arcs, hidden if param count < 10 or not loaded)
    // Gap between arc cells, in scaled px. Named because the fit arithmetic
    // below must subtract the gaps before dividing by the arc count.
    // DESIGN-space px (1280-wide canvas), applied through scaled() at use.
    static constexpr int kFxArcGapPx = 6;
    lv_obj_t* arcRow = lv_obj_create(panel);
    lv_obj_add_style(arcRow, &styles.card, 0);
    lv_obj_set_layout(arcRow, LV_LAYOUT_FLEX);
    // ROW while the cells stay readable, two rows when an algorithm carries
    // more knobs than fit (10 params + WET). Wrapping used to be banned, which
    // shrank cells to the floor and still hid the right-hand parameters on
    // wide Airwindows algorithms. The FX TAIL card is compact so the slot
    // rows now have the height for two rows with names + values.
    // The arc row is a nested card inside the panel card; keeping styles.card's
    // padding here stacked a second inset and cost ~20px of the row, which is
    // exactly what pushed the 7th knob over. Keep the card look, drop the inset.
    lv_obj_set_style_pad_all(arcRow, 0, 0);
    lv_obj_set_style_pad_row(arcRow, 0, 0);
    // pad_column is a SEPARATE flex-gap property - styles.card sets pad_row but
    // leaves pad_column to the default, which measured ~11px per cell in a
    // render and was what actually pushed the 7th arc off the card (not the
    // cell width). Pin it so the fit arithmetic below is exact.
    lv_obj_set_style_pad_column(arcRow, scaled(kFxArcGapPx), 0);
    lv_obj_set_width(arcRow, lv_pct(100));
    lv_obj_set_height(arcRow, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(arcRow, 0);
    lv_obj_set_user_data(arcRow, (void*)(intptr_t)0xAA);  // sentinel: "arc row"

    if (!hasAnyFx) {
        lv_obj_add_flag(arcRow, LV_OBJ_FLAG_HIDDEN);
    }

    populateFxArcRow(arcRow, typeIdx, wetIdx, currentTypeId, ui, styles,
                     widgets, labels, fParams, valueCb);

    return panel;
}

inline void UIModularSlotSection::updatePlugin(
    lv_obj_t* panel,
    int newType,
    AbstractCymbalsUI* ui,
    UIStyles& styles,
    lv_obj_t** widgets,
    lv_obj_t** labels,
    float* fParams,
    lv_event_cb_t valueCb
) {
    using P = PluginCymbals::Parameters;

    // Recover slot from user_data
    int slot = getSlotIndex(panel);
    if (slot < 0 || slot > 5) return;
    const int typeIdx = P::kParamSlot1Type + slot * 16;
    const bool hasPlugin = !fxTypeIsNone(newType);

    // Update algorithm name label (stored in labels[typeIdx])
    if (labels[typeIdx]) {
        const char* algoName = hasPlugin ? fxSurfaceName(newType) : "";
        lv_label_set_text(labels[typeIdx], algoName);
        const bool longName = algoName != nullptr && algoName[8] != '\0';
        lv_obj_set_style_text_font(labels[typeIdx], longName ? uiMicroFont() : getScaledSmallFont(), 0);
    }

    // Get category label. headerRow children:
    // 0=slotChip, 1=dragHandle, 2=bypassBtn, 3=algoLabel, 4=catLabel,
    // 5=removeBtn, 6=flowArrow.
    // REAL panel child order: 0=flowTap, 1=headerRow, 2=flowStub,
    // 3=emptyRow, 4=arcRow, 5=floating chain caption. (flowStub is created
    // after the header; the old comment/index claimed 2=headerRow, so this
    // function was updating the hidden rail instead of the chip/category.)
    lv_obj_t* headerRow = lv_obj_get_child(panel, 1);
    if (headerRow) {
        // Chain-position chip: accent when populated, neutral when empty.
        lv_obj_t* slotChip = lv_obj_get_child(headerRow, 0);
        if (slotChip) {
            lv_obj_t* chipLbl = lv_obj_get_child(slotChip, 0);
            if (chipLbl) {
                lv_obj_set_style_text_color(chipLbl,
                    hasPlugin ? COL_HIGHLIGHT : COL_TEXT_DIM, 0);
            }
            lv_obj_set_style_border_color(slotChip,
                hasPlugin ? COL_HIGHLIGHT : COL_BORDER, 0);
            lv_obj_set_style_border_opa(slotChip,
                hasPlugin ? LV_OPA_70 : LV_OPA_40, 0);
        }

        lv_obj_t* catLbl = lv_obj_get_child(headerRow, 4);  // catLabel is child index 4
        if (catLbl) {
            lv_label_set_text(catLbl, hasPlugin ? fxSurfaceCategory(newType) : "");
        }
    }

    // Panel children: 0=flowTap, 1=headerRow, 2=flowStub, 3=emptyRow, 4=arcRow
    lv_obj_t* emptyRow = lv_obj_get_child(panel, 3);
    lv_obj_t* arcRow = lv_obj_get_child(panel, 4);

    if (emptyRow) {
        if (hasPlugin) lv_obj_add_flag(emptyRow, LV_OBJ_FLAG_HIDDEN);
        else           lv_obj_clear_flag(emptyRow, LV_OBJ_FLAG_HIDDEN);
    }

    // Update bypass button: hidden when empty, active ON toggle when populated
    if (headerRow) {
        lv_obj_t* bypassBtn = lv_obj_get_child(headerRow, 2);  // bypassBtn is child index 2
        if (bypassBtn) {
            lv_obj_t* bypassLbl = lv_obj_get_child(bypassBtn, 0);
            if (hasPlugin) {
                lv_obj_clear_flag(bypassBtn, LV_OBJ_FLAG_HIDDEN);
                lv_label_set_text(bypassLbl, "ON");
                lv_obj_set_style_text_color(bypassLbl, lv_color_white(), 0);
                lv_obj_set_style_text_font(bypassLbl, getScaledSmallFont(), 0);
                lv_obj_clear_state(bypassBtn, LV_STATE_DISABLED);
                lv_obj_add_flag(bypassBtn, LV_OBJ_FLAG_CLICKABLE);
                lv_obj_set_style_bg_grad_dir(bypassBtn, LV_GRAD_DIR_NONE, LV_STATE_CHECKED);
            } else {
                lv_obj_add_flag(bypassBtn, LV_OBJ_FLAG_HIDDEN);
                lv_label_set_text(bypassLbl, "OFF");
                lv_obj_set_style_text_color(bypassLbl, COL_TEXT_DIM, 0);
                lv_obj_set_style_text_font(bypassLbl, getScaledSmallFont(), 0);
                lv_obj_add_state(bypassBtn, LV_STATE_DISABLED);
                lv_obj_clear_flag(bypassBtn, LV_OBJ_FLAG_CLICKABLE);
            }
        }
    }

    // Rebuild the knob grid for the new algorithm. The old arcs' registry
    // slots are cleared FIRST: lv_obj_clean destroys them, and with
    // LV_USE_CHECK_OBJ_VALIDITY=0 a stale widgets[]/labels[] entry would be
    // dereferenced by updateLabel/syncParamWidget.
    // The floating chain-position caption is needed only on EMPTY cards: on a
    // populated card it collides with the second knob row's value line
    // (reported as cut-off labels on ToTape9/ChimeyDeluxe).
    if (lv_obj_t* caption = lv_obj_get_child(panel, 5)) {
        if (hasPlugin) lv_obj_add_flag(caption, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_clear_flag(caption, LV_OBJ_FLAG_HIDDEN);
    }
    if (arcRow != nullptr) {
        for (int i = 0; i < 13; ++i) {
            widgets[typeIdx + 1 + i] = nullptr;
            labels[typeIdx + 1 + i] = nullptr;
        }
        widgets[typeIdx + 14] = nullptr;   // WET
        labels[typeIdx + 14] = nullptr;
        populateFxArcRow(arcRow, typeIdx, typeIdx + 14, newType, ui, styles,
                         widgets, labels, fParams, valueCb);
    }
}

inline void UIModularSlotSection::onRemoveClicked(lv_event_t* e) {
    AbstractCymbalsUI* ui = (AbstractCymbalsUI*)lv_event_get_user_data(e);
    if (!ui) return;
    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    int typeIdx = (int)(intptr_t)lv_obj_get_user_data(btn);

    const int slot = (typeIdx - (int)PluginCymbals::kParamSlot1Type) / 16;

    // Clear type (NONE=0) — drops this slot from the audio route
    ui->editParameter((uint32_t)typeIdx, true);
    ui->setParamValue((uint32_t)typeIdx, 0.0f);
    ui->editParameter((uint32_t)typeIdx, false);
    
    // Reset bypass so the slot starts clean when the next plugin is added
    ui->editParameter((uint32_t)(typeIdx + 15), true);
    ui->setParamValue((uint32_t)(typeIdx + 15), 0.0f);
    ui->editParameter((uint32_t)(typeIdx + 15), false);

    // Guarantee panel visual update regardless of whether parameterChanged fires
    ui->notifySlotTypeChanged(slot, 0);
}

inline void UIModularSlotSection::onAddClicked(lv_event_t* e) {
    AbstractCymbalsUI* ui = (AbstractCymbalsUI*)lv_event_get_user_data(e);
    if (!ui) return;
    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    int slot = (int)(intptr_t)lv_obj_get_user_data(btn);
    ui->openPickerForSlot(slot);
}

END_NAMESPACE_DISTRHO
