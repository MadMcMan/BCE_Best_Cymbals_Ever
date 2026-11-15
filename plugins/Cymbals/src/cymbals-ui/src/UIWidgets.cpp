#include "cymbals-ui/UIWidgets.hpp"
#include <unordered_map>
#include <algorithm>
#include <cmath>

// Discrete snap hook storage (file scope so both anon namespace and UIWidgets methods see it)
static DISTRHO::UIWidgets::DiscreteSnapFn gDiscreteSnap = nullptr;
static void* gDiscreteSnapUserData = nullptr;

namespace {
struct ArcDragState {
    lv_point_t startPoint{0, 0};
    int startValue = 0;
    bool active = false;
};

struct ArcVisualBinding {
    // RETIRED: the rotating needle face. syncSharedArcVisual rotates whatever
    // is in this slot to the value angle, and the material rework replaced the
    // rotating needle with the static pointer stroke - so this is deliberately
    // left NULL. Do NOT park the cap's face disc here: the face carries a
    // vertical gradient, so rotating it sweeps the knob's highlight around with
    // the value (looks like a stray dot drifting across the knob). The face
    // disc has its own slot below.
    lv_obj_t* face = nullptr;
    lv_obj_t* ptr = nullptr;   // rim-segment value pointer (static, no rotation)
    lv_obj_t* cap = nullptr;   // cap widget (re-centered on arc every draw)
    // The cap's face disc, held ONLY so hover/press feedback can lift it. Never
    // rotated - see the note on `face` above.
    lv_obj_t* capFace = nullptr;
};

// Angular span of the value pointer arc, in degrees. Shared by the creation
// (below) and syncSharedArcVisual (which re-applies it each value change) so
// the two can never disagree about the pointer's width.
static constexpr int kKnobPointerSpanDeg = 6;

static std::unordered_map<lv_obj_t*, ArcDragState> gArcDragStates;
// Shift-drag precision. Set by the host layer (see UIWidgets.hpp); read by
// arcDragCb on the LVGL thread, so a plain bool is sufficient - both run on the
// UI thread.
static bool gFineDragActive = false;
// 1/10 travel: the full 0..1000 range needs ~2860px instead of ~286px.
static constexpr float kFineDragFactor = 0.1f;
static std::unordered_map<lv_obj_t*, ArcVisualBinding> gArcVisualBindings;

static int sharedArcAngleFromValue(int value) {
    const int clamped = std::clamp(value, 0, 1000);
    return (2250 + (clamped * 2700 / 1000)) % 3600;
}

static lv_obj_t* findSharedArcFace(lv_obj_t* arc) {
    auto it = gArcVisualBindings.find(arc);
    if (it == gArcVisualBindings.end()) return nullptr;
    return it->second.face;
}

static void syncSharedArcVisual(lv_obj_t* arc) {
    if (!arc) return;
    lv_obj_t* face = findSharedArcFace(arc);
    if (face) {
        lv_obj_set_style_transform_angle(face, sharedArcAngleFromValue(lv_arc_get_value(arc)), 0);
        lv_obj_invalidate(face);
    }
    // Value pointer: a 4-degree arc stroke whose outer radius IS the cap face
    // radius (sized in layoutKnobCapMaterial), so the stroke sweeps a
    // full-diameter radial LINE across the face instead of a rim tick. Native
    // arc — no rotated-buffer resampling at 1x. Same value path as ever.
    auto b = gArcVisualBindings.find(arc);
    if (b == gArcVisualBindings.end() || !b->second.ptr) return;
    const int center = 135 + lv_arc_get_value(arc) * 270 / 1000;
    lv_arc_set_rotation(b->second.ptr, center - kKnobPointerSpanDeg / 2);
    lv_arc_set_bg_angles(b->second.ptr, 0, kKnobPointerSpanDeg);
    lv_arc_set_value(b->second.ptr, 1000);
    lv_obj_invalidate(b->second.ptr);
}

static void sharedArcVisualEventCb(lv_event_t* e) {
    lv_obj_t* arc = (lv_obj_t*)lv_event_get_target(e);
    const lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        syncSharedArcVisual(arc);
        return;
    }

    if (code == LV_EVENT_DELETE) {
        gArcVisualBindings.erase(arc);
    }
}

// DRAW_POST: re-center the cap on the arc's current center every draw. The
// cap's x,y were captured at create time via lv_obj_center; if the parent
// (knobCont) or any ancestor's transform shifts the arc, the cap can drift.
// Recomputing from the arc's current coords on every draw keeps the cap and
// arc visually concentric regardless of any transform the parent carries.
// Guarded: only re-aligns if the cap's current center differs from the arc's
// by more than 1 px, to avoid an infinite invalidation loop (realign ->
// invalidate -> redraw -> realign -> ...).
static void sharedArcCapRealignCb(lv_event_t* e) {
    auto* arc = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
    auto it = gArcVisualBindings.find(arc);
    if (it == gArcVisualBindings.end() || !it->second.cap) return;
    lv_obj_t* cap = it->second.cap;
    // Compute the cap's current center vs. the arc's current center. Only
    // re-align when they differ by more than 1 px; the cap's own DRAW_POST
    // would otherwise trigger an infinite re-alignment loop.
    lv_area_t arcA, capA;
    lv_obj_get_coords(arc, &arcA);
    lv_obj_get_coords(cap, &capA);
    const lv_coord_t arcCx = (lv_coord_t)((int)arcA.x1 + (int)arcA.x2) / 2;
    const lv_coord_t arcCy = (lv_coord_t)((int)arcA.y1 + (int)arcA.y2) / 2;
    const lv_coord_t capCx = (lv_coord_t)((int)capA.x1 + (int)capA.x2) / 2;
    const lv_coord_t capCy = (lv_coord_t)((int)capA.y1 + (int)capA.y2) / 2;
    if (LV_ABS(arcCx - capCx) <= 1 && LV_ABS(arcCy - capCy) <= 1) return;
    lv_obj_align_to(cap, arc, LV_ALIGN_CENTER, 0, 0);
}
// Teal modulation sweep arc around routed knobs only (hidden otherwise).
// Sized LARGER than the knob and centred on it, so it renders as a concentric
// band OUTSIDE the knob rim (the knob container has LV_OBJ_FLAG_OVERFLOW_VISIBLE).
// Created AFTER the value arc so the sweep paints over the arc's dark track.
// Its user_data sentinel (-2) distinguishes it from the main arc (param index);
// findModRing() scans for it (child order is NOT guaranteed).

// LVGL clips children to a container's coords + ext_draw_size even with
// OVERFLOW_VISIBLE. Expand the ext area (computed from styles) via the
// REFR_EXT_DRAW_SIZE hook so the mod band ring can render past the knob.
static void allowChildOverflow(lv_obj_t* obj) {
    lv_obj_add_flag(obj, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_add_event_cb(obj, [](lv_event_t* e) {
        if (lv_event_get_code(e) == LV_EVENT_REFR_EXT_DRAW_SIZE) {
            int32_t* ext = (int32_t*)lv_event_get_param(e);
            *ext += scaled(20);          // ring is arc+28 → 14 per side, +6 for dot
            lv_event_stop_bubbling(e);
        }
    }, LV_EVENT_ALL, nullptr);
}

static lv_obj_t* createModRing(lv_obj_t* parent, lv_coord_t arcSize) {
    lv_obj_t* ring = lv_arc_create(parent);
    // 14 px of band on each side of the knob rim — clearly separated from the
    // rim so the modulation adjustment reads as a distinct outer band.
    const lv_coord_t ringSize = arcSize + scaled(28);
    lv_obj_set_size(ring, ringSize, ringSize);
    lv_obj_center(ring);
    lv_arc_set_rotation(ring, 135);
    lv_arc_set_bg_angles(ring, 0, 270);
    lv_arc_set_range(ring, 0, 1000);
    lv_arc_set_value(ring, 0);
    // Static range line (MAIN) + moving dot (KNOB). The line shows the full
    // possible modulation excursion (min→max); the dot shows the live value.
    // Previously the line itself was live (lo = base, hi = base+delta) which
    // caused 60 Hz flicker as the LFO moved. Now MAIN is static, INDICATOR
    // is hidden, and KNOB is the only moving part.
    lv_obj_set_style_arc_color(ring, COL_MOD_DIM, LV_PART_MAIN);
    lv_obj_set_style_arc_width(ring, scaled(2), LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(ring, true, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(ring, LV_OPA_COVER, LV_PART_MAIN);
    // Hide the bright INDICATOR fill – we only want the thin MAIN track plus dot.
    lv_obj_set_style_arc_opa(ring, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(ring, true, LV_PART_INDICATOR);
    lv_obj_set_style_opa(ring, LV_OPA_COVER, 0);
    // Kill any inherited/set arc-value animation: with the shared-arc knob
    // style applying anim_duration 200 ms, any downstream animation path
    // (theme, future LVGL version) would make the live dot lag the LFO.
    // A routed ring must track its source instantly.
    lv_obj_set_style_anim_duration(ring, 0, 0);
    // Dot at the live value: 6 px circle, source color (set per-tick in PluginUI).
    lv_obj_set_style_bg_opa(ring, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_bg_color(ring, COL_MOD, LV_PART_KNOB);
    lv_obj_set_style_border_width(ring, scaled(2), LV_PART_KNOB);
    lv_obj_set_style_border_color(ring, COL_BG, LV_PART_KNOB);
    lv_obj_set_style_width(ring, scaled(6), LV_PART_KNOB);
    lv_obj_set_style_height(ring, scaled(6), LV_PART_KNOB);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, LV_PART_KNOB);
    lv_obj_set_style_pad_all(ring, 0, LV_PART_KNOB);
    lv_obj_set_user_data(ring, (void*)(intptr_t)(-2));
    lv_obj_add_flag(ring, LV_OBJ_FLAG_HIDDEN); // revealed only when routed
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_CLICKABLE);
    return ring;
}

// ---------------------------------------------------------------------------
// Cap material — the ONE definition. Every knob in this kit (createArc,
// createArcEx, createSimpleArcWithDrag) is built by createSharedArcKnob, which
// calls layoutKnobCapMaterial() and nothing else ever touches the cap's
// material. There is deliberately no second definition anywhere: the plugin
// used to re-derive this material in ui/KnobCapSoften.hpp, matched kit bands
// by (colour,width) tuple and re-patched them at ~35 call sites — one kit
// change silently broke every knob, and "one knob" was 13 places. It is all
// here now, next to the only factory that builds caps.
//
// Look (AvonSynth VCO module): a BLACK panel carrying SILVER knurled caps —
//   * body/rim — the cap itself is the turned outer cylinder: bright steel on
//                the lit (upper-left) sweep shading to dark steel on the
//                shadow side, with a TIGHT ring of fine radial teeth cut into
//                it. Each tooth is a ridge (light) alternating with a groove
//                (dark) so the rim knits contrast instead of reading as a flat
//                band — the single strongest cue of the reference. Tooth
//                pitch is ~1/9 of the cap diameter, clamped so a 10px cap and
//                a 54px cap both get a legible knurl (~14..60 teeth).
//   * face     — a flat top disc with a machined lip where it meets the
//                knurl, plus a soft top-left specular and a bottom-right
//                shade so the disc reads as a turned solid
//   * pointer  — a bright radial line across the face, from behind the hub to
//                the knurl (the reference's readout line; longer and brighter
//                than the old rim segment)
//   * hub      — a dark centre boss the pointer rides over
// Everything is derived from capSize alone, so a 10px cap and a 54px cap come
// out of the same arithmetic: tooth count/pitch, rib width, face inset and
// pointer length are all fractions of capSize, clamped so they never
// degenerate (integer angles are rounded, so step is recomputed from the real
// tooth count — never the other way round).
// The cap is a square with LV_RADIUS_CIRCLE, so every child is centred and no
// material can exceed the cap box (knob containers are 28px ring-padded).
//
// LAYER ORDER (children, back to front):
//   cap bg (steel cylinder) -> rib teeth (alternating ridge/groove)
//   -> face (lip + disc + specular + shade) -> pointer -> hub
// The old `rim` / `face` / `shading` children are gone entirely; the cap's own
// style body now IS the metal.
//
// Cost: each tooth is one lv_arc. A 54px cap builds 60 teeth + lip/disc/
// specular/shade + hub + pointer = 65 children; a 16px arc (10px cap) gets
// 14 teeth + 5 = 19. All are non-clickable with zero KNOB parts, so
// drag/hover/value sync stay on the value arc (untouched), and the audit's
// KNOBBANDS scan still reports 0 unhandled band-arcs.
//
// capSize is in DESIGN pixels (unscaled). Returns the face radius in design
// px (0 when the cap is too small for a face), which createSharedArcKnob
// reuses to size the value pointer's radial line.
// faceOut (optional) receives the face disc, so the hover/press feedback can
// lift the one big visible surface without searching the tree for it.
static int layoutKnobCapMaterial(lv_obj_t* cap, int capSize,
                                 lv_obj_t** faceOut = nullptr) {
    if (faceOut) *faceOut = nullptr;
    if (!cap || capSize <= 0) return 0;

    const int r = capSize / 2;                 // radius in design px
    if (r < 4) return 0;                       // sub-8px caps: no room for a cap

    // --- the cap body is the turned outer cylinder: steel, lit top-left ----
    lv_obj_set_style_bg_color(cap, COL_KNOB_STEEL_LIT, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(cap, COL_KNOB_STEEL_DEEP, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(cap, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cap, LV_OPA_COVER, LV_PART_MAIN);

    // --- knurl: fine radial teeth on the outer band -------------------------
    // Knurling, not gear teeth: many SHALLOW ribs. Depth was 28% of the
    // radius, which on a 54px cap gave 7px notches that read as a gear; 18%
    // keeps the band reading as a turned/knurled cylinder.
    const int ribW = std::max(1, (r * 18) / 100);  // rib depth, ~18% of radius
    const int ribR = r - std::max(1, r / 12);      // outer edge, clear of bev
    const int ribS = std::max(3, 2 * ribR);        // arc bbox (never degenerates)
    // Tooth pitch is a fraction of the cap DIAMETER (pitch ~= d/14, clamped to
    // 2.5..4.5 px) so a 10px cap still gets ~12 teeth and a 54px cap ~44 -
    // fine enough to read as knurling rather than a gear. Never derive the
    // pitch from the tooth count: int-division collapsed that to a fixed 12.
    const double pitchPx = std::clamp((double)capSize / 14.0, 2.5, 4.5);
    const int teeth = std::clamp((int)std::lround(3.14159265 * capSize / pitchPx), 12, 60);
    const int step = 360 / teeth;                  // deg per tooth slot
    const int ridge = std::max(1, step * 55 / 100);// ridge share of the slot
    for (int i = 0; i < teeth; ++i) {
        // Tooth centre angle, 0deg = 3 o'clock, clockwise. The lit sweep is
        // upper-left (about 160..290deg), so the key light reads as upper-left
        // and the shadow side falls on the lower-right, matching the face
        // gradient below.
        const int centre = i * step + step / 2;
        const bool lit = (centre >= 160 && centre <= 290);
        lv_obj_t* rib = lv_arc_create(cap);
        lv_obj_set_size(rib, ribS, ribS);
        lv_obj_center(rib);
        lv_arc_set_rotation(rib, 0);
        lv_arc_set_bg_angles(rib, (int16_t)(i * step + (step - ridge) / 2),
                             (int16_t)(i * step + (step + ridge) / 2));
        lv_arc_set_range(rib, 0, 1000);
        lv_arc_set_value(rib, 1000);   // indicator spans the whole bg arc
        lv_obj_set_style_bg_opa(rib, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_arc_width(rib, 0, LV_PART_MAIN);
        // Alternate ridge (brighter) and groove (darker) around the ring; the
        // whole band also tracks the light direction.
        const bool ridgeTooth = (i & 1) == 0;
        const lv_color_t ribCol = ridgeTooth
            ? (lit ? COL_KNOB_STEEL_LIT : COL_KNOB_STEEL_MID)
            : (lit ? COL_KNOB_STEEL_SHAD : COL_KNOB_STEEL_DEEP);
        lv_obj_set_style_arc_color(rib, ribCol, LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(rib, ribW, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(rib, LV_OPA_COVER, LV_PART_INDICATOR);
        lv_obj_set_style_arc_rounded(rib, false, LV_PART_INDICATOR);
        // Zero out the value-knob part so only the band renders.
        lv_obj_set_style_pad_all(rib, 0, LV_PART_KNOB);
        lv_obj_set_style_bg_opa(rib, LV_OPA_TRANSP, LV_PART_KNOB);
        lv_obj_set_style_border_width(rib, 0, LV_PART_KNOB);
        lv_obj_set_style_shadow_width(rib, 0, LV_PART_INDICATOR);
        lv_obj_clear_flag(rib, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(rib, LV_OBJ_FLAG_SCROLLABLE);
    }

    // --- face: flat top disc with a machined lip -----------------------------
    const int faceR = ribR - ribW;
    if (faceR >= 3) {
        // lip: the turned bevel where face meets knurl
        lv_obj_t* lip = lv_obj_create(cap);
        const int lipS = std::max(3, 2 * (faceR + std::max(1, r / 14)));
        lv_obj_set_size(lip, lipS, lipS);
        lv_obj_center(lip);
        lv_obj_set_style_radius(lip, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(lip, COL_KNOB_STEEL_MID, 0);
        lv_obj_set_style_bg_opa(lip, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(lip, 0, 0);
        lv_obj_set_style_shadow_width(lip, 0, LV_PART_MAIN);
        lv_obj_clear_flag(lip, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(lip, LV_OBJ_FLAG_SCROLLABLE);

        // face body: flat top, cool lit steel, cooler shadow at the bottom
        lv_obj_t* face = lv_obj_create(cap);
        if (faceOut) *faceOut = face;
        const int faceS = std::max(3, 2 * faceR);
        lv_obj_set_size(face, faceS, faceS);
        lv_obj_center(face);
        lv_obj_set_style_radius(face, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(face, COL_KNOB_STEEL_LIT, 0);
        lv_obj_set_style_bg_grad_color(face, COL_KNOB_STEEL_SHAD, 0);
        lv_obj_set_style_bg_grad_dir(face, LV_GRAD_DIR_VER, 0);
        lv_obj_set_style_bg_opa(face, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(face, 0, 0);
        lv_obj_set_style_shadow_width(face, 0, LV_PART_MAIN);
        lv_obj_clear_flag(face, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(face, LV_OBJ_FLAG_SCROLLABLE);

        // specular: soft top-left highlight on the disc
        lv_obj_t* spec = lv_obj_create(cap);
        const int specS = std::max(3, (2 * faceR * 40) / 100);
        lv_obj_set_size(spec, specS, specS);
        lv_obj_center(spec);
        lv_obj_set_style_radius(spec, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(spec, COL_KNOB_STEEL_LIT, 0);
        lv_obj_set_style_bg_opa(spec, LV_OPA_50, 0);
        lv_obj_set_style_border_width(spec, 0, 0);
        lv_obj_clear_flag(spec, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(spec, LV_OBJ_FLAG_SCROLLABLE);
        // Anchor to the CENTRE, not the cap bbox corner: the cap is a circle,
        // so its bounding-box corners fall outside it and a corner-aligned
        // child renders as a blob hanging off the rim. faceR/3 diagonally is
        // 0.47*faceR from centre; plus the 0.4*faceR radius it stays inside.
        lv_obj_align(spec, LV_ALIGN_CENTER, -std::max(1, faceR / 3),
                     -std::max(1, faceR / 3));

        // shade: bottom-right occlusion so the disc reads as a turned solid
        lv_obj_t* shade = lv_obj_create(cap);
        const int shadeS = std::max(3, (2 * faceR * 55) / 100);
        lv_obj_set_size(shade, shadeS, shadeS);
        lv_obj_set_style_radius(shade, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(shade, COL_KNOB_STEEL_SHAD, 0);
        lv_obj_set_style_bg_opa(shade, LV_OPA_40, 0);
        lv_obj_set_style_border_width(shade, 0, 0);
        lv_obj_clear_flag(shade, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(shade, LV_OBJ_FLAG_SCROLLABLE);
        // Same corner bug as the specular: 0.354*faceR offset + 0.55*faceR
        // radius = 0.90*faceR, inside the disc.
        lv_obj_align(shade, LV_ALIGN_CENTER, std::max(1, faceR / 4),
                     std::max(1, faceR / 4));
        return faceR;
    }

    return 0;
}

// Lift the cap face on hover and lift it further while pressed, so the knob
// responds to the pointer before and during a drag. The face's colours are
// LOCAL styles (set in layoutKnobCapMaterial), and a local style beats an added
// one - so this writes the local style directly rather than adding a style.
static void knobHoverCb(lv_event_t* e) {
    lv_obj_t* arc = static_cast<lv_obj_t*>(lv_event_get_target(e));
    auto it = gArcVisualBindings.find(arc);
    if (it == gArcVisualBindings.end()) return;
    lv_obj_t* face = it->second.capFace;
    if (!face) return;

    const lv_event_code_t code = lv_event_get_code(e);
    bool lift;
    lv_opa_t amt;
    switch (code) {
        case LV_EVENT_HOVER_OVER:  lift = true;  amt = LV_OPA_20; break;
        case LV_EVENT_PRESSED:     lift = true;  amt = LV_OPA_40; break;
        case LV_EVENT_RELEASED:
        case LV_EVENT_PRESS_LOST:
            // A release over the knob leaves it hovered; over the void, plain.
            lift = lv_obj_has_state(arc, LV_STATE_HOVERED);
            amt = LV_OPA_20;
            break;
        case LV_EVENT_HOVER_LEAVE:
            // Still pressed (dragging away from the knob): keep the press lift.
            lift = lv_obj_has_state(arc, LV_STATE_PRESSED);
            amt = LV_OPA_40;
            break;
        default: return;
    }

    lv_obj_set_style_bg_color(face, lift ? lv_color_lighten(COL_KNOB_STEEL_LIT, amt)
                                         : COL_KNOB_STEEL_LIT, 0);
    lv_obj_set_style_bg_grad_color(face, lift ? lv_color_lighten(COL_KNOB_STEEL_SHAD, amt)
                                              : COL_KNOB_STEEL_SHAD, 0);
    lv_obj_invalidate(face);
}

static lv_obj_t* createSharedArcKnob(lv_obj_t* parent,
                                     lv_coord_t arcSize,
                                     UIStyles& styles,
                                     const UIStyles::KnobVisualGeometry& geometry,
                                     float initialValue,
                                     const lv_color_t* indicatorOverride = nullptr,
                                     bool withModRing = false) {
    // parent is the knob container in both callers; let the mod band ring
    // (sized arcSize + 28) render outside the knob rim.
    allowChildOverflow(parent);
    lv_obj_t* arc = lv_arc_create(parent);
    lv_obj_set_size(arc, arcSize, arcSize);
    lv_obj_center(arc);
    // (lv_obj_constructor already sets PRESS_LOCK by default: the drag stays
    // alive when the pointer slides off the small knob. Full-range reach is
    // the drag sensitivity's job — see arcDragCb's travel factor.)
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_arc_set_range(arc, 0, 1000);
    lv_arc_set_value(arc, (int)(initialValue * 1000.0f));

    lv_obj_add_style(arc, &styles.arcMain, LV_PART_MAIN);
    lv_obj_add_style(arc, &styles.arcIndicator, LV_PART_INDICATOR);
    lv_obj_add_style(arc, &styles.arcKnob, LV_PART_KNOB);
    lv_obj_set_style_anim_duration(arc, 200, 0);

    // Modulation sweep ring sits above the value arc (visible over its dark
    // track) but below the cap.
    if (withModRing) createModRing(parent, arcSize);

    const int capSize = std::max(arcSize - geometry.capInset, 8);
    lv_obj_t* cap = lv_obj_create(parent);
    lv_obj_set_size(cap, capSize, capSize);
    lv_obj_center(cap);
    lv_obj_add_style(cap, &styles.knobCap, 0);
    lv_obj_clear_flag(cap, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cap, LV_OBJ_FLAG_SCROLLABLE);

    // Cap material — the single definition for EVERY knob size (see
    // layoutKnobCapMaterial). Nothing downstream patches this: there is no
    // second material definition anywhere in the tree.
    lv_obj_t* face = nullptr;
    const int faceR = layoutKnobCapMaterial(cap, capSize, &face);

    // Value pointer: a radial LINE across the face from behind the hub out to
    // the knurl edge (the reference's readout line, not a rim segment). It is
    // still a native lv_arc so there is no rotated-buffer resampling at 1x —
    // the stroke's radial depth (faceR - hubR) makes it a LINE, not a rim tick,
    // and its angular span gives it the visible width the old 4deg/2-3px
    // segment lacked. Same value path (syncSharedArcVisual), same colour.
    const int hubR = (faceR * 20) / 100;      // centre boss radius, design px
    const int ptrLen = faceR - hubR;          // radial depth of the line
    const int ptrS = std::max(3, 2 * faceR);  // bbox = 2*faceR
    lv_obj_t* ptr = lv_arc_create(cap);
    lv_obj_set_size(ptr, ptrS, ptrS);
    lv_obj_center(ptr);
    lv_arc_set_bg_angles(ptr, 0, kKnobPointerSpanDeg);
    lv_arc_set_range(ptr, 0, 1000);
    lv_obj_set_style_bg_opa(ptr, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(ptr, 0, LV_PART_MAIN);
    // BLACK marker line, per the reference knob: a bright turned-steel face
    // with a dark radial index line. White read as a specular streak on the
    // lit face rather than a marker.
    lv_obj_set_style_arc_color(ptr, COL_BLACK, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(ptr, ptrLen, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(ptr, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(ptr, false, LV_PART_INDICATOR);
    lv_obj_set_style_pad_all(ptr, 0, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(ptr, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_border_width(ptr, 0, LV_PART_KNOB);
    lv_obj_clear_flag(ptr, LV_OBJ_FLAG_CLICKABLE);

    // Hub: dark centre boss the pointer rides over. Sized in ONE place (here)
    // so the pointer and the hub agree on where the line starts.
    if (faceR >= 3 && hubR >= 1) {
        const int hubS = std::max(2, 2 * hubR);
        lv_obj_t* hub = lv_obj_create(cap);
        lv_obj_set_size(hub, hubS, hubS);
        lv_obj_center(hub);
        lv_obj_set_style_radius(hub, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(hub, COL_KNOB_STEEL_DEEP, 0);
        lv_obj_set_style_bg_opa(hub, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(hub, 0, 0);
        lv_obj_clear_flag(hub, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(hub, LV_OBJ_FLAG_SCROLLABLE);
    }

    // The old needle face (an empty circle + a shading child that rotated with
    // the value) is gone with the material rework — the pointer line above is
    // the whole value indication. syncSharedArcVisual handles a missing face.
    // Hover/press feedback: the arc answers natively via state-selected styles
    // (arcMainHover/arcIndicatorHover/arcIndicatorPress), and the cap face is
    // lifted here because it is a SIBLING of the arc - LVGL state does not
    // propagate between siblings, so the ring waking up must be mirrored onto
    // the face explicitly.
    // capFace, NOT face: see the note on the struct - `face` is the retired
    // rotation slot and filling it in sweeps the highlight with the value.
    gArcVisualBindings[arc] = {nullptr, ptr, cap, face};
    lv_obj_add_style(arc, &styles.arcMainHover, LV_PART_MAIN | LV_STATE_HOVERED);
    lv_obj_add_style(arc, &styles.arcIndicatorHover, LV_PART_INDICATOR | LV_STATE_HOVERED);
    lv_obj_add_style(arc, &styles.arcIndicatorPress, LV_PART_INDICATOR | LV_STATE_PRESSED);
    lv_obj_add_event_cb(arc, knobHoverCb, LV_EVENT_HOVER_OVER, nullptr);
    lv_obj_add_event_cb(arc, knobHoverCb, LV_EVENT_HOVER_LEAVE, nullptr);
    lv_obj_add_event_cb(arc, knobHoverCb, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(arc, knobHoverCb, LV_EVENT_RELEASED, nullptr);
    lv_obj_add_event_cb(arc, knobHoverCb, LV_EVENT_PRESS_LOST, nullptr);
    syncSharedArcVisual(arc);
    lv_obj_add_event_cb(arc, sharedArcVisualEventCb, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(arc, sharedArcVisualEventCb, LV_EVENT_DELETE, nullptr);
    lv_obj_add_event_cb(arc, sharedArcCapRealignCb, LV_EVENT_DRAW_POST, nullptr);

    return arc;
}

static void arcDragCb(lv_event_t* e) {
    lv_obj_t* arc = (lv_obj_t*)lv_event_get_target(e);
    const lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_DELETE) {
        gArcDragStates.erase(arc);
        return;
    }

    lv_indev_t* indev = lv_indev_get_act();
    if (!indev) return;

    if (code == LV_EVENT_PRESSED) {
        lv_point_t p;
        lv_indev_get_point(indev, &p);
        ArcDragState& st = gArcDragStates[arc];
        st.startPoint = p;
        st.startValue = lv_arc_get_value(arc);
        // Sustained gesture: signal begin on first press (guard against duplicate PRESSED events).
        AbstractCymbalsUI* ui = static_cast<AbstractCymbalsUI*>(lv_event_get_user_data(e));
        if (ui && !st.active) {
            int paramIndex = (int)(intptr_t)lv_obj_get_user_data(arc);
            ui->editParameter((uint32_t)paramIndex, true);
        }
        st.active = true;
        lv_event_stop_bubbling(e);
        return;
    }

    if (code == LV_EVENT_PRESSING) {
        auto it = gArcDragStates.find(arc);
        if (it == gArcDragStates.end() || !it->second.active) return;

        lv_point_t p;
        lv_indev_get_point(indev, &p);
        const int dx = p.x - it->second.startPoint.x;
        const int dy = p.y - it->second.startPoint.y;

        // Up/right increase, down/left decrease. Sensitivity: full 0..1000
        // range in ~286px of travel (was 455px at 2.2 — unreachable within
        // the window from a mid-screen press, which capped the value short of
        // the full circle).
        const float travel = (float)dx - (float)dy;
        const float sens = gFineDragActive ? 3.5f * kFineDragFactor : 3.5f;
        int next = std::clamp((int)std::lround((float)it->second.startValue + travel * sens), 0, 1000);

        if (gDiscreteSnap) {
            AbstractCymbalsUI* ui = static_cast<AbstractCymbalsUI*>(lv_event_get_user_data(e));
            int paramIndex = (int)(intptr_t)lv_obj_get_user_data(arc);
            next = gDiscreteSnap(ui, paramIndex, next, gDiscreteSnapUserData);
            next = std::clamp(next, 0, 1000);
        }

        if (next != lv_arc_get_value(arc)) {
            lv_arc_set_value(arc, next);
            // Preserve the original user data so the callback can access UI state.
            void* userData = lv_event_get_user_data(e);
            lv_obj_send_event(arc, LV_EVENT_VALUE_CHANGED, userData);
        }
        lv_event_stop_bubbling(e);
        return;
    }

    // Round 2 Piece 1 (knob double-click reset): on DOUBLE_CLICKED, set the
    // parameter to its canonical default. Gate on the mod-ring sentinel (-2)
    // and discrete (non-continuous) params so a filter-type selector is not
    // silently re-zeroed. End any in-flight drag first so the host receives
    // a clean post-reset value rather than a stale release.
    if (code == LV_EVENT_DOUBLE_CLICKED) {
        const int paramIndex = (int)(intptr_t)lv_obj_get_user_data(arc);
        if (paramIndex < 0) return;  // mod ring / non-param widget
        AbstractCymbalsUI* ui = static_cast<AbstractCymbalsUI*>(lv_event_get_user_data(e));
        if (!ui) return;
        if (!ui->isParameterContinuous((uint32_t)paramIndex)) return;
        {
            auto it = gArcDragStates.find(arc);
            if (it != gArcDragStates.end()) it->second.active = false;
        }
        const float defNorm = ui->getParameterDefault((uint32_t)paramIndex);
        const int defArc = std::clamp((int)std::lround(defNorm * 1000.0f), 0, 1000);
        if (defArc != lv_arc_get_value(arc)) {
            lv_arc_set_value(arc, defArc);
            // Preserve the original user data so the consumer-side callback
            // (the host's valueChangedCallback) can recover the UI handle.
            void* userData = lv_event_get_user_data(e);
            lv_obj_send_event(arc, LV_EVENT_VALUE_CHANGED, userData);
        }
        ui->setParamValue((uint32_t)paramIndex, defNorm);
        lv_event_stop_bubbling(e);
        return;
    }

    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        auto it = gArcDragStates.find(arc);
        if (it != gArcDragStates.end()) {
            it->second.active = false;
            // Sustained gesture: signal end to close the gesture bracket.
            AbstractCymbalsUI* ui = static_cast<AbstractCymbalsUI*>(lv_event_get_user_data(e));
            if (ui) {
                int paramIndex = (int)(intptr_t)lv_obj_get_user_data(arc);
                ui->editParameter((uint32_t)paramIndex, false);
            }
        }
        lv_event_stop_bubbling(e);
    }
}
} // namespace (UIWidgets helpers)

START_NAMESPACE_DISTRHO

bool UIWidgets::isArcInGesture(lv_obj_t* arc) {
    auto it = gArcDragStates.find(arc);
    return it != gArcDragStates.end() && it->second.active;
}

void UIWidgets::attachArcDrag(lv_obj_t* arc, AbstractCymbalsUI* ui) {
    if (!arc) return;
    lv_obj_add_event_cb(arc, arcDragCb, LV_EVENT_ALL, ui);
}

lv_obj_t* UIWidgets::findModRing(lv_obj_t* knobCont) {
    if (!knobCont || !lv_obj_is_valid(knobCont)) return nullptr;
    const uint32_t n = lv_obj_get_child_count(knobCont);
    for (uint32_t i = 0; i < n; ++i) {
        lv_obj_t* c = lv_obj_get_child(knobCont, i);
        if (c && (intptr_t)lv_obj_get_user_data(c) == -2) return c;
    }
    return nullptr;
}

// Sweep the arc to its new value over 200ms. SAFE against the feedback loop
// that a previous implementation hit: the only LV_EVENT_VALUE_CHANGED send
// sites in lv_arc.c are inside lv_arc_event (the input path, lines ~713/747/
// 757) - lv_arc_set_value -> value_update never fires it. So animating the
// value cannot re-enter the parameter write and snap the knob back. User
// gestures do NOT come through here (arcDragCb sets the value directly so the
// pointer tracks the cursor 1:1); this is the programmatic path - preset
// loads and host automation - where a jump reads as a glitch.
static void arcValueAnimCb(void* var, int32_t v) {
    lv_obj_t* arc = static_cast<lv_obj_t*>(var);
    lv_arc_set_value(arc, v);
    syncSharedArcVisual(arc);
}

// --- draggable scrollbar (LVGL 9 has none; see the header) ------------------
namespace {
struct ScrollbarDragState {
    bool active = false;
    lv_coord_t grabOffset = 0;   // pointer Y minus the thumb's top, at press time
};
std::unordered_map<lv_obj_t*, ScrollbarDragState> gScrollbarDrags;

// Vertical scrollbar track, or false when there is none. The area comes back in
// ABSOLUTE screen coords (lv_obj_scroll.c sets ver_area->y1 = obj->coords.y1),
// so it must NOT have the object's position added again - doing that put the bar
// outside the object and every press missed it.
bool verticalScrollbarArea(lv_obj_t* obj, lv_area_t* out) {
    lv_area_t hor{}, ver{};
    lv_obj_get_scrollbar_area(obj, &hor, &ver);
    if (ver.x2 < ver.x1 || ver.y2 < ver.y1) return false;
    *out = ver;
    return true;
}

void scrollbarDragCb(lv_event_t* e) {
    lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
    const lv_event_code_t code = lv_event_get_code(e);
    ScrollbarDragState& st = gScrollbarDrags[obj];

    if (code == LV_EVENT_DELETE) {
        gScrollbarDrags.erase(obj);
        return;
    }
    if (!lv_obj_has_flag(obj, LV_OBJ_FLAG_SCROLLABLE)) return;

    lv_indev_t* indev = lv_indev_get_act();
    if (!indev) return;
    lv_point_t p;
    lv_indev_get_point(indev, &p);

    if (code == LV_EVENT_PRESSED) {
        lv_area_t bar{};
        if (!verticalScrollbarArea(obj, &bar)) return;
        // applyContentFit() scales the fixed UI canvas; scrollbar geometry is
        // in canvas coordinates while indev points are raw window pixels.
        lv_obj_get_transformed_area(obj, &bar, LV_OBJ_POINT_TRANSFORM_FLAG_RECURSIVE);
        if (p.x < bar.x1 || p.x > bar.x2 || p.y < bar.y1 || p.y > bar.y2) return;
        st.active = true;
        // Grab where the user grabbed: the thumb must follow the pointer rather
        // than jump so its centre lands under it.
        st.grabOffset = (lv_coord_t)((int)p.y - (int)bar.y1);
        return;
    }

    if (code == LV_EVENT_PRESSING && st.active) {
        lv_area_t bar{};
        if (!verticalScrollbarArea(obj, &bar)) { st.active = false; return; }
        lv_obj_get_transformed_area(obj, &bar, LV_OBJ_POINT_TRANSFORM_FLAG_RECURSIVE);
        // Invert LVGL's own thumb model (lv_obj_scroll.c):
        //   sb_h  = thumb length, rem = (obj_h - pad) - sb_h = how far it TRAVELS
        //   sb_y  = rem * scroll_top / scroll_h
        // so the thumb's top offset within the track maps linearly to the scroll
        // position. Mapping the pointer inside the THUMB's own area instead (the
        // first attempt) is wrong: the thumb is short, so it barely moves and the
        // list hardly scrolls.
        const lv_coord_t top_space = lv_obj_get_style_pad_top(obj, LV_PART_SCROLLBAR);
        const lv_coord_t bot_space = lv_obj_get_style_pad_bottom(obj, LV_PART_SCROLLBAR);
        const int thumbLen = (int)bar.y2 - (int)bar.y1 + 1;
        lv_area_t objCoords{};
        lv_obj_get_coords(obj, &objCoords);
        lv_obj_get_transformed_area(obj, &objCoords, LV_OBJ_POINT_TRANSFORM_FLAG_RECURSIVE);
        // Padding is in canvas units. Scale it into the same raw-window units
        // as the transformed scrollbar area before mapping the drag.
        const int localH = (int)lv_obj_get_height(obj);
        const int rawH = (int)objCoords.y2 - (int)objCoords.y1 + 1;
        const float sy = (localH > 0) ? (float)rawH / (float)localH : 1.0f;
        const int scaledTopPad = (int)std::lround((float)top_space * sy);
        const int scaledBottomPad = (int)std::lround((float)bot_space * sy);
        const int trackTop = (int)objCoords.y1 + scaledTopPad;
        const int rem = rawH - scaledTopPad - scaledBottomPad - thumbLen;
        if (rem <= 0) return;

        const int stNow = lv_obj_get_scroll_top(obj);
        const int range = stNow + lv_obj_get_scroll_bottom(obj);   // scroll_h
        if (range <= 0) return;

        int newThumbTop = (int)p.y - (int)st.grabOffset;
        if (newThumbTop < trackTop) newThumbTop = trackTop;
        if (newThumbTop > trackTop + rem) newThumbTop = trackTop + rem;

        const int wantFromTop = (int)((long)(newThumbTop - trackTop) * range / rem);
        lv_obj_scroll_by(obj, 0, stNow - wantFromTop, LV_ANIM_OFF);
        return;
    }

    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        st.active = false;
        return;
    }
}
} // namespace

void UIWidgets::attachScrollbarDrag(lv_obj_t* scrollable) {
    if (!scrollable) return;
    // RESERVE THE BAR'S STRIP. Rows are commonly LV_PCT(100) children, so a bare
    // container lets them extend UNDER the bar - and a press on the bar then also
    // activates the row beneath it, because the row is the event's TARGET and has
    // already run its own handlers by the time this object's handler runs (so
    // lv_event_stop_bubbling here is too late to stop it). lv_list is safe
    // because its theme padding insets the rows; a plain lv_obj is not. Insetting
    // the content by the bar's thickness removes the overlap structurally, so no
    // row handler anywhere has to know about scrollbars.
    // The bar is NOT flush to the object's edge: lv_obj_scroll.c places it at
    //   x2 - pad_right(SCROLLBAR) - thickness + 1 .. x2 - pad_right(SCROLLBAR)
    // so the reservation must clear BOTH the scrollbar part's own right padding
    // and its thickness. Reserving only `thickness` still left the row's last
    // pixel under the bar (measured: row ended at 432, bar started at 430).
    const lv_coord_t thickness = lv_obj_get_style_width(scrollable, LV_PART_SCROLLBAR);
    const lv_coord_t barInset = lv_obj_get_style_pad_right(scrollable, LV_PART_SCROLLBAR);
    const lv_coord_t need = thickness + barInset;
    if (thickness > 0 && lv_obj_get_style_pad_right(scrollable, LV_PART_MAIN) < need) {
        lv_obj_set_style_pad_right(scrollable, need, 0);
    }
    // A press on the bar lands on whatever is at that point - often a ROW, since
    // rows are usually LV_PCT(100) children. LVGL only delivers a child's event
    // to a parent when the parent carries LV_OBJ_FLAG_EVENT_BUBBLE, so without
    // this the press never reaches us at all and the drag does nothing.
    lv_obj_add_flag(scrollable, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(scrollable, scrollbarDragCb, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(scrollable, scrollbarDragCb, LV_EVENT_PRESSING, nullptr);
    lv_obj_add_event_cb(scrollable, scrollbarDragCb, LV_EVENT_RELEASED, nullptr);
    lv_obj_add_event_cb(scrollable, scrollbarDragCb, LV_EVENT_PRESS_LOST, nullptr);
    lv_obj_add_event_cb(scrollable, scrollbarDragCb, LV_EVENT_DELETE, nullptr);
}

void UIWidgets::setFineDragActive(bool active) noexcept { gFineDragActive = active; }
bool UIWidgets::isFineDragActive() noexcept { return gFineDragActive; }

void UIWidgets::setArcValueInstant(lv_obj_t* arc, int value) {
    if (!arc) return;
    // Cancel any sweep already in flight, or it would keep dragging the knob
    // toward its stale target after we snap.
    lv_anim_delete(arc, arcValueAnimCb);
    lv_arc_set_value(arc, std::clamp(value, 0, 1000));
    syncSharedArcVisual(arc);
}

void UIWidgets::setArcValueAndSyncVisual(lv_obj_t* arc, int value) {
    if (!arc) return;
    const int target = std::clamp(value, 0, 1000);
    const int from = lv_arc_get_value(arc);
    // Never let two sweeps fight over one arc.
    lv_anim_delete(arc, arcValueAnimCb);
    if (from == target) {
        syncSharedArcVisual(arc);
        return;
    }
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, arcValueAnimCb);
    lv_anim_set_values(&a, from, target);
    lv_anim_set_duration(&a, 200);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

int UIWidgets::getArcVisualAngle(lv_obj_t* arc) {
    lv_obj_t* face = findSharedArcFace(arc);
    if (!face) return -1;
    return lv_obj_get_style_transform_angle(face, LV_PART_MAIN);
}

void UIWidgets::setDiscreteSnap(DiscreteSnapFn fn, void* userData) {
    gDiscreteSnap = fn;
    gDiscreteSnapUserData = userData;
}

UIWidgets::DiscreteSnapFn UIWidgets::getDiscreteSnap() noexcept {
    return gDiscreteSnap;
}

void* UIWidgets::getDiscreteSnapUserData() noexcept {
    return gDiscreteSnapUserData;
}

void UIWidgets::createArc(lv_obj_t* parent, int paramIndex, const char* name, 
                         AbstractCymbalsUI* ui, UIStyles& styles, 
                         lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                         lv_event_cb_t callback) {
    ArcVisualSpec spec = normalArcVisualSpec();
    createArcEx(parent, paramIndex, name, ui, styles, widgets, labels, fParams, callback, spec, true);
}

void UIWidgets::createArcEx(lv_obj_t* parent, int paramIndex, const char* name,
                            AbstractCymbalsUI* ui, UIStyles& styles,
                            lv_obj_t** widgets, lv_obj_t** labels, float* fParams,
                            lv_event_cb_t callback, const ArcVisualSpec& spec,
                            bool showValue) {
    allowChildOverflow(parent);
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, spec.containerW, spec.containerH);
    lv_obj_set_style_bg_opa(cont, 0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_style_pad_row(cont, scaled(2), 0);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    // No clipping: column needs overflow for row gaps.
    allowChildOverflow(cont);

    // Name above
    // DOT, not WRAP. A wrapped title made its cell taller than its siblings, so
    // that knob's VALUE sat on a lower line than the rest of the row - it reads
    // as the value having "dropped down" (reported on Galactic's Brightness and
    // GlitchShifter's Feedback). One line, ellipsised, keeps every cell equal.
    // An empty name means "no title line" (e.g. the DISPERSION X-LINK knob,
    // whose label sits beside the ring): do not create the label at all, or
    // its line height pushes the ring below the 56px strip.
    if (name != nullptr && name[0] != '\0') {
        lv_obj_t* nameLbl = createKnobTitleLabel(
            cont, name, styles, spec.containerW + spec.titleExtraW, LV_LABEL_LONG_DOT);
        // The face is chosen inside createKnobTitleLabel (adaptive: it steps
        // down only when the name would not fit), so nothing to override.
        lv_obj_set_style_margin_bottom(nameLbl, spec.labelMarginBottom, 0);
    }

    // Knob container sized to fit the mod ring (arcSize+28) so the teal
    // range line is never clipped at flex-row bounds. Previously knobCont was
    // arcSize and relied on OVERFLOW_VISIBLE + ext_draw_size, which flex
    // containers still clip at top/bottom (seen in screenshot).
    lv_obj_t* knobCont = lv_obj_create(cont);
    const lv_coord_t ringPad = spec.ringPad > 0 ? spec.ringPad : scaled(28);
    lv_obj_set_size(knobCont, spec.arcSize + ringPad, spec.arcSize + ringPad);
    lv_obj_set_style_bg_opa(knobCont, 0, 0);
    lv_obj_set_style_border_width(knobCont, 0, 0);
    lv_obj_set_style_pad_all(knobCont, 0, 0);
    lv_obj_clear_flag(knobCont, LV_OBJ_FLAG_SCROLLABLE);
    allowChildOverflow(knobCont);

    const lv_color_t* huePtr = spec.hueOverride ? &spec.arcHue : nullptr;
    lv_obj_t* arc = createSharedArcKnob(knobCont, spec.arcSize, styles,
                                        spec.arcSize < scaled(40)
                                            ? UIStyles::compactKnobGeometry()
                                            : UIStyles::normalKnobGeometry(),
                                        fParams[paramIndex],
                                        huePtr,
                                        true /* withModRing */);
    attachArcDrag(arc, ui);

    if (spec.hueOverride) {
        // Functional color coding: value arc takes the section hue (Pigments
        // style). Indicator + its glow shadow follow.
        lv_obj_set_style_arc_color(arc, spec.arcHue, LV_PART_INDICATOR);
        lv_obj_set_style_shadow_color(arc, spec.arcHue, LV_PART_INDICATOR);
    }

    if (showValue) {
        // Value below arc — compactValueLabel = the distinct bright value tier
        lv_obj_t* valLbl = lv_label_create(cont);
        lv_obj_add_style(valLbl, &styles.compactValueLabel, 0);
        lv_obj_set_style_margin_top(valLbl, spec.valueMarginTop, 0);
        lv_obj_set_width(valLbl, spec.containerW);
        lv_obj_set_style_text_align(valLbl, LV_TEXT_ALIGN_CENTER, 0);
        // Presence tier: main knobs carry 14px bright values; small knobs stay 12.
        if (spec.arcSize >= scaled(50))
            lv_obj_set_style_text_font(valLbl, getScaledFont(), 0);
        // Rail-prominence (dafxpaper dailies): the plugin's 44px rail arcs
        // read as small spec (36) — promote their values to the bright tier so
        // the readout carries weight the 8px bigger face doesn't.
        else if (spec.arcSize >= scaled(44))
            lv_obj_set_style_text_color(valLbl, COL_TEXT, 0);
        // Tiniest faces carry a tiny value line too: two-row FX cells only
        // have ~14px for the readout, and a 12px label there steals the
        // knob's own height.
        if (spec.arcSize < scaled(30))
            lv_obj_set_style_text_font(valLbl, getScaledTinyFont(), 0);
        lv_label_set_long_mode(valLbl, LV_LABEL_LONG_CLIP);
        char buf[32];
        snprintf(buf, sizeof(buf), "%.3f", fParams[paramIndex]);
        lv_label_set_text(valLbl, buf);
        labels[paramIndex] = valLbl;
    } else {
        labels[paramIndex] = nullptr;
    }

    lv_obj_set_user_data(arc, reinterpret_cast<void*>((intptr_t)paramIndex));
    lv_obj_add_event_cb(arc, callback, LV_EVENT_VALUE_CHANGED, ui);

    widgets[paramIndex] = arc;
}

lv_obj_t* UIWidgets::createKnobTitleLabel(lv_obj_t* parent, const char* title,
                                          UIStyles& styles, lv_coord_t width,
                                          lv_label_long_mode_t longMode) {
    lv_obj_t* label = lv_label_create(parent);
    lv_obj_add_style(label, &styles.knobTitleSmall, 0);
    lv_obj_set_width(label, width);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    // Honour the caller's mode. This used to INVERT it (CLIP -> WRAP) on the
    // reasoning that no knob name should be cut off — but a wrapped title is a
    // two-line title, and in a fixed knob row that pushes its knob DOWN and
    // breaks the line of knobs. A clipped/ellipsised single line keeps every
    // row aligned, which is the stronger constraint.
    lv_label_set_long_mode(label, longMode);
    lv_label_set_text(label, title);
    // Pin the height to ONE line. DOTS alone is not enough: with an auto height
    // LVGL grows the label to fit the wrapped text, so "EarlyRF" still rendered
    // as "EarlyR" + "F" on a second line - which made that knob's cell taller
    // and dropped its value below the rest of the row. A one-line height is what
    // actually forces the ellipsis.
    const lv_font_t* font = lv_obj_get_style_text_font(label, LV_PART_MAIN);
    if (font) lv_obj_set_height(label, lv_font_get_line_height(font));

    // Record the title's NATURAL width, measured before LVGL gets a chance to
    // rewrite the buffer. DOTS/CLIP modes REPLACE the label's text with the
    // truncated form ("EncAmnt" becomes "Enc..."), so anything downstream that
    // asks lv_label_get_text to decide whether the text fits measures the
    // already-shortened string and always concludes it fits. Stored as an
    // integer in user_data - no allocation, and the UI audit reads it to turn a
    // silently ellipsised knob title into a failure.
    if (font) {
        lv_coord_t ls = lv_obj_get_style_text_letter_space(label, LV_PART_MAIN);
        lv_point_t natural{0, 0};
        lv_text_get_size(&natural, title, font, ls, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

        // Step DOWN a face rather than ellipsise. An ellipsised title is worse
        // than a slightly smaller one: "MACRO 1".."MACRO 4" all rendered as
        // "MACR..." in a 56px cell, so the ellipsis removed the ONLY character
        // distinguishing the four knobs. Adaptive on purpose - it changes nothing
        // where the name already fits, so no label shrinks for no reason.
        if (natural.x > width) {
            const lv_font_t* smaller = getScaledTinyFont();
            if (smaller && smaller != font) {
                lv_obj_set_style_text_font(label, smaller, 0);
                lv_obj_set_style_text_letter_space(label, 0, 0);
                font = smaller;
                lv_text_get_size(&natural, title, font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
            }
        }
        lv_obj_set_user_data(label, (void*)(intptr_t)natural.x);
    }

    return label;
}

lv_obj_t* UIWidgets::createCompactControlColumn(lv_obj_t* parent, lv_coord_t width,
                                                lv_coord_t height, lv_coord_t rowPad) {
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, width, height);
    lv_obj_set_style_bg_opa(cont, 0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_style_pad_row(cont, rowPad, 0);
    lv_obj_set_layout(cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    return cont;
}

lv_obj_t* UIWidgets::createCompactDropdown(lv_obj_t* parent, UIStyles& styles,
                                           lv_coord_t width, lv_coord_t height) {
    if (height <= 0) height = LV_SIZE_CONTENT; // guard: 0 height collapses the control
    lv_obj_t* dropdown = lv_dropdown_create(parent);
    lv_obj_set_size(dropdown, width, height);
    lv_obj_set_scrollbar_mode(dropdown, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(dropdown, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(dropdown, &styles.compactSelectMain, LV_PART_MAIN);
    lv_obj_add_style(dropdown, &styles.compactSelectIndicator, LV_PART_INDICATOR);

    lv_obj_t* list = lv_dropdown_get_list(dropdown);
    if (list != nullptr) {
        lv_obj_add_style(list, &styles.compactSelectListMain, LV_PART_MAIN);
        lv_obj_add_style(list, &styles.compactSelectListSelected, LV_PART_SELECTED);
    }

    return dropdown;
}

lv_obj_t* UIWidgets::createCompactLabeledDropdown(lv_obj_t* parent, int paramIndex,
                                                  const char* name, const char* options,
                                                  AbstractCymbalsUI* ui, UIStyles& styles,
                                                  lv_obj_t** widgets, float* fParams,
                                                  lv_event_cb_t callback,
                                                  lv_coord_t width,
                                                  lv_coord_t dropdownH,
                                                  lv_coord_t rowPad) {
    lv_obj_t* cont = createCompactControlColumn(parent, width, LV_SIZE_CONTENT, rowPad);
    createKnobTitleLabel(cont, name, styles, width);

    lv_obj_t* dropdown = createCompactDropdown(cont, styles, width, dropdownH);
    lv_dropdown_set_options(dropdown, options);
    lv_dropdown_set_selected(dropdown, (int)fParams[paramIndex]);
    lv_obj_set_user_data(dropdown, reinterpret_cast<void*>((intptr_t)paramIndex));
    lv_obj_add_event_cb(dropdown, callback, LV_EVENT_VALUE_CHANGED, ui);

    if (widgets) widgets[paramIndex] = dropdown;
    return dropdown;
}

lv_obj_t* UIWidgets::createCompactValueLabel(lv_obj_t* parent, UIStyles& styles,
                                             lv_coord_t width, lv_coord_t marginTop) {
    lv_obj_t* label = lv_label_create(parent);
    lv_obj_add_style(label, &styles.compactValueLabel, 0);
    lv_obj_set_width(label, width);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_margin_top(label, marginTop, 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    return label;
}

lv_obj_t* UIWidgets::createSimpleArcWithDrag(lv_obj_t* parent, lv_coord_t arcSize,
                                            float initialValue, UIStyles& styles,
                                            lv_event_cb_t dragCallback,
                                            void* dragUserData) {
    // The mod band ring extends past the knob container; let it paint into the
    // surrounding gap instead of being clipped at the caller's container bounds
    // (EQ-editor and comb-dock arc columns pass their per-arc containers here).
    allowChildOverflow(parent);
    lv_obj_t* knobCont = lv_obj_create(parent);
    lv_obj_set_size(knobCont, arcSize, arcSize);
    lv_obj_set_style_bg_opa(knobCont, 0, 0);
    lv_obj_set_style_border_width(knobCont, 0, 0);
    lv_obj_set_style_pad_all(knobCont, 0, 0);
    lv_obj_clear_flag(knobCont, LV_OBJ_FLAG_SCROLLABLE);
    allowChildOverflow(knobCont);
    // Geometry by arc size, exactly like createArcEx (UIWidgets.cpp:517-519).
    // This used to hardcode compactKnobGeometry(), so a 44px caller got
    // capInset 6 (cap = 38) where createArcEx's rail reference at the same 44px
    // gets capInset 12 (cap = 32). The oversized cap left only ~3px clearance
    // to the 44px value-arc radius, so the value arc hugged the cap and the
    // knob read as a dark blob instead of matching the rail. Callers below
    // scaled(40) (the 38px EQ card knobs) are unaffected — still compact.
    lv_obj_t* arc = createSharedArcKnob(knobCont, arcSize, styles,
                                        arcSize < scaled(40)
                                            ? UIStyles::compactKnobGeometry()
                                            : UIStyles::normalKnobGeometry(),
                                        initialValue,
                                        nullptr /* indicatorOverride */,
                                        true /* withModRing */);

    if (dragCallback) {
        lv_obj_add_event_cb(arc, dragCallback, LV_EVENT_ALL, dragUserData);
    }

    return arc;
}

lv_obj_t* UIWidgets::createSpectrumAnalyzer(lv_obj_t* parent, int32_t w, int32_t h, UIStyles& /*styles*/, lv_obj_t** barObjects, lv_obj_t** peakDotOut) {
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_size(container, w, h);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(container, COL_EQ_CANVAS_BG, 0);
    lv_obj_set_style_border_width(container, 1, 0);
    lv_obj_set_style_border_color(container, COL_ACCENT, 0);
    lv_obj_set_style_radius(container, scaled(8), 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_layout(container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
    lv_obj_set_style_pad_all(container, scaled(3), 0);
    lv_obj_set_style_pad_gap(container, scaled(1), 0);

    // Resting display furniture as REAL child objects (created before the
    // bars, so the live bars render on top): dB grid rows, a baseline glow
    // and bottom frequency tick labels — the strip reads as an instrument
    // display at silence, not an empty dashed box (critic round 6).
    auto furniture = [&](lv_coord_t yPct, lv_coord_t h, lv_color_t color, lv_opa_t opa) {
        lv_obj_t* line = lv_obj_create(container);
        lv_obj_set_size(line, lv_pct(100), h);
        lv_obj_add_flag(line, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_set_style_bg_color(line, color, 0);
        lv_obj_set_style_bg_opa(line, opa, 0);
        lv_obj_set_style_border_width(line, 0, 0);
        lv_obj_set_style_radius(line, 0, 0);
        lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_align(line, LV_ALIGN_TOP_MID, 0, yPct);
        return line;
    };
    furniture(scaled(25), 1, COL_EQ_GRID, 230);
    furniture(scaled(50), 1, COL_EQ_GRID, 230);
    furniture(scaled(75), 1, COL_EQ_GRID, 230);
    lv_obj_t* baseline = furniture(lv_pct(100), 1, COL_HIGHLIGHT, 102);  // baseline glow
    // Idle pulse: the baseline breathes (opa 0.4..0.7) so silence reads alive.
    lv_anim_t pulse;
    lv_anim_init(&pulse);
    lv_anim_set_var(&pulse, baseline);
    lv_anim_set_exec_cb(&pulse, [](void* var, int32_t v) {
        lv_obj_set_style_bg_opa((lv_obj_t*)var, (lv_opa_t)v, 0);
    });
    lv_anim_set_values(&pulse, 102, 178);
    lv_anim_set_time(&pulse, 1600);
    lv_anim_set_playback_time(&pulse, 1600);
    lv_anim_set_repeat_count(&pulse, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&pulse);
    {
        static const char* kTickTxt[3] = {"100", "1k", "10k"};
        for (int t = 0; t < 3; ++t) {
            lv_obj_t* tick = lv_label_create(container);
            lv_label_set_text(tick, kTickTxt[t]);
            lv_obj_add_flag(tick, LV_OBJ_FLAG_IGNORE_LAYOUT);
            lv_obj_set_style_text_font(tick, getScaledMicroFont(), 0);
            lv_obj_set_style_text_color(tick, COL_EQ_FREQ_LABEL, 0);
            lv_obj_set_style_text_opa(tick, LV_OPA_80, 0);
            lv_obj_align(tick, LV_ALIGN_BOTTOM_MID, (lv_coord_t)((t - 1) * scaled(120)), scaled(-2));
        }
    }

    if (barObjects != nullptr) {
        const int numBands = FFTAnalyzer::kNumBands;
        for (int i = 0; i < numBands; ++i) {
            lv_obj_t* bar = lv_bar_create(container);
            lv_obj_set_size(bar, 0, lv_pct(100));
            lv_obj_set_flex_grow(bar, 1);
            lv_bar_set_range(bar, 0, 1000);
            lv_bar_set_value(bar, 0, LV_ANIM_OFF);

            lv_obj_set_style_radius(bar, scaled(2), LV_PART_MAIN);
            // Idle bands read as a subtle column grid (vs. invisible on the
            // canvas bg) so the analyzer looks instrumented even in silence.
            lv_obj_set_style_bg_color(bar, COL_EQ_GRID, LV_PART_MAIN);
            lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN);

            lv_obj_set_style_radius(bar, scaled(2), LV_PART_INDICATOR);
            lv_obj_set_style_bg_color(bar, COL_HIGHLIGHT, LV_PART_INDICATOR);
            lv_obj_set_style_bg_grad_color(bar, COL_EQ_LOW, LV_PART_INDICATOR);
            lv_obj_set_style_bg_grad_dir(bar, LV_GRAD_DIR_VER, LV_PART_INDICATOR);
            lv_obj_set_style_bg_grad_stop(bar, 200, LV_PART_INDICATOR);

            barObjects[i] = bar;
        }
    }

    // Caption chip (top-left) so the silent state has a label, not a void.
    lv_obj_t* caption = lv_label_create(container);
    lv_label_set_text(caption, "SPECTRUM");
    lv_obj_add_flag(caption, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_style_text_font(caption, getScaledMicroFont(), 0);
    lv_obj_set_style_text_color(caption, COL_TEXT_DIM, 0);
    lv_obj_set_style_text_opa(caption, LV_OPA_70, 0);
    lv_obj_set_pos(caption, scaled(6), scaled(2));

    // Live output LED (top-right): glows amber while audio flows through the
    // analyzer; sits dim as a dark ember in silence. Driven by the UI timer.
    lv_obj_t* peakDot = lv_obj_create(container);
    lv_obj_set_size(peakDot, scaled(6), scaled(6));
    lv_obj_add_flag(peakDot, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_style_radius(peakDot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(peakDot, COL_ACCENT, 0);
    lv_obj_set_style_bg_opa(peakDot, LV_OPA_60, 0);
    lv_obj_set_style_border_width(peakDot, 0, 0);
    lv_obj_align(peakDot, LV_ALIGN_TOP_RIGHT, scaled(-6), scaled(4));
    lv_obj_clear_flag(peakDot, LV_OBJ_FLAG_SCROLLABLE);
    if (peakDotOut) *peakDotOut = peakDot;


    return container;
}

lv_obj_t* UIWidgets::makeRow(lv_obj_t* parent) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, 0, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    return row;
}

lv_obj_t* UIWidgets::createScreenStrip(lv_obj_t* parent, UIStyles& styles, const char* title) {
    lv_obj_t* strip = lv_obj_create(parent);
    lv_obj_add_style(strip, &styles.panel, 0);
    // One shared header height for every stage (COLOUR is the reference).
    // DISPERSION's X-Link knob needs 60px content + pad/border; 72 fits all.
    lv_obj_set_size(strip, lv_pct(100), scaled(56));
    lv_obj_set_style_min_height(strip, scaled(56), 0);
    lv_obj_set_style_radius(strip, scaled(6), 0);
    lv_obj_set_style_pad_hor(strip, scaled(10), 0);
    lv_obj_set_style_pad_ver(strip, scaled(4), 0);
    lv_obj_set_style_pad_column(strip, scaled(8), 0);
    lv_obj_set_style_bg_opa(strip, LV_OPA_COVER, 0);
    lv_obj_set_layout(strip, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(strip, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(strip, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(strip, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* titleLbl = lv_label_create(strip);
    lv_label_set_text(titleLbl, title);
    lv_obj_add_style(titleLbl, &styles.title, 0);
    lv_obj_set_user_data(titleLbl, (void*)(intptr_t)-1);

    lv_obj_t* spacer = lv_obj_create(strip);
    lv_obj_set_size(spacer, 0, 1);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_style_bg_opa(spacer, 0, 0);
    lv_obj_set_style_border_width(spacer, 0, 0);
    lv_obj_set_user_data(spacer, (void*)(intptr_t)-1);
    lv_obj_clear_flag(spacer, LV_OBJ_FLAG_SCROLLABLE);

    return strip;
}

END_NAMESPACE_DISTRHO
