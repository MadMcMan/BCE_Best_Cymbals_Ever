#ifndef MOD_CABLE_ROPE_HPP
#define MOD_CABLE_ROPE_HPP

// Verlet-rope mod drag cable (plugin-owned; the shared kit stays untouched).
//
// The old drag wire was a near-straight bezier from the source badge to a
// pendulum tip pinned 28px under the cursor: a stiff stick glued to the
// mouse with a wiggle at the end. A held patch cord behaves the opposite
// way — the plug sits IN the hand (at the cursor) while the cable pays out
// behind it with slack, lag, and swing. This header simulates exactly that:
//
//   - CableRope: pure-math verlet chain (no LVGL/DPF deps, unit-tested in
//     tests/test_cable_rope.cpp). Point 0 is pinned at the source badge,
//     point N is pinned at the hand (cursor). Gravity + payout slack shape
//     the belly; hand motion pumps swing energy through the constraints.
//   - drawCableRope: renders the chain as a polyline on an lv_layer_t.
//     Kept inline + LVGL-only so the sim TU never links LVGL.
//
// Fixed-timestep design: step() assumes one call per UI tick at ~60Hz
// (dt = 1/60), matching the existing 16ms UI timer. Payout smoothing and
// damping are tuned in per-tick units — do NOT call with wall-clock dt.

#include <cmath>

#ifdef LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif
namespace cable_rope {

struct Vec {
    float x = 0.0f;
    float y = 0.0f;
};

struct CableRope {
    static constexpr int kSegs = 12;          // spans; 13 points
    static constexpr int kRelaxIters = 4;     // constraint passes per tick
    static constexpr float kSlack = 1.25f;    // rope pays out to 125% of chord
    static constexpr float kGravity = 2200.0f;// px/s^2 (snappy UI feel)
    static constexpr float kDamping = 0.985f;     // velocity retention per tick
    static constexpr float kPayoutRate = 6.0f;    // payout smoothing per second

    Vec p[kSegs + 1];    // point 0 = badge end, point N = hand end
    Vec pp[kSegs + 1];   // previous positions (verlet velocity)
    float restLen = 0.0f;// total rope length (grows as you pull)
    float segLen = 0.0f; // restLen / kSegs

    static float dist(float ax, float ay, float bx, float by) {
        const float dx = bx - ax, dy = by - ay;
        return std::sqrt(dx * dx + dy * dy);
    }

    // (Re)start a drag: rope runs straight badge -> hand, zero velocity.
    void reset(float ax, float ay, float hx, float hy, float minLen) {
        for (int i = 0; i <= kSegs; ++i) {
            const float t = (float)i / (float)kSegs;
            p[i].x = pp[i].x = ax + (hx - ax) * t;
            p[i].y = pp[i].y = ay + (hy - ay) * t;
        }
        restLen = dist(ax, ay, hx, hy) * kSlack;
        if (restLen < minLen) restLen = minLen;
        segLen = restLen / (float)kSegs;
    }

    // One fixed 1/60s tick. Hand (cursor) pins the last point.
    void step(float ax, float ay, float hx, float hy, float minLen) {
        constexpr float dt = 1.0f / 60.0f;
        // Payout: feed rope toward 125% of the live chord so the belly
        // always has slack, smoothed so fast yanks don't pop.
        float want = dist(ax, ay, hx, hy) * kSlack;
        if (want < minLen) want = minLen;
        restLen += (want - restLen) * kPayoutRate * dt;
        segLen = restLen / (float)kSegs;
        // Verlet integrate interior points (ends are pinned below).
        const float gStep = kGravity * dt * dt;
        for (int i = 1; i < kSegs; ++i) {
            const float vx = (p[i].x - pp[i].x) * kDamping;
            const float vy = (p[i].y - pp[i].y) * kDamping;
            pp[i] = p[i];
            p[i].x += vx;
            p[i].y += vy + gStep;
        }
        // Constrain: pin ends, enforce segment length.
        for (int it = 0; it < kRelaxIters; ++it) {
            p[0].x = ax; p[0].y = ay;
            p[kSegs].x = hx; p[kSegs].y = hy;
            for (int i = 0; i < kSegs; ++i) {
                Vec& a = p[i];
                Vec& b = p[i + 1];
                float dx = b.x - a.x, dy = b.y - a.y;
                float d = std::sqrt(dx * dx + dy * dy);
                if (d < 1e-6f) continue;
                const float diff = (d - segLen) / d;
                const bool pinA = (i == 0);
                const bool pinB = (i + 1 == kSegs);
                if (pinA && pinB) {
                    // Single-span degenerate case (kSegs == 1): split.
                    a.x += dx * 0.5f * diff; a.y += dy * 0.5f * diff;
                    b.x -= dx * 0.5f * diff; b.y -= dy * 0.5f * diff;
                } else if (pinA) {
                    b.x -= dx * diff; b.y -= dy * diff;
                } else if (pinB) {
                    a.x += dx * diff; a.y += dy * diff;
                } else {
                    a.x += dx * 0.5f * diff; a.y += dy * 0.5f * diff;
                    b.x -= dx * 0.5f * diff; b.y -= dy * 0.5f * diff;
                }
            }
        }
        p[0].x = ax; p[0].y = ay;
        p[kSegs].x = hx; p[kSegs].y = hy;
    }
};

// Polyline render of the simulated chain. Same stroke language as the
// persistent cables (2px source-colored wire); the plug dot at the hand
// stays in the caller's draw callback.
inline void drawCableRope(lv_layer_t* layer, const CableRope& rope,
                           lv_color_t color, lv_coord_t width, lv_opa_t opa) {
    if (!layer) return;
    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    dsc.color = color;
    dsc.width = width;
    dsc.opa = opa;
    lv_point_precise_t prev = { rope.p[0].x, rope.p[0].y };
    for (int i = 1; i <= CableRope::kSegs; ++i) {
        lv_point_precise_t cur = { rope.p[i].x, rope.p[i].y };
        dsc.p1 = prev;
        dsc.p2 = cur;
        lv_draw_line(layer, &dsc);
        prev = cur;
    }
}

} // namespace cable_rope

#endif // MOD_CABLE_ROPE_HPP
