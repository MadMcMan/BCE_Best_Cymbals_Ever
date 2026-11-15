#ifndef UI_MSEG_EDITOR_HPP
#define UI_MSEG_EDITOR_HPP

#include "lvgl.h"
#include "UICommon.hpp"
#include "UIStyles.hpp"
#include "../PluginCymbals.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

START_NAMESPACE_DISTRHO

struct UIMsegEditor {
    struct State {
        // State for multi-segment envelope editor
        // Ownership: heap-allocated with 'new', deleted in event_cb LV_EVENT_DELETE
        AbstractCymbalsUI* ui;
        int valueParams[8];   // Parameter indices for 8 points
        int timeParams[6];    // Parameter indices for time between points
        int curveParams[7];   // Parameter indices for curve shape
        int dragPoint = -1;   // Current drag target (-1 if none)
        int dragCurve = -1;   // Current drag curve (-1 if none)
        bool dragging = false;
        lv_point_t dragLast;
        lv_obj_t* obj = nullptr;   // editor object, for invalidation from the timer
        float playPhase = -1.0f;   // live MSEG playhead (0..1), -1 = hidden
        lv_timer_t* playTimer = nullptr;
    };

    // Polls the DSP's stick-pitch MSEG position and redraws the playhead while
    // a note is sounding. No-op (no invalidate) when the phase is unchanged.
    static void playheadTimerCb(lv_timer_t* t) {
        State* st = static_cast<State*>(t->user_data);
        if (!st || !st->ui || !st->obj) return;
        const float ph = st->ui->getStickMsegPhase();
        if (std::fabs(ph - st->playPhase) > 0.001f) {
            st->playPhase = ph;
            lv_obj_invalidate(st->obj);
        }
    }

    // OWNERSHIP: State* allocated with 'new', stored in LVGL user_data,
    // DELETED in event_cb LV_EVENT_DELETE handler when LVGL destroys obj
    static lv_obj_t* create(lv_obj_t* parent, AbstractCymbalsUI* ui, UIStyles& /*styles*/) {
        // Ownership: parent (LVGL)
        lv_obj_t* obj = lv_obj_create(parent);
        lv_obj_set_size(obj, 275, 120);
        lv_obj_set_style_bg_color(obj, COL_PANEL_DARK, 0);
        lv_obj_set_style_bg_opa(obj, 200, 0);
        lv_obj_set_style_border_color(obj, COL_ACCENT, 0);
        lv_obj_set_style_border_width(obj, 1, 0);
        lv_obj_set_style_radius(obj, 4, 0);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);

        // Ownership: user-allocated, deleted in event_cb when LV_EVENT_DELETE fires
        State* state = new State();
        state->ui = ui;
        for(int i=0; i<8; i++) state->valueParams[i] = PluginCymbals::kParamStickPitchMsegPoint0Value + i;
        for(int i=0; i<6; i++) state->timeParams[i] = PluginCymbals::kParamStickPitchMsegPoint1Time + i;
        for(int i=0; i<7; i++) state->curveParams[i] = PluginCymbals::kParamStickPitchMsegPoint0Curve + i;

        state->obj = obj;
        // 8ms: the default pitch sweep is only ~21ms, so a 33ms poll would
        // resolve at most one frame per note. Invalidate only on phase change.
        state->playTimer = lv_timer_create(playheadTimerCb, 8, state);

        lv_obj_set_user_data(obj, state);
        lv_obj_add_event_cb(obj, event_cb, LV_EVENT_ALL, state);

        return obj;
    }

private:
    static void event_cb(lv_event_t* e) {
        lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
        State* state = (State*)lv_event_get_user_data(e);
        lv_event_code_t code = lv_event_get_code(e);

        if (code == LV_EVENT_DRAW_POST) {
            draw_main(e);
        }
        else if (code == LV_EVENT_PRESSED) {
            lv_indev_t* indev = lv_indev_get_act();
            lv_point_t p;
            lv_indev_get_point(indev, &p);
            const lv_point_t lp = uiObjLocalPoint(obj, p);
            int lx = lp.x;
            int ly = lp.y;

            int w = lv_obj_get_width(obj);
            int h = lv_obj_get_height(obj);
            const int kPad = 8;
            const int plotW = std::max(1, w - 2 * kPad);
            const int plotH = std::max(1, h - 2 * kPad);
            auto plotX = [&](float t) { return kPad + (int)(t * plotW); };
            auto plotY = [&](float v) { return kPad + (int)((1.0f - v) * plotH); };

            state->dragPoint = -1;
            state->dragCurve = -1;

            // Check points first
            for (int i = 0; i < 8; i++) {
                float tx = (i == 0) ? 0.0f : (i == 7 ? 1.0f : state->ui->getParamValue(state->timeParams[i-1]));
                float ty = state->ui->getParamValue(state->valueParams[i]);
                int px = plotX(tx);
                int py = plotY(ty);

                if (std::abs(lx - px) < 12 && std::abs(ly - py) < 12) {
                    state->dragPoint = i;
                    state->dragging = true;
                    state->dragLast = lp;
                    // Gesture begin for every param this drag may touch
                    // (mirrors the XY-pad PRESSED bracketing).
                    state->ui->editParameter((uint32_t)state->valueParams[i], true);
                    if (i > 0 && i < 7)
                        state->ui->editParameter((uint32_t)state->timeParams[i - 1], true);
                    break;
                }
            }

            // If no point hit, check curves
            if (!state->dragging) {
                for (int i = 0; i < 7; i++) {
                    float t0 = (i == 0) ? 0.0f : state->ui->getParamValue(state->timeParams[i-1]);
                    float t1 = (i == 6) ? 1.0f : state->ui->getParamValue(state->timeParams[i]);
                    float v0 = state->ui->getParamValue(state->valueParams[i]);
                    float v1 = state->ui->getParamValue(state->valueParams[i+1]);
                    float curve = (state->ui->getParamValue(state->curveParams[i]) - 0.5f) * 2.0f;

                    float fx = (float)(lx - kPad) / plotW;
                    if (fx >= t0 - 0.02f && fx <= t1 + 0.02f) {
                        float relT = (fx - t0) / std::max(0.001f, t1 - t0);
                        relT = std::clamp(relT, 0.0f, 1.0f);
                        float p_val = 1.0f;
                        if (curve > 0) p_val = 1.0f + curve * 4.0f;
                        else p_val = 1.0f / (1.0f - curve * 4.0f);
                        float ct = std::pow(relT, p_val);
                        float cv = v0 + (v1 - v0) * ct;
                        int cpy = plotY(cv);

                        if (std::abs(ly - cpy) < 15) {
                            state->dragCurve = i;
                            state->dragging = true;
                            state->dragLast = lp;
                            state->ui->editParameter((uint32_t)state->curveParams[i], true);
                            break;
                        }
                    }
                }
            }
            lv_obj_invalidate(obj);
        }
        else if (code == LV_EVENT_PRESSING) {
            if (state->dragging) {
                lv_indev_t* indev = lv_indev_get_act();
                lv_point_t p;
                lv_indev_get_point(indev, &p);
                const lv_point_t lp = uiObjLocalPoint(obj, p);

                
                int w = lv_obj_get_width(obj);
                int h = lv_obj_get_height(obj);
                const int kPad = 8;
                const int plotW = std::max(1, w - 2 * kPad);
                const int plotH = std::max(1, h - 2 * kPad);

                if (state->dragPoint != -1) {
                    float nx = (float)(lp.x - kPad) / plotW;
                    float ny = 1.0f - (float)(lp.y - kPad) / plotH;
                    ny = std::clamp(ny, 0.0f, 1.0f);

                    state->ui->setParamValue(state->valueParams[state->dragPoint], ny);
                    state->ui->updateParamLabel(state->valueParams[state->dragPoint]);

                    if (state->dragPoint > 0 && state->dragPoint < 7) {
                        float prevT = (state->dragPoint == 1) ? 0.0f : state->ui->getParamValue(state->timeParams[state->dragPoint - 2]);
                        float nextT = (state->dragPoint == 6) ? 1.0f : state->ui->getParamValue(state->timeParams[state->dragPoint]);
                        nx = std::clamp(nx, prevT + 0.001f, nextT - 0.001f);
                        state->ui->setParamValue(state->timeParams[state->dragPoint - 1], nx);
                        state->ui->updateParamLabel(state->timeParams[state->dragPoint - 1]);
                    }
                }
                else if (state->dragCurve != -1) {
                    float dy = (float)(lp.y - state->dragLast.y) / (float)h;
                    float curP = state->ui->getParamValue(state->curveParams[state->dragCurve]);
                    // invert dy because screen up is negative, and we want up to increase "p" or "curvature"
                    // let's just make it simple: dragging down makes it more concave (lower p), up more convex (higher p)
                    float nextP = std::clamp(curP - dy, 0.0f, 1.0f);
                    state->ui->setParamValue(state->curveParams[state->dragCurve], nextP);
                    state->ui->updateParamLabel(state->curveParams[state->dragCurve]);
                    state->dragLast = lp;
                }
                lv_obj_invalidate(obj);
            }
        }
        else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
            // Close the gesture brackets opened at PRESSED before clearing
            // which point/curve was touched.
            if (state->dragging) {
                if (state->dragPoint != -1) {
                    state->ui->editParameter((uint32_t)state->valueParams[state->dragPoint], false);
                    if (state->dragPoint > 0 && state->dragPoint < 7)
                        state->ui->editParameter((uint32_t)state->timeParams[state->dragPoint - 1], false);
                } else if (state->dragCurve != -1) {
                    state->ui->editParameter((uint32_t)state->curveParams[state->dragCurve], false);
                }
            }
            state->dragging = false;
            state->dragPoint = -1;
            state->dragCurve = -1;
            lv_obj_invalidate(obj);
        }
        else if (code == LV_EVENT_DOUBLE_CLICKED) {
            lv_indev_t* indev = lv_indev_get_act();
            lv_point_t p;
            lv_indev_get_point(indev, &p);
            const lv_point_t lp = uiObjLocalPoint(obj, p);
            int lx = lp.x;
            int ly = lp.y;
            int w = lv_obj_get_width(obj);
            int h = lv_obj_get_height(obj);
            const int kPad = 8;
            const int plotW = std::max(1, w - 2 * kPad);
            const int plotH = std::max(1, h - 2 * kPad);

            float fx = (float)(lx - kPad) / (float)plotW;
            float fy = 1.0f - (float)(ly - kPad) / (float)plotH;
            fx = std::clamp(fx, 0.0f, 1.0f);
            fy = std::clamp(fy, 0.0f, 1.0f);

            // Find closest point based on time (X)
            int closest = -1;
            float minDist = 2.0f;
            for (int i = 0; i < 8; i++) {
                float tx = (i == 0) ? 0.0f : (i == 7 ? 1.0f : state->ui->getParamValue(state->timeParams[i-1]));
                float dist = std::abs(fx - tx);
                if (dist < minDist) {
                    minDist = dist;
                    closest = i;
                }
            }

            if (closest != -1) {
                // Set the value (Y)
                state->ui->editParameter((uint32_t)state->valueParams[closest], true);
                state->ui->setParamValue(state->valueParams[closest], fy);
                state->ui->editParameter((uint32_t)state->valueParams[closest], false);
                state->ui->updateParamLabel(state->valueParams[closest]);

                // Set the time (X) for inner points
                if (closest > 0 && closest < 7) {
                    float prevT = (closest == 1) ? 0.0f : state->ui->getParamValue(state->timeParams[closest - 2]);
                    float nextT = (closest == 6) ? 1.0f : state->ui->getParamValue(state->timeParams[closest]);
                    float nx = std::clamp(fx, prevT + 0.001f, nextT - 0.001f);
                    state->ui->editParameter((uint32_t)state->timeParams[closest - 1], true);
                    state->ui->setParamValue(state->timeParams[closest - 1], nx);
                    state->ui->editParameter((uint32_t)state->timeParams[closest - 1], false);
                    state->ui->updateParamLabel(state->timeParams[closest - 1]);
                }
                lv_obj_invalidate(obj);
            }
        }
        // No scroll bubbling block - allows wheel to bubble up to Sidebar
        // OWNERSHIP CLEANUP: Delete heap-allocated state when LVGL destroys object
        else if (code == LV_EVENT_DELETE) {
            if (state->playTimer) lv_timer_del(state->playTimer);
            delete state;
        }
    }

    static void draw_main(lv_event_t* e) {
        lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
        State* state = (State*)lv_event_get_user_data(e);
        lv_layer_t* layer = lv_event_get_layer(e);
        lv_area_t coords;
        lv_obj_get_coords(obj, &coords);
        int w = lv_obj_get_width(obj);
        int h = lv_obj_get_height(obj);

        // Inset the plot area so points/curves never sit flush against the border.
        const int kPad = 8;
        const int plotW = std::max(1, w - 2 * kPad);
        const int plotH = std::max(1, h - 2 * kPad);
        auto plotX = [&](float t) { return coords.x1 + kPad + (int)(t * plotW); };
        auto plotY = [&](float v) { return coords.y1 + kPad + (int)((1.0f - v) * plotH); };

        // Subtle reference grid so the envelope reads as a calibrated graph.
        lv_draw_line_dsc_t grid_dsc;
        lv_draw_line_dsc_init(&grid_dsc);
        grid_dsc.color = COL_EQ_GRID;
        grid_dsc.width = 1;
        grid_dsc.opa = 110;
        for (int g = 1; g <= 3; ++g) {
            const int gy = plotY((float)g / 4.0f);
            grid_dsc.p1.x = coords.x1; grid_dsc.p1.y = gy;
            grid_dsc.p2.x = coords.x1 + w - 1; grid_dsc.p2.y = gy;
            lv_draw_line(layer, &grid_dsc);
        }
        // Baseline (zero envelope) slightly brighter.
        grid_dsc.color = COL_EQ_GRID_ZERO;
        grid_dsc.opa = 180;
        const int gy0 = plotY(0.0f);
        grid_dsc.p1.x = coords.x1; grid_dsc.p1.y = gy0;
        grid_dsc.p2.x = coords.x1 + w - 1; grid_dsc.p2.y = gy0;
        lv_draw_line(layer, &grid_dsc);

        lv_draw_line_dsc_t line_dsc;
        lv_draw_line_dsc_init(&line_dsc);
        line_dsc.color = COL_HIGHLIGHT;
        line_dsc.width = 2;
        line_dsc.opa = 200;

        for (int i = 0; i < 7; i++) {
            float t0 = (i == 0) ? 0.0f : state->ui->getParamValue(state->timeParams[i-1]);
            float t1 = (i == 6) ? 1.0f : state->ui->getParamValue(state->timeParams[i]);
            float v0 = state->ui->getParamValue(state->valueParams[i]);
            float v1 = state->ui->getParamValue(state->valueParams[i+1]);
            float curve = (state->ui->getParamValue(state->curveParams[i]) - 0.5f) * 2.0f;

            lv_point_t pS = { plotX(t0), plotY(v0) };
            for(int s=1; s<=15; s++) {
                float relT = (float)s / 15.0f;
                float p = 1.0f;
                if (curve > 0) p = 1.0f + curve * 4.0f;
                else p = 1.0f / (1.0f - curve * 4.0f);
                float ct = std::pow(relT, p);
                float outT = t0 + (t1 - t0) * relT;
                float outV = v0 + (v1 - v0) * ct;
                lv_point_t pE = { plotX(outT), plotY(outV) };
                line_dsc.p1.x = pS.x;
                line_dsc.p1.y = pS.y;
                line_dsc.p2.x = pE.x;
                line_dsc.p2.y = pE.y;
                lv_draw_line(layer, &line_dsc);
                pS = pE;
            }
        }

        lv_draw_rect_dsc_t p_dsc;
        lv_draw_rect_dsc_init(&p_dsc);
        p_dsc.radius = LV_RADIUS_CIRCLE;
        p_dsc.border_width = 1;
        p_dsc.border_color = lv_color_white();

        for (int i = 0; i < 8; i++) {
            float tx = (i == 0) ? 0.0f : (i == 7 ? 1.0f : state->ui->getParamValue(state->timeParams[i-1]));
            float ty = state->ui->getParamValue(state->valueParams[i]);
            int px = plotX(tx);
            int py = plotY(ty);
            lv_area_t p_area = {px - 4, py - 4, px + 4, py + 4};
            p_dsc.bg_color = (state->dragPoint == i) ? COL_ACCENT : COL_HIGHLIGHT;
            lv_draw_rect(layer, &p_dsc, &p_area);
        }

        // Live playhead: a cursor line sweeping left -> right across the plot
        // while a note sounds, with a marker dot riding the envelope curve.
        if (state->playPhase >= 0.0f && state->playPhase <= 1.0f) {
            const float t = state->playPhase;
            int seg = 0;
            while (seg < 7) {
                const float tNext = (seg == 6) ? 1.0f : state->ui->getParamValue(state->timeParams[seg]);
                if (t <= tNext) break;
                ++seg;
            }
            if (seg > 6) seg = 6;
            const float t0 = (seg == 0) ? 0.0f : state->ui->getParamValue(state->timeParams[seg - 1]);
            const float t1 = (seg == 6) ? 1.0f : state->ui->getParamValue(state->timeParams[seg]);
            const float v0 = state->ui->getParamValue(state->valueParams[seg]);
            const float v1 = state->ui->getParamValue(state->valueParams[seg + 1]);
            const float curve = (state->ui->getParamValue(state->curveParams[seg]) - 0.5f) * 2.0f;
            const float relT = std::clamp((t - t0) / std::max(0.001f, t1 - t0), 0.0f, 1.0f);
            const float p = (curve > 0) ? (1.0f + curve * 4.0f) : (1.0f / (1.0f - curve * 4.0f));
            const float v = v0 + (v1 - v0) * std::pow(relT, p);
            const int px = plotX(t);
            const int py = plotY(std::clamp(v, 0.0f, 1.0f));

            lv_draw_line_dsc_t ph_dsc;
            lv_draw_line_dsc_init(&ph_dsc);
            ph_dsc.color = COL_HIGHLIGHT;
            ph_dsc.width = 1;
            ph_dsc.opa = 130;
            ph_dsc.p1.x = px; ph_dsc.p1.y = coords.y1 + kPad;
            ph_dsc.p2.x = px; ph_dsc.p2.y = coords.y1 + h - kPad;
            lv_draw_line(layer, &ph_dsc);

            lv_draw_rect_dsc_t ph_dot;
            lv_draw_rect_dsc_init(&ph_dot);
            ph_dot.radius = LV_RADIUS_CIRCLE;
            ph_dot.bg_color = COL_HIGHLIGHT;
            ph_dot.bg_opa = LV_OPA_COVER;
            ph_dot.border_width = 1;
            ph_dot.border_color = lv_color_white();
            lv_area_t ph_area = {px - 3, py - 3, px + 3, py + 3};
            lv_draw_rect(layer, &ph_dot, &ph_area);
        }
    }
};

END_NAMESPACE_DISTRHO

#endif
