#ifndef ADSR_CURVE_PREVIEW_HPP
#define ADSR_CURVE_PREVIEW_HPP

#include "UICommon.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

START_NAMESPACE_DISTRHO

// Tiny ADSR shape preview for the four main envelopes (stick amp/pitch, final,
// master). The plot follows AdsrEnvelope's own curve law:
//   level = start + (target - start) * p^(2^-curve)
// so turning a DECAY/RELEASE curve knob visibly bends the segment while the
// A/D/S/R values set its width. One shared painter keeps the FILTER-style
// pages from each inventing a different glyph.
struct AdsrCurvePreview {
    struct State {
        AbstractCymbalsUI* ui = nullptr;
        uint32_t attackParam = 0;
        uint32_t decayParam = 0;
        uint32_t sustainParam = 0;
        uint32_t releaseParam = 0;
        uint32_t decayCurveParam = 0;
        uint32_t releaseCurveParam = 0;
    };

    static void deleteStateCb(lv_event_t* e) {
        delete static_cast<State*>(lv_event_get_user_data(e));
    }

    static void invalidateCb(lv_event_t* e) {
        lv_obj_t* preview = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
        // lv_obj_invalidate() ignores HIDDEN; guard explicitly so an automated
        // ADSR knob on a hidden IMPULSES/FX page does not dirty the visible
        // page underneath.
        if (preview && lv_obj_is_visible(preview)) lv_obj_invalidate(preview);
    }

    // Attach the invalidation callback to every arc that feeds the preview
    // (A/D/S/R + the two curve powers). Sync/preset writes send
    // VALUE_CHANGED from syncParamWidget, so host automation repaints too.
    static void attach(lv_obj_t* arc, lv_obj_t* preview) {
        if (arc && preview)
            lv_obj_add_event_cb(arc, invalidateCb, LV_EVENT_VALUE_CHANGED, preview);
    }

    static lv_obj_t* create(lv_obj_t* parent, AbstractCymbalsUI* ui,
                            uint32_t attackParam, uint32_t decayParam,
                            uint32_t sustainParam, uint32_t releaseParam,
                            uint32_t decayCurveParam, uint32_t releaseCurveParam,
                            lv_coord_t width, lv_coord_t height) {
        if (parent == nullptr || ui == nullptr) return nullptr;
        lv_obj_t* obj = lv_obj_create(parent);
        lv_obj_set_size(obj, width, height);
        lv_obj_set_style_bg_color(obj, COL_PANEL_DARK, 0);
        lv_obj_set_style_bg_opa(obj, LV_OPA_80, 0);
        lv_obj_set_style_border_color(obj, COL_ACCENT, 0);
        lv_obj_set_style_border_width(obj, 1, 0);
        lv_obj_set_style_radius(obj, scaled(3), 0);
        lv_obj_set_style_pad_all(obj, 0, 0);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        auto* state = new State();
        state->ui = ui;
        state->attackParam = attackParam;
        state->decayParam = decayParam;
        state->sustainParam = sustainParam;
        state->releaseParam = releaseParam;
        state->decayCurveParam = decayCurveParam;
        state->releaseCurveParam = releaseCurveParam;
        lv_obj_set_user_data(obj, state);
        lv_obj_add_event_cb(obj, deleteStateCb, LV_EVENT_DELETE, state);
        lv_obj_add_event_cb(obj, drawCb, LV_EVENT_DRAW_MAIN, nullptr);
        return obj;
    }

    static void drawCb(lv_event_t* e) {
        lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
        auto* st = static_cast<State*>(lv_obj_get_user_data(obj));
        if (!obj || !st || !st->ui) return;
        lv_layer_t* layer = lv_event_get_layer(e);
        lv_area_t coords;
        lv_obj_get_coords(obj, &coords);
        const int w = lv_obj_get_width(obj);
        const int h = lv_obj_get_height(obj);
        if (w < 14 || h < 10) return;

        const auto v = [&](uint32_t p) {
            return std::clamp(st->ui->getParamValue(p), 0.0f, 1.0f);
        };
        const float a = v(st->attackParam);
        const float d = v(st->decayParam);
        const float s = v(st->sustainParam);
        const float r = v(st->releaseParam);
        const float dc = v(st->decayCurveParam);
        const float rc = v(st->releaseCurveParam);
        const float eD = std::pow(2.0f, -(dc * 2.0f - 1.0f));
        const float eR = std::pow(2.0f, -(rc * 2.0f - 1.0f));

        // Relative stage widths: the controls' times are perceptually mapped,
        // so the normalized values are enough to make the plot move with them.
        float wa = 0.12f + a * 0.20f;
        float wd = 0.18f + d * 0.22f;
        float ws = 0.16f + s * 0.10f;
        float wr = 0.18f + r * 0.22f;
        const float total = wa + wd + ws + wr;
        wa /= total; wd /= total; ws /= total; wr /= total;

        const int pad = std::max(2, (int)scaled(3));
        const int plotW = std::max(1, w - 2 * pad);
        const int plotH = std::max(1, h - 2 * pad);
        const int x0 = coords.x1 + pad;
        const int y0 = coords.y1 + pad;
        const int yBase = y0 + plotH;

        lv_draw_line_dsc_t grid;
        lv_draw_line_dsc_init(&grid);
        grid.color = COL_HAIRLINE;
        grid.opa = 90;
        grid.width = 1;
        grid.p1 = {(float)x0, (float)(yBase - plotH / 2)};
        grid.p2 = {(float)(x0 + plotW), (float)(yBase - plotH / 2)};
        lv_draw_line(layer, &grid);

        lv_draw_line_dsc_t trace;
        lv_draw_line_dsc_init(&trace);
        trace.color = COL_HIGHLIGHT;
        trace.opa = LV_OPA_COVER;
        trace.width = std::max(1, (int)scaled(1));
        trace.round_start = 1;
        trace.round_end = 1;

        auto levelAt = [&](float t) {
            // t = normalized stage time inside the whole envelope.
            if (t <= wa) {
                const float p = wa > 0.0f ? t / wa : 1.0f;
                return p; // attack: linear (attack curve is not exposed here)
            }
            if (t <= wa + wd) {
                const float p = wd > 0.0f ? (t - wa) / wd : 1.0f;
                return 1.0f + (s - 1.0f) * std::pow(p, eD);
            }
            if (t <= wa + wd + ws) return s;
            const float p = wr > 0.0f ? (t - wa - wd - ws) / wr : 1.0f;
            return s * (1.0f - std::pow(std::clamp(p, 0.0f, 1.0f), eR));
        };

        constexpr int kSteps = 16;
        lv_point_precise_t prev = {
            (float)x0,
            (float)(yBase - (int)(std::clamp(levelAt(0.0f), 0.0f, 1.0f) * (float)plotH))
        };
        for (int i = 1; i <= kSteps; ++i) {
            const float t = (float)i / (float)kSteps;
            const int px = x0 + (int)(t * (float)plotW);
            const int py = yBase - (int)(std::clamp(levelAt(t), 0.0f, 1.0f) * (float)plotH);
            lv_point_precise_t cur = {(float)px, (float)py};
            trace.p1 = prev;
            trace.p2 = cur;
            lv_draw_line(layer, &trace);
            prev = cur;
        }
    }
};

END_NAMESPACE_DISTRHO

#endif // ADSR_CURVE_PREVIEW_HPP
