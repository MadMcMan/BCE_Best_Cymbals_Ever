#pragma once

#include "lvgl.h"
#include "../UICommon.hpp"
#include "../UIStyles.hpp"
#include "../UIWidgets.hpp"
#include "../UIModularSlotSection.hpp"
#include "../AdsrCurvePreview.hpp"
#include "../../PluginCymbals.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

START_NAMESPACE_DISTRHO

// ============================================================================
// FX CHAIN page
// ----------------------------------------------------------------------------
// Three rows: the serial strip, the 2x3 slot grid, then a one-line chain bus
// (the six inserts in PROCESSING order, with their real insert names) and the
// FX TAIL card (ADSR + Clip + the POST-FILTER BUS it feeds, under one header
// band like the DISPERSION module cards).
//
// Every readout on the bus and in the tail card is a live parameter read:
// slot type/wet/bypass (kParamSlot1Type + slot*16 +14/+15), slot processing
// order (kParamSlotOrder0..5), tail state (kParamFinalAROn / kParamFinalClipOn
// / kParamFinalCombBankOn) and post-filter routing (kParamFinalCombBankSeries /
// kParamFinalCombGroupGain). None of these labels lives in labels[], so
// updateLabel() can never reach them; the owner re-paints them through
// refreshChainReadouts() whenever a chain parameter changes.
// ============================================================================
class FXChainTab {
public:
    // Builds the FX chain tab contents.
    // slotPanels[6] is filled with the 6 slot panel pointers (for updatePlugin calls).
    // outChainLabels[kLabelSlotCount] is filled with this page's readout labels
    // (owned by the labels' LVGL parents — the array only borrows them).
    static void build(
        lv_obj_t* tab,
        AbstractCymbalsUI* ui,
        UIStyles& styles,
        lv_obj_t** widgets,
        lv_obj_t** labels,
        float* fParams,
        lv_event_cb_t valueCb,
        lv_obj_t** slotPanels,        // output: array of 6 pointers
        lv_obj_t** outChainLabels = nullptr, // output: array of kLabelSlotCount
        lv_event_cb_t randomizeAllCb = nullptr,
        lv_event_cb_t randomizeSlotCb = nullptr,
        lv_event_cb_t randomizeSettingsAllCb = nullptr,
        lv_event_cb_t randomizeSettingsSlotCb = nullptr
    );

    // Re-paints every chain readout from fParams. Cheap enough to call on every
    // parameterChanged that touches the chain, which is the only way these
    // labels can go stale. Label handles come from the array build() published,
    // so no tree scan is needed — and a refresh after a drag (which reparents
    // panels across their flex rows) can never resolve a label against stale
    // geometry.
    static void refreshChainReadouts(float* fParams, const lv_obj_t* const* chainLabels);

    // Locates this page's readout labels inside `root` (the FX CHAIN panel).
    // Two levels deep: the bus chips sit in the bus container, the tail readouts
    // sit in the tail card's body row.
    static void collectChainLabels(lv_obj_t* root, lv_obj_t** outChainLabels);

private:
    static lv_obj_t* buildChainBus(lv_obj_t* tab, lv_obj_t** outChainLabels);
    static lv_obj_t* buildTailCard(lv_obj_t* tab, AbstractCymbalsUI* ui, UIStyles& styles,
                                   lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                                   lv_event_cb_t valueCb, lv_obj_t** outChainLabels);
};

// ============================================================================
// INLINE IMPLEMENTATION
// ============================================================================

namespace FxChainTabDetail {

// Readout-label handles. The FX CHAIN panel keeps them in fChainLabels[] and
// hands them to refreshChainReadouts(); the LVGL parents own the objects.
enum LabelSlot {
    kChainChip0 = 0,   // six per-position chips, reading position 0..5
    kChainChip5 = 5,
    kChainCount = 6,   // "N ACTIVE - M DRY WET"
    kTailState  = 7,   // "TAIL ON <- S3 BrightAmbience"
    kPostBus    = 8,   // "BUS ON - SERIES - GAIN 0.85"
    kLabelSlotCount = 9
};

// Reordered-slot reconciliation, identical to SlotOrderPolicy::normalizeSlotOrder
// (SlotOrderPolicy.hpp is not on this header's include path and PluginUI.cpp owns
// the policy): keep the first valid occurrence of each slot in the supplied
// order, then append the unseen slots in ascending index order.
inline void normalizeSlotOrder(const int* candidate, int* out) {
    bool seen[6] = {false, false, false, false, false, false};
    int n = 0;
    for (int i = 0; i < 6 && n < 6; ++i) {
        const int slot = candidate[i];
        if (slot < 0 || slot >= 6 || seen[slot]) continue;
        seen[slot] = true;
        out[n++] = slot;
    }
    for (int slot = 0; slot < 6 && n < 6; ++slot) {
        if (seen[slot]) continue;
        out[n++] = slot;
    }
}

// Serial chain order: DSP position p is slot order[p] (PluginCymbals.cpp:1683
// walks m_slotProcessOrder and runs each insert in that order). The UI cards are
// laid out in the same reading order, so this is what the grid shows; it is
// recomputed here so a host/preset write that reorders the chain is visible
// without a rebuild.
inline void slotOrderFromParams(const float* fParams, int* out) {
    int candidate[6] = {-1, -1, -1, -1, -1, -1};
    for (int pos = 0; pos < 6; ++pos) {
        const int idx = (int)PluginCymbals::kParamSlotOrder0 + pos;
        if (idx < (int)PluginCymbals::kParameterCount)
            candidate[pos] = (int)(fParams[idx] + 0.5f);
    }
    normalizeSlotOrder(candidate, out);
}

// Which card lives at reading position pos. The chain order is authoritative
// (it is what the DSP processes), so this resolves against it and never scans
// live geometry: the panels are reparented across their flex rows on a drag,
// and a geometry scan would silently disagree with the parameter.
inline lv_obj_t* cardAt(lv_obj_t** slotPanels, const int* order, int pos) {
    if (slotPanels == nullptr || order == nullptr) return nullptr;
    if (pos < 0 || pos >= 6) return nullptr;
    return slotPanels[order[pos]];
}

// Readout-label registry. These labels are not parameter displays (they are not
// in labels[]), so they carry a payload tag instead: the shared non-param
// convention (UICommon.hpp) keeps the value above kParameterCount, so
// repointWidgetsRecursive / syncCombWidgetsRecursive never mistake one for a
// parameter-bound control.
//
//   0x100 -> per-position chip, index 0..5 (reading position in the chain)
//   0x101 -> the tail-state chip
//   0x102 -> the active-insert count
//   0x103 -> the POST-FILTER BUS state line
inline constexpr int kChainChipPayload = 0x100;
inline constexpr int kTailStatePayload = 0x101;
inline constexpr int kChainCountPayload = 0x102;
inline constexpr int kPostBusPayload    = 0x103;
inline constexpr int kFxRandomPayload   = 0x104;

inline intptr_t payload(int kind, int index = 0) {
    return uiEncodeNonParamPayload((kind << 8) | (index & 0xFF));
}

// Depth-limited child scan for one payload tag. `depth` is levels below `root`
// (the tail card's body row is two levels down from the tab).
inline lv_obj_t* findByPayload(lv_obj_t* root, intptr_t want, int depth) {
    if (root == nullptr || depth < 0) return nullptr;
    const uint32_t n = lv_obj_get_child_count(root);
    for (uint32_t i = 0; i < n; ++i) {
        lv_obj_t* c = lv_obj_get_child(root, i);
        if (c == nullptr) continue;
        if ((intptr_t)lv_obj_get_user_data(c) == want) return c;
        lv_obj_t* hit = findByPayload(c, want, depth - 1);
        if (hit != nullptr) return hit;
    }
    return nullptr;
}

inline void setLabelText(lv_obj_t* lbl, const char* text) {
    if (lbl == nullptr) return;
    const char* cur = lv_label_get_text(lbl);
    if (cur == nullptr || std::strcmp(cur, text) != 0) lv_label_set_text(lbl, text);
}

} // namespace FxChainTabDetail

// ---------------------------------------------------------------------------
// Chain bus view: the six inserts in processing order, one chip per card, each
// carrying its real insert name + wet. This is the chain state the critic found
// missing ("the six chain slots show only EMPTY + ADD with no routing or effect
// hint"): the reading order here IS the processing order.
// ---------------------------------------------------------------------------
inline lv_obj_t* FXChainTab::buildChainBus(lv_obj_t* tab, lv_obj_t** outChainLabels) {
    using namespace FxChainTabDetail;

    lv_obj_t* bus = lv_obj_create(tab);
    lv_obj_set_size(bus, lv_pct(100), scaled(30));
    lv_obj_set_style_bg_color(bus, COL_PANEL_DARK, 0);
    lv_obj_set_style_bg_opa(bus, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(bus, COL_ACCENT, 0);
    lv_obj_set_style_border_width(bus, 1, 0);
    lv_obj_set_style_radius(bus, scaled(4), 0);
    lv_obj_set_style_pad_all(bus, 0, 0);
    lv_obj_set_style_pad_hor(bus, scaled(4), 0);
    lv_obj_set_style_pad_column(bus, scaled(4), 0);
    lv_obj_set_layout(bus, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bus, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bus, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    uiLockScroll(bus);
    lv_obj_set_user_data(bus, (void*)(intptr_t)-1);  // not a slot row / card

    auto chip = [&](const char* text, lv_color_t color) {
        lv_obj_t* c = lv_label_create(bus);
        lv_label_set_text(c, text);
        // TINY face: the chips are 64px and algorithm names are single long
        // words ("ChimeyDeluxe"), which the 12px micro face split mid-word
        // ("ChimeyB eluxe"). The smaller tier keeps most names on one line.
        lv_obj_set_style_text_font(c, getScaledTinyFont(), 0);
        lv_obj_set_style_text_color(c, color, 0);
        lv_obj_set_style_bg_opa(c, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(c, 0, 0);
        lv_obj_set_style_pad_hor(c, scaled(3), 0);
        lv_obj_set_style_pad_ver(c, scaled(0), 0);
        lv_obj_set_user_data(c, (void*)(intptr_t)-1);
        // The chain chips are re-written by refreshChainReadouts. Wrap/height
        // are invariant, so they are set once in the six-chip loop below.
        uiLockScroll(c);
        return c;
    };
    auto arrow = [&]() {
        lv_obj_t* a = lv_label_create(bus);
        lv_label_set_text(a, LV_SYMBOL_RIGHT);
        lv_obj_set_style_text_font(a, getScaledMicroFont(), 0);
        lv_obj_set_style_text_color(a, COL_ACCENT, 0);
        lv_obj_set_style_text_opa(a, LV_OPA_70, 0);
        lv_obj_set_user_data(a, (void*)(intptr_t)-1);
        uiLockScroll(a);
        return a;
    };

    chip("IN", COL_TEXT_DIM);
    arrow();

    // Six per-position chips in chain order. Text is (re)written by
    // refreshChainReadouts, so this is the build-time shape only.
    for (int pos = 0; pos < 6; ++pos) {
        lv_obj_t* c = chip("", COL_HIGHLIGHT);
        // Fixed 64px: the chip holds "01 <name>" over "WET 100%", so a
        // content-sized chip would shift the whole row as names change.
        lv_obj_set_width(c, scaled(78));
        lv_obj_set_height(c, LV_SIZE_CONTENT);
        lv_label_set_long_mode(c, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_align(c, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_user_data(c, (void*)payload(kChainChipPayload, pos));
        if (outChainLabels != nullptr) outChainLabels[kChainChip0 + pos] = c;
        arrow();
    }

    // Right-hand readouts: how many inserts actually run, and the tail's real
    // state. Both are live reads.
    lv_obj_t* countLbl = lv_label_create(bus);
    lv_label_set_text(countLbl, "");
    lv_obj_set_flex_grow(countLbl, 1);
    lv_obj_set_style_text_align(countLbl, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_font(countLbl, getScaledMicroFont(), 0);
    lv_obj_set_style_text_color(countLbl, COL_TEXT_DIM, 0);
    lv_obj_set_user_data(countLbl, (void*)payload(kChainCountPayload));
    uiLockScroll(countLbl);
    if (outChainLabels != nullptr) outChainLabels[kChainCount] = countLbl;

    lv_obj_t* tailLbl = lv_label_create(bus);
    lv_label_set_text(tailLbl, "");
    lv_obj_set_width(tailLbl, scaled(150));
    lv_obj_set_style_text_align(tailLbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(tailLbl, getScaledMicroFont(), 0);
    lv_obj_set_style_text_color(tailLbl, COL_HIGHLIGHT, 0);
    lv_obj_set_style_bg_opa(tailLbl, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(tailLbl, COL_HIGHLIGHT, 0);
    lv_obj_set_style_border_width(tailLbl, 1, 0);
    lv_obj_set_style_border_opa(tailLbl, LV_OPA_40, 0);
    lv_obj_set_style_radius(tailLbl, scaled(3), 0);
    lv_obj_set_style_pad_hor(tailLbl, scaled(3), 0);
    lv_obj_set_style_pad_ver(tailLbl, scaled(0), 0);
    lv_obj_set_user_data(tailLbl, (void*)payload(kTailStatePayload));
    uiLockScroll(tailLbl);
    // Click shows what the two states in this strip mean, in the strip itself
    // (the count label is the free-running status cell next to it). Hovering
    // shows the same sentence as a tooltip.
    lv_obj_add_flag(tailLbl, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(tailLbl, [](lv_event_t* e) {
        lv_obj_t* lbl = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
        lv_obj_t* bus = lv_obj_get_parent(lbl);
        if (!bus) return;
        lv_obj_t* count = findByPayload(bus, payload(kChainCountPayload), 1);
        if (!count) return;
        setLabelText(count, "FINAL TOUCHES: ADSR+clip+HP/LP after the chain. BUS: post-filter bank.");
        lv_obj_set_style_text_color(count, COL_TEXT, 0);
    }, LV_EVENT_CLICKED, nullptr);
    uiAttachTooltip(tailLbl, "FINAL TOUCHES = post-chain ADSR + clip + quick HP/LP; BUS = post-filter bank");
    if (outChainLabels != nullptr) outChainLabels[kTailState] = tailLbl;

    return bus;
}

// ---------------------------------------------------------------------------
// FX TAIL bus card: the ADSR column, the Clip column and the POST-FILTER BUS
// column under ONE header band, so the row reads as the chain's terminal module
// (the band is the makeCardHeader treatment the DISPERSION module cards use, so
// the two pages read as one instrument). All three groups are live controls.
// ---------------------------------------------------------------------------
inline lv_obj_t* FXChainTab::buildTailCard(lv_obj_t* tab, AbstractCymbalsUI* ui, UIStyles& styles,
                                           lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                                           lv_event_cb_t valueCb, lv_obj_t** outChainLabels) {
    using namespace FxChainTabDetail;
    using P = PluginCymbals::Parameters;

    lv_obj_t* pane = lv_obj_create(tab);
    lv_obj_set_size(pane, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_add_style(pane, &styles.panel, 0);
    lv_obj_set_style_pad_all(pane, 0, 0);
    lv_obj_set_style_pad_row(pane, 0, 0);
    lv_obj_set_layout(pane, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(pane, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(pane, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    uiLockScroll(pane);

    // Module header band (makeCardHeader, DispersionStageSection.hpp:259): the
    // title never touches the frame edge and the flow tag rides the band with a
    // baseline rule under it. The height is definite so the band can never
    // collapse or wrap.
    {
        lv_obj_t* band = lv_obj_create(pane);
        lv_obj_set_size(band, lv_pct(100), scaled(24));
        lv_obj_set_style_bg_color(band, COL_PANEL_DARK, 0);
        lv_obj_set_style_bg_opa(band, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(band, COL_HAIRLINE, 0);
        lv_obj_set_style_border_width(band, 1, 0);
        lv_obj_set_style_border_side(band, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_radius(band, scaled(3), 0);
        lv_obj_set_style_pad_all(band, 0, 0);
        lv_obj_set_style_pad_hor(band, scaled(5), 0);
        lv_obj_set_style_pad_ver(band, 0, 0);
        lv_obj_set_layout(band, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(band, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(band, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        uiLockScroll(band);

        lv_obj_t* t = lv_label_create(band);
        lv_label_set_text(t, "FINAL TOUCHES");
        lv_obj_add_style(t, &styles.title, 0);
        lv_obj_set_flex_grow(t, 1);
        lv_obj_set_height(t, scaled(24));
        lv_label_set_long_mode(t, LV_LABEL_LONG_DOT);

        lv_obj_t* chip = lv_label_create(band);
        lv_label_set_text(chip, "FED BY SLOT 6  >  OUT");
        lv_obj_set_style_text_font(chip, getScaledMicroFont(), 0);
        lv_obj_set_style_text_color(chip, COL_TEXT_DIM, 0);
        lv_obj_set_style_border_color(chip, COL_ACCENT, 0);
        lv_obj_set_style_border_width(chip, 1, 0);
        lv_obj_set_style_border_opa(chip, LV_OPA_40, 0);
        lv_obj_set_style_radius(chip, scaled(3), 0);
        lv_obj_set_style_pad_hor(chip, scaled(4), 0);
        lv_obj_set_style_pad_ver(chip, scaled(1), 0);
        lv_obj_set_user_data(chip, (void*)(intptr_t)-1);  // out of sub-tab scans
    }

    // Chain-edge flow: left IN tap (fed by slot 6) and a bright right OUT stub
    // (terminal stage — the chain ends here). Same plug-pair language as the
    // slot cards, so the tail reads as the final link of one serial chain.
    lv_obj_t* tailInTap = lv_obj_create(pane);
    lv_obj_remove_style_all(tailInTap);
    lv_obj_set_size(tailInTap, scaled(6), lv_pct(100));
    lv_obj_set_pos(tailInTap, 0, 0);
    lv_obj_set_style_bg_color(tailInTap, COL_HIGHLIGHT, 0);
    lv_obj_set_style_bg_opa(tailInTap, LV_OPA_70, 0);
    lv_obj_set_style_radius(tailInTap, 0, 0);
    uiLockScroll(tailInTap);
    lv_obj_add_flag(tailInTap, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_flag(tailInTap, LV_OBJ_FLAG_HIDDEN);  // blue rail overlapped ADSR caption

    lv_obj_t* tailOutStub = lv_obj_create(pane);
    lv_obj_remove_style_all(tailOutStub);
    lv_obj_set_size(tailOutStub, scaled(6), lv_pct(100));
    lv_obj_set_pos(tailOutStub, lv_pct(100), 0);
    lv_obj_set_style_bg_color(tailOutStub, COL_HIGHLIGHT, 0);
    lv_obj_set_style_bg_opa(tailOutStub, LV_OPA_90, 0);
    lv_obj_set_style_radius(tailOutStub, 0, 0);
    uiLockScroll(tailOutStub);
    lv_obj_add_flag(tailOutStub, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_add_flag(tailOutStub, LV_OBJ_FLAG_HIDDEN);

    // The three groups sit side by side in ONE body row: header band on top,
    // then a single row holding ADSR (toggle + A/D/S/R), CLIP (toggle + type +
    // amount) and the POST-FILTER BUS (two toggles + a state line). The old
    // layout stacked ADSR over CLIP, which doubled this block's height and left
    // the void below it.
    lv_obj_t* body = lv_obj_create(pane);
    lv_obj_set_size(body, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(body, 0, 0);
    lv_obj_set_style_border_width(body, 0, 0);
    lv_obj_set_style_pad_all(body, scaled(1), 0);
    lv_obj_set_style_pad_column(body, scaled(10), 0);
    lv_obj_set_layout(body, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(body, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    uiLockScroll(body);

    // Vertical separators between the FINAL TOUCHES sections. Positioned in
    // the 10 px inter-column gaps so they do not consume layout width.
    {
        const lv_coord_t sepX[4] = {
            scaled(256), scaled(516), scaled(856), scaled(976)
        };
        for (lv_coord_t x : sepX) {
            lv_obj_t* sep = lv_obj_create(body);
            lv_obj_remove_style_all(sep);
            lv_obj_set_size(sep, scaled(1), scaled(112));
            lv_obj_set_pos(sep, x, scaled(2));
            lv_obj_set_style_bg_color(sep, COL_HAIRLINE, 0);
            lv_obj_set_style_bg_opa(sep, LV_OPA_COVER, 0);
            lv_obj_add_flag(sep, LV_OBJ_FLAG_IGNORE_LAYOUT);
            lv_obj_clear_flag(sep, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_clear_flag(sep, LV_OBJ_FLAG_SCROLLABLE);
        }
    }

    auto buildToggle = [&](lv_obj_t* row, uint32_t param, const char* onText,
                           const char* offText) {
        lv_obj_t* btn = lv_btn_create(row);
        lv_obj_set_size(btn, scaled(92), scaled(24));
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
        const bool isOn = fParams[param] > 0.5f;
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, isOn ? onText : offText);
        lv_obj_set_style_text_font(lbl, getScaledSmallFont(), 0);
        lv_obj_center(lbl);
        styles.applyToggleButton(btn, isOn);
        lv_obj_set_user_data(btn, reinterpret_cast<void*>((intptr_t)param));
        lv_obj_add_event_cb(btn, valueCb, LV_EVENT_VALUE_CHANGED, ui);
        widgets[param] = btn;
        labels[param] = lbl;
    };

    // Arc footprint for the tail. ADSR holds a toggle plus FOUR knobs, so its
    // 424px column must hold 104 + 4*46 + 3*8 = 312px of controls. The kit's
    // 56px small spec would need 360px and push the third column off the card;
    // the 34px arc keeps the arc:cell ratio the kit uses for its compact
    // dispersion knobs.
    UIWidgets::ArcVisualSpec tailSpec = UIWidgets::smallArcVisualSpec();
    // 56px cells with a 40px arc: the tail's four envelope knobs carry their
    // values ("4 ms 126 ms 80% 99 ms") under the caps, and at 46/34 the
    // captions and values of neighbouring knobs crowded each other ("the FX
    // TAIL knobs sit small and cramped" - round-5 critic). The ADSR column
    // has 424px for a toggle plus four knobs, so the wider cells fit.
    // Compact tail: the FX slot cards need their height back for the two-row
    // knob grid on wide Airwindows algorithms. Smaller faces plus a tighter
    // ring margin keep every tail control and its value line intact.
    tailSpec.containerW = scaled(46);
    tailSpec.arcSize = scaled(30);
    tailSpec.ringPad = scaled(14);
    // 30 + face + 20 covers title line + value line: the compact 12px value
    // allowance clipped the Amount/frequency readouts.
    tailSpec.containerH = scaled(30) + tailSpec.arcSize + scaled(20);
    UIWidgets::ArcVisualSpec adsrSpec = tailSpec;
    // Six controls share the 250px column: four ADSR knobs + two compact
    // curve powers. 4*46 + 2*26 + five 2px gaps = 246px, so the row fits.
    adsrSpec.containerW = scaled(46);
    adsrSpec.arcSize = scaled(30);
    adsrSpec.containerH = scaled(30) + adsrSpec.arcSize + scaled(20);
    UIWidgets::ArcVisualSpec adsrCurveSpec = adsrSpec;
    adsrCurveSpec.containerW = scaled(26);
    adsrCurveSpec.arcSize = scaled(18);
    adsrCurveSpec.containerH = scaled(30) + adsrCurveSpec.arcSize + scaled(20);
    auto adsrArc = [&](lv_obj_t* row, uint32_t param, const char* name) {
        UIWidgets::createArcEx(row, (int)param, name, ui, styles, widgets, labels,
                               fParams, valueCb, adsrSpec, true);
    };
    auto adsrCurveArc = [&](lv_obj_t* row, uint32_t param, const char* name) {
        UIWidgets::createArcEx(row, (int)param, name, ui, styles, widgets, labels,
                               fParams, valueCb, adsrCurveSpec, true);
    };

    // --- Two ADSR columns: the tail envelope and the master output envelope.
    //     Each is 250px; the clip and bus columns still fit.
    auto buildAdsrColumn = [&](const char* title, uint32_t onParam,
                               uint32_t a, uint32_t d, uint32_t su, uint32_t r,
                               uint32_t dc, uint32_t rc) {
        lv_obj_t* col = lv_obj_create(body);
        lv_obj_set_size(col, scaled(250), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(col, 0, 0);
        lv_obj_set_style_border_width(col, 0, 0);
        lv_obj_set_style_pad_all(col, 0, 0);
        lv_obj_set_style_pad_row(col, scaled(2), 0);
        lv_obj_set_layout(col, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        uiLockScroll(col);
        lv_obj_t* head = lv_obj_create(col);
        lv_obj_set_size(head, lv_pct(100), scaled(18));
        lv_obj_set_style_bg_opa(head, 0, 0);
        lv_obj_set_style_border_width(head, 0, 0);
        lv_obj_set_style_pad_all(head, 0, 0);
        lv_obj_set_layout(head, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(head, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(head, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        uiLockScroll(head);
        lv_obj_t* cap = lv_label_create(head);
        lv_label_set_text(cap, title);
        lv_obj_add_style(cap, &styles.title, 0);
        lv_obj_set_style_text_font(cap, uiMicroFont(), 0);
        lv_obj_set_height(cap, scaled(18));
        lv_obj_t* toggleRow = lv_obj_create(col);
        lv_obj_set_size(toggleRow, lv_pct(100), scaled(26));
        lv_obj_set_style_bg_opa(toggleRow, 0, 0);
        lv_obj_set_style_border_width(toggleRow, 0, 0);
        lv_obj_set_style_pad_all(toggleRow, 0, 0);
        lv_obj_set_layout(toggleRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(toggleRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(toggleRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        uiLockScroll(toggleRow);
        buildToggle(toggleRow, onParam, "ON", "OFF");
        lv_obj_t* row = lv_obj_create(col);
        lv_obj_set_size(row, lv_pct(100), scaled(82));
        lv_obj_set_style_bg_opa(row, 0, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_style_pad_column(row, scaled(6), 0);
        lv_obj_set_layout(row, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        uiLockScroll(row);
        adsrArc(row, a, "Attack");
        adsrArc(row, d, "Decay");
        adsrArc(row, su, "Sustain");
        adsrArc(row, r, "Release");
        adsrCurveArc(row, dc, "DCR");
        adsrCurveArc(row, rc, "RCR");
        lv_obj_t* preview = AdsrCurvePreview::create(
            head, ui, a, d, su, r, dc, rc, scaled(80), scaled(14));
        for (uint32_t p : { a, d, su, r, dc, rc })
            AdsrCurvePreview::attach(widgets[p], preview);
        lv_obj_set_style_pad_column(row, scaled(2), 0);
    };
    buildAdsrColumn("FINAL ADSR", P::kParamFinalAROn,
                    P::kParamFinalARAttack, P::kParamFinalARDecay,
                    P::kParamFinalARSustain, P::kParamFinalARRelease,
                    P::kParamFinalARDecayCurve, P::kParamFinalARReleaseCurve);
    buildAdsrColumn("MASTER ADSR", P::kParamMasterAROn,
                    P::kParamMasterARAttack, P::kParamMasterARDecay,
                    P::kParamMasterARSustain, P::kParamMasterARRelease,
                    P::kParamMasterARDecayCurve, P::kParamMasterARReleaseCurve);

    // --- Clip column: clipper on/off + type + amount ---
    // TYPE is a real control (it selects the DSP's soft/hard clip branch,
    // PluginCymbals.cpp:2100) and AMOUNT is its drive/threshold. The toggle
    // sits above the type/amount pair so both stage groups' second rows start
    // at the same height: the tail's three groups read as one module.
    lv_obj_t* clipCol = lv_obj_create(body);
    lv_obj_set_size(clipCol, scaled(330), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(clipCol, 0, 0);
    lv_obj_set_style_border_width(clipCol, 0, 0);
    lv_obj_set_style_pad_all(clipCol, 0, 0);
    lv_obj_set_style_pad_row(clipCol, scaled(2), 0);
    lv_obj_set_layout(clipCol, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(clipCol, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(clipCol, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    uiLockScroll(clipCol);

    lv_obj_t* clipCap = lv_label_create(clipCol);
    lv_label_set_text(clipCap, "CLIP");
    lv_obj_add_style(clipCap, &styles.title, 0);
    lv_obj_set_style_text_font(clipCap, uiMicroFont(), 0);
    lv_obj_set_height(clipCap, scaled(18));

    lv_obj_t* clipToggleRow = lv_obj_create(clipCol);
    lv_obj_set_size(clipToggleRow, lv_pct(100), scaled(26));
    lv_obj_set_style_bg_opa(clipToggleRow, 0, 0);
    lv_obj_set_style_border_width(clipToggleRow, 0, 0);
    lv_obj_set_style_pad_all(clipToggleRow, 0, 0);
    lv_obj_set_style_pad_column(clipToggleRow, scaled(8), 0);
    lv_obj_set_layout(clipToggleRow, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(clipToggleRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(clipToggleRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    uiLockScroll(clipToggleRow);
    buildToggle(clipToggleRow, P::kParamFinalClipOn, "CLIP ON", "CLIP OFF");

    lv_obj_t* clipCtrlRow = lv_obj_create(clipCol);
    lv_obj_set_size(clipCtrlRow, lv_pct(100), scaled(82));
    lv_obj_set_style_bg_opa(clipCtrlRow, 0, 0);
    lv_obj_set_style_border_width(clipCtrlRow, 0, 0);
    lv_obj_set_style_pad_all(clipCtrlRow, 0, 0);
    lv_obj_set_style_pad_column(clipCtrlRow, 0, 0);
    lv_obj_set_layout(clipCtrlRow, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(clipCtrlRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(clipCtrlRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    uiLockScroll(clipCtrlRow);
    lv_obj_t* clipTypeDd = UIWidgets::createDropdown(clipCtrlRow, (int)P::kParamFinalClipType,
                                                     "Clip Type", "Soft\nHard", ui, styles,
                                                     widgets, fParams, valueCb);
    lv_obj_set_width(clipTypeDd, scaled(144));
    if (lv_obj_t* ddCont = lv_obj_get_parent(clipTypeDd))
        lv_obj_set_width(ddCont, scaled(144));
    uiEnableDropdownListScroll(clipTypeDd);
    uiFixDropdownPopup(clipTypeDd);
    UIWidgets::ArcVisualSpec clipSpec = tailSpec;
    clipSpec.containerW = scaled(56);
    clipSpec.arcSize = scaled(30);
    clipSpec.ringPad = scaled(14);
    clipSpec.containerH = scaled(30) + clipSpec.arcSize + scaled(20);
    auto clipArc = [&](lv_obj_t* row, uint32_t param, const char* name) {
        UIWidgets::createArcEx(row, (int)param, name, ui, styles, widgets, labels,
                               fParams, valueCb, clipSpec, true);
    };
    clipArc(clipCtrlRow, P::kParamFinalClipAmount, "Amt");
    clipArc(clipCtrlRow, P::kParamFinalClipMix, "Mix");
    clipArc(clipCtrlRow, P::kParamFinalClipOutput, "Out");

    // --- Quick final-touches tone shaping: one-pole HP then LP after the
    //     chain, before the master envelope. Two knobs, no modal.
    lv_obj_t* toneCol = lv_obj_create(body);
    lv_obj_set_size(toneCol, scaled(110), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(toneCol, 0, 0);
    lv_obj_set_style_border_width(toneCol, 0, 0);
    lv_obj_set_style_pad_all(toneCol, 0, 0);
    lv_obj_set_style_pad_row(toneCol, scaled(2), 0);
    lv_obj_set_layout(toneCol, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(toneCol, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(toneCol, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    uiLockScroll(toneCol);
    lv_obj_t* toneCap = lv_label_create(toneCol);
    lv_label_set_text(toneCap, "HP / LP 24dB");
    lv_obj_add_style(toneCap, &styles.title, 0);
    lv_obj_set_style_text_font(toneCap, uiMicroFont(), 0);
    lv_obj_set_height(toneCap, scaled(18));
    lv_obj_t* toneSpacer = lv_obj_create(toneCol);
    lv_obj_set_size(toneSpacer, lv_pct(100), scaled(26));
    lv_obj_set_style_bg_opa(toneSpacer, 0, 0);
    lv_obj_set_style_border_width(toneSpacer, 0, 0);
    uiLockScroll(toneSpacer);
    lv_obj_t* toneRow = lv_obj_create(toneCol);
    lv_obj_set_size(toneRow, lv_pct(100), scaled(82));
    lv_obj_set_style_bg_opa(toneRow, 0, 0);
    lv_obj_set_style_border_width(toneRow, 0, 0);
    lv_obj_set_style_pad_all(toneRow, 0, 0);
    lv_obj_set_style_pad_column(toneRow, scaled(6), 0);
    lv_obj_set_layout(toneRow, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(toneRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(toneRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    uiLockScroll(toneRow);
    UIWidgets::ArcVisualSpec toneSpec = tailSpec;
    toneSpec.containerW = scaled(50);
    auto toneArc = [&](lv_obj_t* row, uint32_t param, const char* name) {
        UIWidgets::createArcEx(row, (int)param, name, ui, styles, widgets, labels,
                               fParams, valueCb, toneSpec, true);
    };
    toneArc(toneRow, P::kParamFinalTouchHPF, "HPF");
    toneArc(toneRow, P::kParamFinalTouchLPF, "LPF");
    // --- POST-FILTER BUS column: the module the chain is inserted before.
    //     The plugin's post-filter bank runs in the SAME per-sample block as the
    //     serial chain (PluginCymbals.cpp:2013 vs :2045) and the chain sits in
    //     front of it, so this is the chain's real destination. Its on/off and
    //     series/parallel routing are the two controls that decide how the bank
    //     processes; the rest of the bank lives on the COLOUR tab.
    lv_obj_t* busCol = lv_obj_create(body);
    lv_obj_set_size(busCol, scaled(250), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(busCol, 0, 0);
    lv_obj_set_style_border_width(busCol, 0, 0);
    lv_obj_set_style_pad_all(busCol, 0, 0);
    lv_obj_set_style_pad_row(busCol, scaled(2), 0);
    lv_obj_set_layout(busCol, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(busCol, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(busCol, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    uiLockScroll(busCol);

    lv_obj_t* busCap = lv_label_create(busCol);
    lv_label_set_text(busCap, "POST-FILTER BUS");
    lv_obj_set_style_text_font(busCap, uiMicroFont(), 0);
    lv_obj_add_style(busCap, &styles.title, 0);
    lv_obj_set_height(busCap, scaled(18));

    lv_obj_t* busToggleRow = lv_obj_create(busCol);
    lv_obj_set_size(busToggleRow, lv_pct(100), scaled(26));
    lv_obj_set_style_bg_opa(busToggleRow, 0, 0);
    lv_obj_set_style_border_width(busToggleRow, 0, 0);
    lv_obj_set_style_pad_all(busToggleRow, 0, 0);
    lv_obj_set_style_pad_column(busToggleRow, scaled(8), 0);
    lv_obj_set_layout(busToggleRow, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(busToggleRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(busToggleRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    uiLockScroll(busToggleRow);
    buildToggle(busToggleRow, P::kParamFinalCombBankOn, "BUS", "BUS");
    buildToggle(busToggleRow, P::kParamFinalCombBankSeries, "SERIES", "PARALLEL");

    // Live state line for the bus column: the same two facts the toggles read
    // plus the group gain, which has no control on this page. It hangs off the
    // COLUMN, so the toggle row stays one line of controls.
    lv_obj_t* postLbl = lv_label_create(busCol);
    lv_label_set_text(postLbl, "");
    lv_obj_set_width(postLbl, scaled(236));
    lv_label_set_long_mode(postLbl, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_font(postLbl, getScaledMicroFont(), 0);
    lv_obj_set_style_text_color(postLbl, COL_TEXT, 0);
    lv_obj_set_style_text_align(postLbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_user_data(postLbl, (void*)payload(kPostBusPayload));
    if (outChainLabels != nullptr) outChainLabels[kPostBus] = postLbl;

    return pane;
}

inline void FXChainTab::collectChainLabels(lv_obj_t* root, lv_obj_t** outChainLabels) {
    using namespace FxChainTabDetail;
    if (outChainLabels == nullptr) return;
    for (int i = 0; i < kLabelSlotCount; ++i) outChainLabels[i] = nullptr;
    if (root == nullptr) return;
    // Depth 2 reaches the bus container's children and the tail card's body row.
    for (int pos = 0; pos < 6; ++pos) {
        outChainLabels[kChainChip0 + pos] =
            findByPayload(root, payload(kChainChipPayload, pos), 2);
    }
    outChainLabels[kChainCount] = findByPayload(root, payload(kChainCountPayload), 2);
    outChainLabels[kTailState]  = findByPayload(root, payload(kTailStatePayload), 2);
    outChainLabels[kPostBus]    = findByPayload(root, payload(kPostBusPayload), 2);
}

inline void FXChainTab::build(lv_obj_t* tab, AbstractCymbalsUI* ui, UIStyles& styles,
                              lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                              lv_event_cb_t valueCb, lv_obj_t** slotPanels,
                              lv_obj_t** outChainLabels,
                              lv_event_cb_t randomizeAllCb,
                              lv_event_cb_t randomizeSlotCb,
                              lv_event_cb_t randomizeSettingsAllCb,
                              lv_event_cb_t randomizeSettingsSlotCb) {

    using namespace FxChainTabDetail;

    if (outChainLabels != nullptr) {
        for (int i = 0; i < kLabelSlotCount; ++i) outChainLabels[i] = nullptr;
    }

    // Make the tab content non-scrollable, column layout. Six full slot panels
    // plus the tail card cannot stack in the 574px content area, so slots are
    // arranged in two rows of three (2x3 grid); each row is a flex row whose
    // panels share the width equally. The chain bus (one 28px line) and the TAIL
    // card fill the space the slot rows leave, so the page is dense top to
    // bottom instead of ending in a void.
    lv_obj_set_layout(tab, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(tab, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(tab, scaled(8), 0);
    // Tight row gap: the turn marker between the two slot rows is the only
    // separator needed, and every pixel saved here goes back to the cards.
    lv_obj_set_style_pad_row(tab, scaled(3), 0);
    lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLLABLE);

    // Page header: the chain is serial and position-numbered.
    lv_obj_t* strip = UIWidgets::createScreenStrip(tab, styles, "FX CHAIN - SERIAL");
    if (randomizeAllCb != nullptr) {
        auto headerDice = [&](lv_event_cb_t cb, bool shuffle, const char* tip) {
            lv_obj_t* btn = lv_btn_create(strip);
            lv_obj_set_size(btn, scaled(28), scaled(28));
            styles.applyMomentaryButton(btn);
            lv_obj_clear_state(btn, LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(btn, COL_CONTROL_FACE, 0);
            if (shuffle) {
                lv_obj_t* l = lv_label_create(btn);
                lv_label_set_text(l, LV_SYMBOL_SHUFFLE);
                lv_obj_set_style_text_font(l, &lv_font_montserrat_12, 0);
                lv_obj_center(l);
            } else {
                lv_obj_t* die = uiDiceIcon(btn, COL_TEXT);
                if (die != nullptr) lv_obj_center(die);
            }
            lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, ui);
            uiAttachTooltip(btn, tip);
        };
        headerDice(randomizeAllCb, false, "Randomize chain count, sources and settings");
        if (randomizeSettingsAllCb != nullptr)
            headerDice(randomizeSettingsAllCb, true, "Randomize settings of the loaded FX only");
    }

    // For slot-by-slot drag callbacks, we need a "roller" callback too.
    lv_event_cb_t dragCb = ui->getDragCallback();

    // Build the 6 slot panels in two rows of three. No separator row between
    // the rows: the 01..06 position chips and the IN > > > > > > OUT bus line
    // already carry the serial turn, and a dedicated arrow row cost ~20px of
    // card height that two-row algorithms need for their value lines.
    for (int row = 0; row < 2; ++row) {
        lv_obj_t* rowCont = lv_obj_create(tab);
        // The rows GROW into whatever the tail card does not need: the slot
        // grid is the page's main content, so the leftover height belongs
        // here. Content-sized rows left a bare band under the tail card
        // (round-4 critic: "an unbroken black void down to the keyboard").
        lv_obj_set_size(rowCont, lv_pct(100), 0);
        lv_obj_set_flex_grow(rowCont, 1);
        lv_obj_set_style_bg_opa(rowCont, 0, 0);
        lv_obj_set_style_border_width(rowCont, 0, 0);
        lv_obj_set_style_pad_all(rowCont, 0, 0);
        lv_obj_set_style_pad_column(rowCont, scaled(4), 0);
        lv_obj_set_layout(rowCont, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(rowCont, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(rowCont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        uiLockScroll(rowCont);
        lv_obj_set_user_data(rowCont, (void*)(intptr_t)(100 + row + 1));  // row sentinels

        for (int col = 0; col < 3; ++col) {
            const int slot = row * 3 + col;
            lv_obj_t* panel = UIModularSlotSection::build(
                rowCont, slot, ui, styles, widgets, labels, fParams, valueCb, dragCb);
            // Equal thirds within the row: flex-grow from a zero width base.
            lv_obj_set_width(panel, 0);
            lv_obj_set_flex_grow(panel, 1);
            // Full row height (LVGL 9 has no stretch align): a grown row makes
            // TALL empty slots - the bar's own FX page - instead of a tall row
            // with short cards floating in it.
            lv_obj_set_height(panel, lv_pct(100));
            // Chain position caption: each card says WHERE it sits in the
            // serial chain ("01 FX A", "02 FX B", ...). Six empty slots with
            // the same "EMPTY" pill were "a wall of identical voids" instead
            // of a signal-path view (round-5 critic): with the position and
            // the insert's own category on the card, the row reads as the
            // chain it is, and an empty slot says which stage of the chain
            // is missing.
            {
                int order[6] = {0, 1, 2, 3, 4, 5};
                slotOrderFromParams(fParams, order);
                int position = 0;
                for (int p = 0; p < 6; ++p)
                    if (order[p] == slot) { position = p; break; }
                const int typeP = (int)PluginCymbals::kParamSlot1Type + slot * 16;
                const int typeVal = (int)std::lround(fParams[typeP]);
                char cap[40];
                if (typeVal > 0 && !fxTypeIsNone(typeVal)) {
                    const char* nm = fxSurfaceName(typeVal);
                    if (nm == nullptr || nm[0] == '\0') nm = "FX";
                    snprintf(cap, sizeof(cap), "%02d  %s", position + 1, nm);
                } else {
                    // Empty: the card already says EMPTY and offers ADD, so
                    // the caption carries only WHICH stage of the chain is
                    // missing.
                    snprintf(cap, sizeof(cap), "SLOT %02d", position + 1);
                }
                // FLOATING so it does not take a row of the card's flex
                // column, and aligned to the card's TOP RIGHT: the card
                // already carries its own slot-number badge in the top-left
                // corner, and the caption is the CHAIN position, which is a
                // different thing (the badge is the card, the caption is
                // where the card sits in the serial chain).
                // Populated cards now carry two knob rows whose VALUE line
                // sits on the card's bottom edge; the floating chain caption
                // collided with it ("04 None" over a readout). The header
                // already names the algorithm and the slot chip carries the
                // card number, so the caption is only needed on empty cards.
                lv_obj_t* capLbl = (typeVal > 0) ? nullptr : lv_label_create(panel);
                if (capLbl == nullptr) {
                    // nothing to draw for populated cards
                } else {
                lv_label_set_text(capLbl, cap);
                lv_obj_set_style_text_font(capLbl, uiMicroFont(), 0);
                lv_obj_set_style_text_color(capLbl, COL_TEXT_DIM, 0);
                lv_obj_set_style_text_letter_space(capLbl, scaled(1), 0);
                lv_obj_set_style_pad_all(capLbl, 0, 0);
                lv_obj_add_flag(capLbl, LV_OBJ_FLAG_FLOATING);
                // BOTTOM CENTRE: the card's top-left holds its own slot-number
                // badge and its top-right holds the close and flow buttons, so
                // the only free space is under the centred EMPTY/ADD content.
                lv_obj_align(capLbl, LV_ALIGN_BOTTOM_MID, 0, -scaled(4));
                lv_obj_clear_flag(capLbl, LV_OBJ_FLAG_CLICKABLE);
                lv_obj_clear_flag(capLbl, LV_OBJ_FLAG_SCROLLABLE);
                lv_obj_move_foreground(capLbl);
                }
            }
            if (randomizeSlotCb != nullptr) {
                lv_obj_t* headerRow = lv_obj_get_child(panel, 1);
                if (headerRow != nullptr) {
                    lv_obj_t* dice = lv_btn_create(headerRow);
                    lv_obj_set_size(dice, scaled(26), scaled(18));
                    styles.applyMomentaryButton(dice);
                    lv_obj_clear_state(dice, LV_STATE_CHECKED);
                    lv_obj_set_style_bg_color(dice, COL_CONTROL_FACE, 0);
                    lv_obj_t* die = uiDiceIcon(dice, COL_TEXT);
                    if (die != nullptr) lv_obj_center(die);
                    lv_obj_set_user_data(dice,
                        (void*)payload(kFxRandomPayload, slot));
                    lv_obj_add_event_cb(dice, randomizeSlotCb, LV_EVENT_CLICKED, ui);
                    uiAttachTooltip(dice, "Randomize this FX slot (source + settings)");
                    lv_obj_move_to_index(dice, 5);
                    if (randomizeSettingsSlotCb != nullptr) {
                        lv_obj_t* shuffle = lv_btn_create(headerRow);
                        lv_obj_set_size(shuffle, scaled(26), scaled(18));
                        styles.applyMomentaryButton(shuffle);
                        lv_obj_clear_state(shuffle, LV_STATE_CHECKED);
                        lv_obj_set_style_bg_color(shuffle, COL_CONTROL_FACE, 0);
                        lv_obj_t* sl = lv_label_create(shuffle);
                        lv_label_set_text(sl, LV_SYMBOL_SHUFFLE);
                        lv_obj_set_style_text_font(sl, &lv_font_montserrat_12, 0);
                        lv_obj_center(sl);
                        lv_obj_set_user_data(shuffle,
                            (void*)payload(kFxRandomPayload, slot));
                        lv_obj_add_event_cb(shuffle, randomizeSettingsSlotCb, LV_EVENT_CLICKED, ui);
                        uiAttachTooltip(shuffle, "Randomize this slot's settings only");
                        lv_obj_move_to_index(shuffle, 6);
                    }
                }
            }
            if (slotPanels) slotPanels[slot] = panel;
        } // for col
    }

    // One line above the tail card: the six inserts in processing order with
    // their real insert names, so the chain's state is readable without opening
    // every card.
    buildChainBus(tab, outChainLabels);

    // The terminal module: ADSR + Clip + the POST-FILTER BUS it feeds.
    buildTailCard(tab, ui, styles, widgets, labels, fParams, valueCb, outChainLabels);

    // Build-time paint of the readouts (the owner re-paints on any chain
    // parameter change).
    refreshChainReadouts(fParams, outChainLabels);
}

// ---------------------------------------------------------------------------
// Live readouts. Every value comes from the parameter array:
//   * slot insert names / wet / bypass -> kParamSlot1Type + slot*16 (+14/+15)
//   * processing order                 -> kParamSlotOrder0..5
//   * tail + post-filter state         -> kParamFinal* / kParamFinalCombBank*
// Nothing is invented and nothing is decorative.
// ---------------------------------------------------------------------------
inline void FXChainTab::refreshChainReadouts(float* fParams, const lv_obj_t* const* chainLabels) {
    using namespace FxChainTabDetail;
    using P = PluginCymbals::Parameters;

    if (fParams == nullptr || chainLabels == nullptr) return;

    auto setText = [](lv_obj_t* lbl, const char* text) {
        if (lbl == nullptr) return;
        setLabelText(lbl, text);
    };

    int order[6] = {0, 1, 2, 3, 4, 5};
    slotOrderFromParams(fParams, order);

    int activeCount = 0;
    int dryCount = 0;
    int lastLoaded = -1;

    for (int pos = 0; pos < 6; ++pos) {
        const int slot = order[pos];
        const int typeIdx = (int)P::kParamSlot1Type + slot * 16;
        const int typeId = (int)std::lround(fParams[typeIdx]);
        const bool typeIsNone = fxTypeIsNone(typeId);
        const bool bypassed = fParams[typeIdx + 15] >= 0.5f;
        const float wet = std::clamp(fParams[typeIdx + 14], 0.0f, 1.0f);

        lv_obj_t* chip = const_cast<lv_obj_t*>(chainLabels[kChainChip0 + pos]);
        if (chip != nullptr) {
            char buf[48];
            lv_color_t want = COL_TEXT_DIM;
            if (typeIsNone) {
                snprintf(buf, sizeof(buf), "%02d EMPTY", slot + 1);
            } else {
                // Wet is the slot's own dry/wet mix — the ONLY mix this insert
                // has on the DSP path (PluginCymbals.cpp:2048). A bypassed slot
                // is loaded but out of the path, so it is marked rather than
                // silently counted as a running insert.
                const char* nm = fxSurfaceName(typeId);
                const char* name = (nm != nullptr && nm[0]) ? nm : "?";
                if (bypassed) {
                    snprintf(buf, sizeof(buf), "%02d %s\nBYP", slot + 1, name);
                } else {
                    snprintf(buf, sizeof(buf), "%02d %s\nWET %3.0f%%", slot + 1, name, wet * 100.0f);
                    want = COL_HIGHLIGHT;
                }
            }
            // Text/colour writes only on change; the wrap/height are invariant
            // and set once in buildChainBus. LVGL style writes invalidate and
            // mark the parent layout-dirty even when the value is identical.
            const lv_color_t cur = lv_obj_get_style_text_color(chip, LV_PART_MAIN);
            if (cur.red != want.red || cur.green != want.green || cur.blue != want.blue)
                lv_obj_set_style_text_color(chip, want, 0);
            setText(chip, buf);
        }

        if (typeIsNone) continue;
        lastLoaded = pos;
        if (bypassed) continue;
        ++activeCount;
        if (wet < 0.02f) ++dryCount;
    }

    // How many inserts actually run. A bypassed slot is a loaded insert that is
    // not in the path, so it must not be counted as active; a wet=0 insert is
    // loaded and unmuted but contributes no effect.
    lv_obj_t* chainLbl = const_cast<lv_obj_t*>(chainLabels[kChainCount]);
    if (chainLbl != nullptr) {
        char buf[72];
        if (activeCount == 0) {
            snprintf(buf, sizeof(buf), "%d ACTIVE - CHAIN IDLE", activeCount);
            lv_obj_set_style_text_color(chainLbl, COL_TEXT_DIM, 0);
        } else {
            snprintf(buf, sizeof(buf), "%d ACTIVE - %d DRY WET", activeCount, dryCount);
            lv_obj_set_style_text_color(chainLbl, COL_TEXT, 0);
        }
        setText(chainLbl, buf);
    }

    // The tail chip names the last insert in the chain that the tail is fed by,
    // or the bus itself when the chain is empty (the tail and the post-filter
    // bank still run on the dry signal).
    lv_obj_t* tailLbl = const_cast<lv_obj_t*>(chainLabels[kTailState]);
    if (tailLbl != nullptr) {
        const bool tailOn = fParams[P::kParamFinalAROn] > 0.5f;
        const bool clipOn = fParams[P::kParamFinalClipOn] > 0.5f;
        const bool busOn  = fParams[P::kParamFinalCombBankOn] > 0.5f;
        const bool lit = (tailOn || clipOn || busOn);
        char buf[72];
        if (lastLoaded < 0) {
            snprintf(buf, sizeof(buf), "FINAL %s - %s", tailOn ? "ON" : "OFF",
                     busOn ? "BUS ON" : "BUS OFF");
        } else {
            const int slot = order[lastLoaded];
            const int typeIdx = (int)P::kParamSlot1Type + slot * 16;
            const char* nm = fxSurfaceName((int)std::lround(fParams[typeIdx]));
            snprintf(buf, sizeof(buf), "FINAL %s <- S%d %s", tailOn ? "ON" : "OFF",
                     slot + 1, (nm != nullptr && nm[0]) ? nm : "?");
        }
        setText(tailLbl, buf);
        lv_obj_set_style_text_color(tailLbl, lit ? COL_HIGHLIGHT : COL_TEXT_DIM, 0);
        lv_obj_set_style_border_opa(tailLbl, lit ? LV_OPA_70 : LV_OPA_40, 0);
    }

    // Post-filter bus state: series/parallel routing plus the group gain the
    // bank applies, both real reads.
    lv_obj_t* postLbl = const_cast<lv_obj_t*>(chainLabels[kPostBus]);
    if (postLbl != nullptr) {
        const bool busOn = fParams[P::kParamFinalCombBankOn] > 0.5f;
        const bool series = fParams[P::kParamFinalCombBankSeries] > 0.5f;
        const float gain = std::clamp(fParams[P::kParamFinalCombGroupGain], 0.0f, 1.0f);
        char buf[80];
        snprintf(buf, sizeof(buf), "%s - %s - GAIN %.2f", busOn ? "BUS ON" : "BUS OFF",
                 series ? "SERIES" : "PARALLEL", gain);
        setText(postLbl, buf);
    }
}

END_NAMESPACE_DISTRHO
