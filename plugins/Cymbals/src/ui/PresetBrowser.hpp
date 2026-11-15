// Preset browser panel: u-he REPRO-style directory browser with search,
// multi-coloured favourites, tag picker, and drag'n'drop folder copy.
// Header-only LVGL UI; hosts implement PresetBrowserHost (CymbalsUI does).
// Pure search/favourites/copy logic lives in PresetBrowserLogic.hpp.
#ifndef PRESET_BROWSER_HPP
#define PRESET_BROWSER_HPP

#include "UICommon.hpp"
#include "UIStyles.hpp"
#include "UIWidgets.hpp"
#include "PresetBrowserLogic.hpp"
#include "../PluginCymbals.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

START_NAMESPACE_DISTRHO

using namespace PresetBrowserLogic;

// --- preset-row glyphs -------------------------------------------------------
// Round-3 row enrichment: a favourite STAR plus bank/author/tag metadata on
// every preset card. No shipped font carries a star glyph (montserrat stops at
// U+00FF, the kit ships no symbol font and LV_USE_VECTOR_GRAPHIC is 0 here), so
// the star is rasterised once per colour into a shared ARGB8888 canvas raster
// that every row references. Rasters are process-lifetime statics: LVGL only
// frees draw bufs flagged LV_IMAGE_FLAGS_ALLOCATED (heap-decoded images), so the
// image cache never releases them, and the pixels are written once per colour.
namespace PresetRowGlyphs {

inline constexpr int kStarPx = 14;     // base star side before scaled()
inline constexpr int kStarSlots = 7;   // 6 favourite colours + "off"
// Row pitch in pixels from LVGL's own draw-buf stride rule (rounds the row
// bytes up to LV_DRAW_BUF_STRIDE_ALIGN), so the raster stays valid if that
// alignment ever changes.
inline constexpr int kStarRowPx =
    (int)((LV_DRAW_BUF_STRIDE(kStarPx, LV_COLOR_FORMAT_ARGB8888) + 3) / 4);

// Slots 0..5 mirror kFavColours order; slot 6 is the not-favourited star.
inline lv_color32_t starSlotColour(int slot) {
    static const lv_color_t kCols[kStarSlots] = {
        COL_FAV_RED, COL_FAV_ORANGE, COL_FAV_YELLOW,
        COL_FAV_GREEN, COL_FAV_BLUE, COL_FAV_PURPLE,
        COL_ACCENT,
    };
    if (slot < 0 || slot >= kStarSlots) slot = kStarSlots - 1;
    const lv_opa_t opa = (slot < kStarSlots - 1) ? LV_OPA_COVER : LV_OPA_50;
    return lv_color_to_32(kCols[slot], opa);
}

struct StarRaster {
    // Pixel array first and 64-byte aligned: lv_canvas_set_buffer logs (once
    // per row) when the data is below LV_DRAW_BUF_ALIGN, and the draw unit may
    // want more. `ready` trails the pixels so it never shifts the alignment.
    alignas(64) lv_color32_t px[kStarRowPx * kStarPx];
    bool ready = false;
};

// Fill the 5-point star into the slot's raster on first use, then hand out the
// shared pixels (stride = 4 bytes/px, one ARGB8888 pixel per entry). Scanline
// polygon fill: 10 vertices alternating outer (6.4) and inner (2.7) radii at
// 36-degree steps, centred on 6.5 in the 14px box.
inline const void* starRasterPixels(int slot) {
    static StarRaster rasters[kStarSlots];
    if (slot < 0 || slot >= kStarSlots) slot = kStarSlots - 1;
    StarRaster& r = rasters[slot];
    if (!r.ready) {
        std::memset(r.px, 0, sizeof(r.px));
        const lv_color32_t c = starSlotColour(slot);
        float xs[10], ys[10];
        for (int i = 0; i < 10; ++i) {
            const float ang = -1.57079637f + (float)i * 0.62831853f;
            const float rad = (i % 2 == 0) ? 6.4f : 2.7f;
            xs[i] = 6.5f + rad * std::cos(ang);
            ys[i] = 6.5f + rad * std::sin(ang);
        }
        for (int y = 0; y < kStarPx; ++y) {
            const float yy = (float)y + 0.5f;
            float cross[10];
            int n = 0;
            for (int e = 0; e < 10; ++e) {
                const int e2 = (e == 9) ? 0 : e + 1;
                const float y0 = ys[e], y1 = ys[e2];
                if ((y0 <= yy && y1 > yy) || (y1 <= yy && y0 > yy)) {
                    const float t = (yy - y0) / (y1 - y0);
                    cross[n++] = xs[e] + t * (xs[e2] - xs[e]);
                }
            }
            for (int a = 1; a < n; ++a) {
                const float v = cross[a];
                int b = a - 1;
                while (b >= 0 && cross[b] > v) { cross[b + 1] = cross[b]; --b; }
                cross[b + 1] = v;
            }
            for (int p = 0; p + 1 < n; p += 2) {
                int x0 = (int)std::ceil(cross[p] - 0.5f);
                int x1 = (int)std::floor(cross[p + 1] - 0.5f);
                if (x0 < 0) x0 = 0;
                if (x1 > kStarPx - 1) x1 = kStarPx - 1;
                for (int x = x0; x <= x1; ++x) {
                    r.px[y * kStarRowPx + x] = c;
                }
            }
        }
        r.ready = true;
    }
    return r.px;
}

// The clickable star hit box (4px larger than the glyph, so the hover circle
// reads as a control affordance). The caller wires the CLICKED handler and
// passes the row index via user_data.
inline lv_obj_t* createRowStar(lv_obj_t* parent, int favSlot) {
    lv_obj_t* box = lv_obj_create(parent);
    lv_obj_set_size(box, scaled(kStarPx + 4), scaled(kStarPx + 4));
    lv_obj_set_style_pad_all(box, 0, 0);
    lv_obj_set_style_bg_opa(box, 0, 0);
    lv_obj_set_style_border_width(box, 0, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(box, LV_OPA_20, LV_STATE_HOVERED);
    lv_obj_set_style_bg_color(box, COL_HIGHLIGHT, LV_STATE_HOVERED);
    lv_obj_set_style_radius(box, LV_RADIUS_CIRCLE, LV_STATE_HOVERED);

    lv_obj_t* cv = lv_canvas_create(box);
    lv_canvas_set_buffer(cv, const_cast<void*>(starRasterPixels(favSlot)),
                         kStarPx, kStarPx, LV_COLOR_FORMAT_ARGB8888);
    lv_obj_set_size(cv, scaled(kStarPx), scaled(kStarPx));
    lv_obj_clear_flag(cv, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cv, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(cv);
    return box;
}

// Mini pill chip used for the bank (type) badge and tag pills: recessed fill,
// hairline border in the lane colour, micro tracked caps.
inline lv_obj_t* createRowPill(lv_obj_t* parent, const char* text, lv_color_t colour) {
    lv_obj_t* pill = lv_obj_create(parent);
    lv_obj_set_size(pill, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_hor(pill, scaled(6), 0);
    lv_obj_set_style_pad_ver(pill, 0, 0);
    lv_obj_set_style_bg_color(pill, COL_PANEL_DARK, 0);
    lv_obj_set_style_bg_opa(pill, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(pill, 1, 0);
    lv_obj_set_style_border_color(pill, colour, 0);
    lv_obj_set_style_radius(pill, LV_RADIUS_CIRCLE, 0);
    lv_obj_clear_flag(pill, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(pill, LV_SCROLLBAR_MODE_OFF);
    lv_obj_t* lbl = lv_label_create(pill);
    lv_label_set_text(lbl, text);
    // 12px, not the micro face: the bank/tag pills are the row's metadata
    // and were too small to read at a glance.
    lv_obj_set_style_text_font(lbl, getScaledSmallFont(), 0);
    lv_obj_set_style_text_color(lbl, colour, 0);
    lv_obj_set_style_text_letter_space(lbl, scaled(1), 0);
    lv_obj_center(lbl);
    return pill;
}

// Bank (type) label from the preset's folder relPath: "cymbals/crashes" ->
// "CRASHES", the preset-dir root -> "ROOT".
inline std::string bankLabelFor(const std::string& folder) {
    if (folder.empty()) return "ROOT";
    const size_t slash = folder.find_last_of('/');
    std::string leaf = (slash == std::string::npos) ? folder : folder.substr(slash + 1);
    if (leaf.empty()) return "ROOT";
    for (char& c : leaf) c = (char)std::toupper((unsigned char)c);
    return leaf;
}

// Functional lane colour per bank family (same palette philosophy as the EQ
// band / envelope lanes): engine-cyan for the cymbals family, the lane hues for
// the other skeleton roots, neutral for the root / unknown folders.
inline lv_color_t bankColourFor(const std::string& folder) {
    const size_t slash = folder.find('/');
    const std::string root = toLower(folder.substr(0, slash));
    if (root == "cymbals")    return COL_HIGHLIGHT;
    if (root == "basses" || root == "leads") return COL_ENV;
    if (root == "pads")       return COL_MACRO;
    if (root == "effects")    return COL_MOD;
    if (root == "percussion") return COL_TIME;
    return COL_TEXT_DIM;
}

// --- round-4: per-row type badge ---
// Lane colour per inferred kind. Deliberately a wider spread than the bank
// colours: the badge is the row's classification glyph and must stay legible
// as a colour before the 2-letter text is read.
inline lv_color_t kindColourFor(PresetBrowserLogic::PresetKind kind) {
    using PresetBrowserLogic::PresetKind;
    switch (kind) {
        case PresetKind::Cymbal:     return COL_HIGHLIGHT;    // engine cyan
        case PresetKind::Crash:      return COL_ERROR;        // crash lane
        case PresetKind::Ride:       return COL_MACRO;        // purple
        case PresetKind::Hat:        return COL_FAV_GREEN;
        case PresetKind::Sizzle:     return COL_TIME;
        case PresetKind::Splash:     return COL_FAV_ORANGE;
        case PresetKind::Bell:       return COL_FAV_YELLOW;
        case PresetKind::Orchestral: return COL_MOD;
        case PresetKind::Drum:       return COL_ENV;
        case PresetKind::Crumpet:    return COL_FAV_PURPLE;
        case PresetKind::Percussion: return COL_FAV_BLUE;
        case PresetKind::Generic:
        default:                     return COL_TEXT_DIM;
    }
}

// Fixed 26x16 badge so the type column never jitters between rows. Same
// recessed fill + hairline lane border as createRowPill, just sized.
inline lv_obj_t* createRowKindBadge(lv_obj_t* parent, PresetBrowserLogic::PresetKind kind) {
    const bool unclassified = (kind == PresetBrowserLogic::PresetKind::Generic);
    const lv_color_t lane = kindColourFor(kind);
    lv_obj_t* badge = lv_obj_create(parent);
    lv_obj_set_size(badge, scaled(26), scaled(16));
    lv_obj_set_style_pad_all(badge, 0, 0);
    lv_obj_set_style_bg_color(badge, COL_PANEL_DARK, 0);
    // Generic rows show nothing: "GE" read as an unexplained code. The slot
    // stays (fixed-width column) but the glyph/border are transparent; the
    // tooltip still explains when a row could not be classified.
    lv_obj_set_style_bg_opa(badge, unclassified ? LV_OPA_TRANSP : LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(badge, unclassified ? 0 : 1, 0);
    lv_obj_set_style_border_color(badge, lane, 0);
    lv_obj_set_style_radius(badge, scaled(3), 0);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(badge, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* lbl = lv_label_create(badge);
    lv_label_set_text(lbl, unclassified ? "" : PresetBrowserLogic::kindTag(kind));
    lv_obj_set_style_text_font(lbl, uiMicroFont(), 0);
    lv_obj_set_style_text_color(lbl, lane, 0);
    lv_obj_set_style_text_letter_space(lbl, scaled(1), 0);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(lbl, lv_pct(100));
    lv_obj_center(lbl);
    uiAttachTooltip(badge, unclassified
        ? "Unclassified: no instrument word in the preset name or folder"
        : PresetBrowserLogic::kindLabel(kind));
    return badge;
}

// --- round-4: classification-source chip ---
// "Search-engine" chip that says where the row's classification came from:
// the folder root (ROOT), a favourite slot (FAV), or the preset's own name
// overriding its bank (OVR). Sits beside the bank pill on the name line.
// nullptr = draw nothing: when neither the name nor the bank classified the
// preset, the bank pill already reads "ROOT" and a second ROOT chip is pure
// noise (that is the root-folder "Untitled Preset" case the capture caught).
inline const char* sourceChipText(PresetBrowserLogic::PresetKind inferred,
                                  PresetBrowserLogic::PresetKind fromBank,
                                  bool isFavourite) {
    using PresetBrowserLogic::PresetKind;
    if (inferred == PresetKind::Generic && fromBank == PresetKind::Generic)
        return isFavourite ? "FAV" : nullptr;
    if (inferred != PresetKind::Generic && fromBank != PresetKind::Generic &&
        inferred != fromBank)
        return "OVR";
    if (isFavourite) return "FAV";
    return "ROOT";
}

inline lv_color_t sourceChipColour(const char* text) {
    if (text && std::strcmp(text, "OVR") == 0) return COL_ENV;
    if (text && std::strcmp(text, "FAV") == 0) return COL_FAV_YELLOW;
    return COL_TEXT_DIM;
}

// 16px mini chip for the library mini-card: recessed fill, hairline lane
// border, micro tracked caps. Same language as createRowPill, shorter.
inline lv_obj_t* createMiniChip(lv_obj_t* parent, const char* text, lv_color_t colour) {
    lv_obj_t* chip = lv_obj_create(parent);
    lv_obj_set_size(chip, LV_SIZE_CONTENT, scaled(16));
    lv_obj_set_style_pad_hor(chip, scaled(6), 0);
    lv_obj_set_style_pad_ver(chip, 0, 0);
    lv_obj_set_style_bg_color(chip, COL_PANEL_DARK, 0);
    lv_obj_set_style_bg_opa(chip, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(chip, 1, 0);
    lv_obj_set_style_border_color(chip, colour, 0);
    lv_obj_set_style_radius(chip, LV_RADIUS_CIRCLE, 0);
    lv_obj_clear_flag(chip, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(chip, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(chip, LV_SCROLLBAR_MODE_OFF);
    lv_obj_t* lbl = lv_label_create(chip);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, uiMicroFont(), 0);
    lv_obj_set_style_text_color(lbl, colour, 0);
    lv_obj_set_style_text_letter_space(lbl, scaled(1), 0);
    lv_obj_center(lbl);
    return chip;
}

// --- round-4: tonal-profile thumbnail ---
// A small reference spectral slope (dB against log-frequency) rasterised into
// a shared ARGB8888 canvas. The per-row thumbnail uses this metadata estimate
// (parsing every preset's full DSP state for a thumbnail is not affordable);
// the PRESET INFO plot upgrades to the LOADED preset's real IR spectrum when
// the host provides one, so the panel matches what EXPORT IR produces.
inline constexpr int kCurveW = 38;  // base pixels before scaled()
inline constexpr int kCurveH = 22;
inline constexpr int kCurveRowPx =
    (int)((LV_DRAW_BUF_STRIDE(kCurveW, LV_COLOR_FORMAT_ARGB8888) + 3) / 4);
inline constexpr int kPlotW = 270;  // PRESET INFO plot
inline constexpr int kPlotH = 72;
inline constexpr int kPlotRowPx =
    (int)((LV_DRAW_BUF_STRIDE(kPlotW, LV_COLOR_FORMAT_ARGB8888) + 3) / 4);

// Log-frequency axis -> pixel x.
inline int curveXForHz(int w, float hz) {
    const double lo = std::log((double)PresetBrowserLogic::kProfileMinHz);
    const double hi = std::log((double)PresetBrowserLogic::kProfileMaxHz);
    const double t = (std::log((double)(hz > 0.0f ? hz : PresetBrowserLogic::kProfileMinHz)) - lo)
                   / (hi - lo);
    const double clamped = t < 0.0 ? 0.0 : (t > 1.0 ? 1.0 : t);
    return (int)std::lround(clamped * (double)(w - 1));
}

inline float hzForCurveX(int w, int x) {
    const double lo = std::log((double)PresetBrowserLogic::kProfileMinHz);
    const double hi = std::log((double)PresetBrowserLogic::kProfileMaxHz);
    const double t = (w > 1) ? (double)x / (double)(w - 1) : 0.0;
    return (float)std::exp(lo + t * (hi - lo));
}

// dB axis -> pixel y (14 dB headroom windows, 0 dB centred).
inline int curveYForDb(int h, float db) {
    const double t = ((double)db - PresetBrowserLogic::kProfileDbMin)
                   / (PresetBrowserLogic::kProfileDbMax - PresetBrowserLogic::kProfileDbMin);
    const double clamped = t < 0.0 ? 0.0 : (t > 1.0 ? 1.0 : t);
    return (int)std::lround((1.0 - clamped) * (double)(h - 1));
}

inline void blendPx(lv_color32_t* px, int rowPx, int x, int y,
                    lv_color32_t c, lv_opa_t opa) {
    lv_color32_t& d = px[y * rowPx + x];
    const uint32_t a = (uint32_t)opa;
    const uint32_t ia = 255u - a;
    d.red   = (uint8_t)(((uint32_t)c.red   * a + (uint32_t)d.red   * ia) / 255u);
    d.green = (uint8_t)(((uint32_t)c.green * a + (uint32_t)d.green * ia) / 255u);
    d.blue  = (uint8_t)(((uint32_t)c.blue  * a + (uint32_t)d.blue  * ia) / 255u);
    d.alpha = (uint8_t)(a > d.alpha ? a : d.alpha);
}

// Rasterise the profile. `frames` adds the 0 dB datum + 1 kHz decade grid the
// bigger info-panel plot shows; the row thumbnail stays a bare slope.
inline void drawProfileFrame(lv_color32_t* px, int w, int h, int rowPx, bool frames) {
    const lv_color32_t bg = lv_color_to_32(COL_PANEL_DARK, LV_OPA_COVER);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) px[y * rowPx + x] = bg;
    }
    if (!frames) return;
    const lv_color32_t grid = lv_color_to_32(COL_EQ_GRID, LV_OPA_COVER);
    const lv_color32_t zero = lv_color_to_32(COL_EQ_GRID_ZERO, LV_OPA_COVER);
    for (const float hz : {100.0f, 1000.0f, 10000.0f}) {
        const int gx = curveXForHz(w, hz);
        if (gx < 0 || gx >= w) continue;
        for (int y = 0; y < h; ++y) blendPx(px, rowPx, gx, y, grid, LV_OPA_10);
    }
    const int zy = curveYForDb(h, 0.0f);
    if (zy >= 0 && zy < h) {
        for (int x = 0; x < w; ++x) blendPx(px, rowPx, x, zy, zero, LV_OPA_40);
    }
}

inline void drawCurvePixels(lv_color32_t* px, int w, int h, int rowPx,
                            const std::vector<float>& db, lv_color_t colour) {
    if (db.size() < 2) return;
    const lv_color32_t fill = lv_color_to_32(colour, LV_OPA_COVER);
    const lv_color32_t line = lv_color_to_32(colour, LV_OPA_COVER);
    auto dbAtX = [&](int x) {
        const float t = (w > 1) ? (float)x / (float)(w - 1) : 0.0f;
        const float pos = t * (float)(db.size() - 1);
        const int i0 = std::clamp((int)pos, 0, (int)db.size() - 1);
        const int i1 = std::min(i0 + 1, (int)db.size() - 1);
        const float frac = pos - (float)i0;
        return db[(size_t)i0] * (1.0f - frac) + db[(size_t)i1] * frac;
    };
    int prevY = curveYForDb(h, dbAtX(0));
    for (int x = 0; x < w; ++x) {
        const float dbv = dbAtX(x);
        int y = curveYForDb(h, dbv);
        y = std::clamp(y, 0, h - 1);
        const int top = std::min(y, prevY);
        const int bottom = std::max(y, prevY);
        for (int yy = top; yy <= bottom && yy < h; ++yy) px[yy * rowPx + x] = line;
        for (int yy = y + 1; yy < h; ++yy) blendPx(px, rowPx, x, yy, fill, LV_OPA_20);
        prevY = y;
    }
}

inline void drawProfileRaster(lv_color32_t* px, int w, int h, int rowPx,
                              const PresetBrowserLogic::PresetProfile& p,
                              lv_color_t colour, bool frames) {
    drawProfileFrame(px, w, h, rowPx, frames);
    const lv_color32_t fill = lv_color_to_32(colour, LV_OPA_COVER);
    const lv_color32_t line = lv_color_to_32(colour, LV_OPA_COVER);
    int prevY = curveYForDb(h, PresetBrowserLogic::profileDbAt(
                                  p, hzForCurveX(w, 0)));
    for (int x = 0; x < w; ++x) {
        const float db = PresetBrowserLogic::profileDbAt(p, hzForCurveX(w, x));
        int y = curveYForDb(h, db);
        if (y < 0) y = 0;
        if (y > h - 1) y = h - 1;
        const int top = y < prevY ? y : prevY;
        const int bottom = y > prevY ? y : prevY;
        for (int yy = top; yy <= bottom && yy < h; ++yy) {
            px[yy * rowPx + x] = line;
        }
        for (int yy = y + 1; yy < h; ++yy) {
            blendPx(px, rowPx, x, yy, fill, LV_OPA_20);
        }
        prevY = y;
    }
}

inline void drawProfileCurveRaster(lv_color32_t* px, int w, int h, int rowPx,
                                   const std::vector<float>& db, lv_color_t colour,
                                   bool frames) {
    drawProfileFrame(px, w, h, rowPx, frames);
    drawCurvePixels(px, w, h, rowPx, db, colour);
}

// PRESET INFO plot raster: one panel exists at a time, so the pixels are a
// function-local static (same process-lifetime rule as the star rasters) and
// the canvas keeps a stable buffer pointer across redraws.
struct PlotRaster {
    alignas(64) lv_color32_t px[kPlotRowPx * kPlotH];
};

inline PlotRaster& sharedPlotRaster() {
    static PlotRaster r;
    return r;
}

inline void redrawProfileCanvas(lv_obj_t* cv,
                                const PresetBrowserLogic::PresetProfile& p,
                                lv_color_t colour) {
    if (!cv) return;
    PlotRaster& r = sharedPlotRaster();
    drawProfileRaster(r.px, kPlotW, kPlotH, kPlotRowPx, p, colour, true);
    lv_obj_invalidate(cv);
}

inline void redrawProfileCanvasCurve(lv_obj_t* cv,
                                     const std::vector<float>& db,
                                     lv_color_t colour) {
    if (!cv) return;
    PlotRaster& r = sharedPlotRaster();
    drawProfileCurveRaster(r.px, kPlotW, kPlotH, kPlotRowPx, db, colour, true);
    lv_obj_invalidate(cv);
}

// Per-row thumbnail: heap raster owned by the canvas (freed on LV_EVENT_DELETE).
inline lv_obj_t* createRowCurve(lv_obj_t* parent,
                                const PresetBrowserLogic::PresetProfile& p,
                                lv_color_t colour) {
    struct CurveBuf {
        alignas(64) lv_color32_t px[kCurveRowPx * kCurveH];
    };
    CurveBuf* buf = new CurveBuf();
    drawProfileRaster(buf->px, kCurveW, kCurveH, kCurveRowPx, p, colour, false);
    lv_obj_t* cv = lv_canvas_create(parent);
    lv_canvas_set_buffer(cv, buf->px, kCurveW, kCurveH, LV_COLOR_FORMAT_ARGB8888);
    lv_obj_set_size(cv, scaled(kCurveW), scaled(kCurveH));
    lv_obj_set_style_radius(cv, scaled(3), 0);
    // A hairline OUTLINE around a 38x22 raster read as an empty dashed box
    // rather than a display (round-4 critic: "a dashed, outline-only shape
    // ... looks like an unstyled or broken element"). A recessed dark well
    // with the curve drawn on it reads as a small scope.
    lv_obj_set_style_bg_color(cv, COL_PANEL_DARK, 0);
    lv_obj_set_style_bg_opa(cv, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cv, 0, 0);
    lv_obj_clear_flag(cv, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cv, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cv, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_user_data(cv, buf);
    lv_obj_add_event_cb(cv, [](lv_event_t* e) {
        CurveBuf* b = static_cast<CurveBuf*>(lv_event_get_user_data(e));
        delete b;
    }, LV_EVENT_DELETE, nullptr);
    return cv;
}

// Every lv_obj is CLICKABLE by default in this LVGL, so each row container,
// label and pill became the input target and swallowed the click before it
// reached the row card. Clear the flag on the whole row subtree except the
// favourite star (the one genuinely interactive child), making the full card
// click-to-load.
inline void makeRowClickThrough(lv_obj_t* row, lv_obj_t* keep) {
    if (row == nullptr) return;
    const uint32_t n = lv_obj_get_child_count(row);
    for (uint32_t i = 0; i < n; ++i) {
        lv_obj_t* c = lv_obj_get_child(row, i);
        if (c == nullptr || c == keep) continue;
        lv_obj_clear_flag(c, LV_OBJ_FLAG_CLICKABLE);
        makeRowClickThrough(c, keep);
    }
}

// Upgrade an existing row canvas from the metadata estimate to a real IR
// spectrum (same normalisation as the PRESET INFO plot).
inline void redrawRowCurve(lv_obj_t* cv, const std::vector<float>& db, lv_color_t colour) {
    if (!cv || !lv_obj_is_valid(cv) || db.size() < 2) return;
    void* buf = const_cast<void*>(lv_canvas_get_buf(cv));
    if (!buf) return;
    drawProfileCurveRaster(static_cast<lv_color32_t*>(buf), kCurveW, kCurveH, kCurveRowPx,
                           db, colour, false);
    lv_obj_invalidate(cv);
}

}  // namespace PresetRowGlyphs

// Implemented by CymbalsUI; keeps the panel decoupled from the plugin class.
class PresetBrowserHost {
public:
    virtual ~PresetBrowserHost() = default;
    virtual std::string presetDir() const = 0;
    virtual std::vector<PluginCymbals::PresetFileInfo> listPresetFiles() const = 0;
    virtual void requestPresetLoad(const std::string& path) = 0;   // setState("preset_load_path", path)
    virtual void setPresetMetadata(const std::string& name, const std::string& desc,
                                   const std::string& author, const std::string& tagsCsv,
                                   const std::string& usage) = 0;  // plugin->setPresetMetadata(...)
    virtual bool savePreset(const std::string& requestedPath, std::string& outError) = 0;
    virtual void requestIrExport(float seconds) = 0;               // setState("preset_export_ir_secs", ...)
    virtual std::string lastRequestedPresetPath() const = 0;       // stored by requestPresetLoad impl
    virtual void onPresetCommitted() = 0;                          // header name update + widget sync
    virtual void resetToInit() {}                                  // INIT button: defaults + clear identity
    // Optional: normalised log-magnitude curve of the LOADED preset's impulse
    // response (peak 0 dB, 48 Hz..15 kHz). When this returns true the info
    // panel draws the real IR spectrum instead of the metadata estimate.
    // Default: unavailable (headless mocks / hosts without a plugin pointer).
    virtual bool renderPresetIrProfile(std::vector<float>& outDb, int points) {
        (void)outDb; (void)points;
        return false;
    }
    // Row thumbnails: cached profile for a preset FILE (no rendering), and a
    // one-at-a-time loader used by the browser's thumbnail timer.
    virtual bool getCachedPresetProfile(const std::string& path, std::vector<float>& outDb) {
        (void)path; (void)outDb;
        return false;
    }
    virtual bool loadPresetProfileForFile(const std::string& path, std::vector<float>& outDb) {
        (void)path; (void)outDb;
        return false;
    }
};

class PresetBrowser {
public:
    ~PresetBrowser() {
        if (fThumbTimer) { lv_timer_del(fThumbTimer); fThumbTimer = nullptr; }
        if (fTagsPreviewTimer) { lv_timer_del(fTagsPreviewTimer); fTagsPreviewTimer = nullptr; }
    }
    // Builds the whole panel. widgets/labels/fParams/valueCb are optional and
    // only used for the math-mode toggle; the render harness passes nullptr.
    // `ui` must be the AbstractCymbalsUI* of the host (NOT the host itself):
    // valueChangedCallback recovers the UI from the event user_data via a
    // void*->AbstractCymbalsUI* cast, which cannot adjust subobject offsets.
    void attach(lv_obj_t* panel, PresetBrowserHost* host, UIStyles* styles,
                lv_obj_t** widgets = nullptr, lv_obj_t** labels = nullptr,
                float* params = nullptr, lv_event_cb_t valueCb = nullptr,
                AbstractCymbalsUI* ui = nullptr) {
        fPanel = panel;
        fHost = host;
        fStyles = styles;
        fWidgets = widgets;
        fLabels = labels;
        fParams = params;  // was `fParams = fParams` — param shadowed the member
        fValueCb = valueCb;
        fUi = ui;
        fAttached = true;

        lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_all(panel, scaled(10), 0);
        lv_obj_set_style_pad_row(panel, scaled(10), 0);

        lv_obj_t* presetStrip = UIWidgets::createScreenStrip(panel, *fStyles, "PRESETS");
        {
            lv_obj_t* initBtn = lv_btn_create(presetStrip);
            lv_obj_set_size(initBtn, scaled(48), scaled(26));
            fStyles->applyMomentaryButton(initBtn);
            // Immediately after the "PRESETS" title, not at the far edge.
            lv_obj_move_to_index(initBtn, 1);
            lv_obj_add_event_cb(initBtn, [](lv_event_t* e) {
                PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
                if (b != nullptr && b->fHost != nullptr) b->fHost->resetToInit();
            }, LV_EVENT_CLICKED, this);
            uiAttachTooltip(initBtn, "Initialise every parameter to its default");
            lv_obj_t* l = lv_label_create(initBtn);
            lv_label_set_text(l, "INIT");
            lv_obj_set_style_text_font(l, getScaledMicroFont(), 0);
            lv_obj_center(l);
        }

        lv_obj_t* body = lv_obj_create(panel);
        lv_obj_set_size(body, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(body, 1);
        lv_obj_set_style_bg_opa(body, 0, 0);
        lv_obj_set_style_border_width(body, 0, 0);
        lv_obj_set_style_pad_column(body, scaled(8), 0);
        lv_obj_set_layout(body, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(body, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(body, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_clear_flag(body, LV_OBJ_FLAG_SCROLLABLE);

        buildLeftColumn(body);
        buildCenterColumn(body);
        buildRightColumn(body);

        // Drag timer must die with the panel.
        lv_obj_add_event_cb(panel, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (b) b->fDrag.active = false;
        }, LV_EVENT_DELETE, this);

        refresh(true);

        // Some hosts deliver the initial preset_loaded_* state before the
        // editor (and these fields) exist. onLoadedField cached it; publish
        // the cache once now that the textareas are built. After this the
        // panel is event-driven — there is deliberately no per-frame sync.
        syncInfoPanelFromPlugin();

        // Dev aid: render/verify the tag picker without a click. Deferred so
        // the panel has its final size (attach runs before the first layout).
        if (std::getenv("CYMBALS_TAGS_PREVIEW") != nullptr && fTagsPreviewTimer == nullptr) {
            fTagsPreviewTimer = lv_timer_create([](lv_timer_t* t) {
                auto* b = static_cast<PresetBrowser*>(lv_timer_get_user_data(t));
                if (b) b->fTagsPreviewTimer = nullptr;
                lv_timer_del(t);
                if (b) b->openTagPicker();
            }, 1, this);
        }

        // Upgrade row thumbnails from the metadata estimate to each preset's
        // real IR spectrum, one row at a time (the render costs ~25 ms, so it
        // must never block list construction).
        if (!fThumbTimer)
            fThumbTimer = lv_timer_create(thumbTimerCb, 320, this);
    }

    // Rescan the preset dir + favourites, rebuild tree/list/nav.
    void refresh(bool forceListRebuild = false) {
        if (!fAttached || !fHost) return;
        const std::string dir = fHost->presetDir();
        ensureSkeletonDirs(dir);

        if (forceListRebuild || fPresetDir != dir || fEntries.empty()) {
            fPresetDir = dir;
            rebuildEntries(fHost->listPresetFiles());
        }
        reloadFavouritesAndRebuild();
    }

    // -1 / +1 over the filtered list, wraps; -1 starts at the end.
    void navStep(int dir) {
        // A stale/empty nav list must not make the header arrows dead: the
        // list is built at attach, so presets added later (or a filter that
        // matched nothing) left nothing to walk. Rescan once, unfiltered.
        if (fNavPaths.empty() && fHost != nullptr) {
            fPresetDir = fHost->presetDir();
            rebuildEntries(fHost->listPresetFiles());
            fView = ViewState{};
            fQuery.clear();
            fTypeFilter = 0;
            rebuildList();
        }
        if (fNavPaths.empty()) {
            setStatus(fPresetDir.empty() ? "No preset directory"
                                         : "No presets found in " + fPresetDir, true);
            return;
        }
        const size_t n = fNavPaths.size();
        // Resolve a missing index from the host's last requested path BEFORE
        // stepping. Only onPresetStatus() normally sets fNavIndex, and it is a
        // no-op unless the plugin reports a successful load back to the UI. If
        // that echo never arrives the index stays -1, so every press resolved
        // to slot 0 and the arrows appeared dead ("doesn't go through the
        // presets"). Deriving it from what the host was last asked to load
        // makes the walk advance regardless of whether that echo lands.
        if (fNavIndex < 0) {
            const std::string last = fHost->lastRequestedPresetPath();
            for (size_t i = 0; !last.empty() && i < n; ++i) {
                if (fNavPaths[i] == last) { fNavIndex = (int)i; break; }
            }
        }
        if (dir > 0) {
            fNavIndex = (fNavIndex < 0) ? 0 : (int)(((size_t)fNavIndex + 1) % n);
        } else {
            fNavIndex = (fNavIndex < 0) ? (int)(n - 1) : (int)((fNavIndex + n - 1) % n);
        }
        fHost->requestPresetLoad(fNavPaths[(size_t)fNavIndex]);
    }

    void setStatus(const std::string& text, bool isError) {
        if (!fStatusLabel) return;
        lv_label_set_text(fStatusLabel, text.c_str());
        lv_obj_set_style_text_color(fStatusLabel, isError ? COL_ERROR : COL_TEXT_DIM, 0);
    }

    // Push `value` into an info field unless the user owns that field. A
    // programmatic write is flagged so its VALUE_CHANGED does not look like a
    // user edit.
    void applyInfoValue(lv_obj_t* area, const std::string& value, bool& edited) {
        if (!area || edited) return;
        if (std::string(lv_textarea_get_text(area)) == value) return;
        fApplyingInfo = true;
        lv_textarea_set_text(area, value.c_str());
        fApplyingInfo = false;
    }

    // Re-applies the last-known loaded metadata to the info fields the user
    // has NOT edited. Called from uiIdle every frame. Comparing against the
    // loaded value alone is not enough: while typing, the textarea differs
    // from it on every tick, so the field was rewritten before the next
    // keystroke landed and nothing could be typed.
    void syncInfoPanelFromPlugin() {
        if (!fAttached) return;
        applyInfoValue(fNameArea, fLoadedName, fNameEdited);
        applyInfoValue(fAuthorArea, fLoadedAuthor, fAuthorEdited);
        applyInfoValue(fDescArea, fLoadedDesc, fDescEdited);
        applyInfoValue(fUsageArea, fLoadedUsage, fUsageEdited);
    }

    // preset_status from stateChanged. On success re-targets the header nav.
    void onPresetStatus(const std::string& status, bool isError) {
        setStatus(status, isError);
        if (!isError) {
            fLastLoadedPath = fHost->lastRequestedPresetPath();
            fNavIndex = -1;
            for (size_t i = 0; i < fNavPaths.size(); ++i) {
                if (fNavPaths[i] == fLastLoadedPath) { fNavIndex = (int)i; break; }
            }
            syncFavouriteSwatches();
            fHost->onPresetCommitted();
        }
    }

    // preset_loaded_name/description/usage/author/tags from stateChanged.
    // A load owns the fields again: clear the edit flags so the freshly
    // loaded metadata replaces whatever the user had typed.
    void onLoadedField(const std::string& key, const std::string& value) {
        if (key == "preset_loaded_name") {
            fLoadedName = value; fNameEdited = false; applyInfoValue(fNameArea, value, fNameEdited);
            // A new preset arrived: re-render its own IR for the tonal profile.
            // Done once per load (the name field is the first metadata push).
            // Render once per loaded PATH, not once per metadata echo: the
            // host-mediated path can echo preset_loaded_name after the direct
            // UI push, and each render costs ~25 ms of detached DSP work.
            const std::string path = fHost ? fHost->lastRequestedPresetPath() : std::string();
            if (fIrProfileDb.empty() || path != fIrProfileForPath) {
                fIrProfileDb.clear();
                if (fHost) fHost->renderPresetIrProfile(fIrProfileDb, kIrProfilePoints);
                fIrProfileForPath = path;
            }
        }
        else if (key == "preset_loaded_description") { fLoadedDesc = value; fDescEdited = false; applyInfoValue(fDescArea, value, fDescEdited); }
        else if (key == "preset_loaded_usage") { fLoadedUsage = value; fUsageEdited = false; applyInfoValue(fUsageArea, value, fUsageEdited); }
        else if (key == "preset_loaded_author") { fLoadedAuthor = value; fAuthorEdited = false; applyInfoValue(fAuthorArea, value, fAuthorEdited); }
        else if (key == "preset_loaded_tags") { fLoadedTags = value; setTagsFromCsv(value); }
        // The thumbnail + character word derive from these fields, so keep the
        // panel in sync the moment any of them changes (fires once per preset
        // load and during the attach-time initial push).
        updateProfilePanel();
    }

    std::vector<std::string> navPaths() const { return fNavPaths; }

    // --- tonal-profile preview (round-4) ---
    // Re-render the loaded preset's reference slope + its character word.
    // Metadata-derived only: nothing about the preset JSON format or the
    // favourites sidecar changes.
    void updateProfilePanel() {
        if (!fAttached) return;
        // Read the LIVE fields, not just the last loaded metadata: the curve
        // must react while the user edits the name/description/tags, and a
        // committed tag chip is metadata the character words read.
        auto liveText = [](lv_obj_t* area, const std::string& fallback) {
            if (area == nullptr) return fallback;
            const char* t = lv_textarea_get_text(area);
            return std::string(t ? t : "");
        };
        if (fIrProfileDb.size() >= 2) {
            // The info panel's profile is the preset's OWN impulse response
            // spectrum, so it matches what EXPORT IR produces.
            PresetRowGlyphs::redrawProfileCanvasCurve(fProfileCanvas, fIrProfileDb, COL_HIGHLIGHT);
            if (fProfileCharLbl) {
                char word[40];
                PresetBrowserLogic::profileCharacterFromCurve(
                    fIrProfileDb,
                    PresetBrowserLogic::kProfileMinHz,
                    PresetBrowserLogic::kProfileMaxHz,
                    word, sizeof(word));
                if (std::string(lv_label_get_text(fProfileCharLbl)) != word)
                    lv_label_set_text(fProfileCharLbl, word);
            }
            return;
        }
        const PresetBrowserLogic::PresetProfile p =
            PresetBrowserLogic::profileFromMetadata(
                {liveText(fNameArea, fLoadedName),
                 liveText(fDescArea, fLoadedDesc),
                 liveText(fUsageArea, fLoadedUsage),
                 tagsCsv()});
        PresetRowGlyphs::redrawProfileCanvas(fProfileCanvas, p, COL_HIGHLIGHT);
        if (fProfileCharLbl) {
            char word[40];
            PresetBrowserLogic::profileCharacter(p, word, sizeof(word));
            if (std::string(lv_label_get_text(fProfileCharLbl)) != word)
                lv_label_set_text(fProfileCharLbl, word);
        }
    }

    // --- tag chips -------------------------------------------------------
    static std::string trimTag(std::string s) {
        size_t a = 0, b = s.size();
        while (a < b && std::isspace((unsigned char)s[a])) ++a;
        while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
        return s.substr(a, b - a);
    }
    static std::string foldTag(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) out.push_back((char)std::tolower((unsigned char)c));
        return out;
    }
    std::string tagsCsv() const {
        std::string out;
        for (size_t i = 0; i < fEditTags.size(); ++i) {
            if (i) out += ",";
            out += fEditTags[i];
        }
        return out;
    }
    void setTagsFromCsv(const std::string& csv) {
        fEditTags.clear();
        std::string token;
        for (char c : csv) {
            if (c == ',' || c == ';') {
                const std::string t = trimTag(token);
                if (!t.empty()) fEditTags.push_back(t);
                token.clear();
            } else {
                token.push_back(c);
            }
        }
        const std::string t = trimTag(token);
        if (!t.empty()) fEditTags.push_back(t);
        rebuildInfoTagChips();
    }
    void rebuildInfoTagChips() {
        if (!fInfoTagRow) return;
        // Delete chip children only; the input must survive.
        for (int32_t i = (int32_t)lv_obj_get_child_count(fInfoTagRow) - 1; i >= 0; --i) {
            lv_obj_t* ch = lv_obj_get_child(fInfoTagRow, (uint32_t)i);
            if (ch != fTagInput) lv_obj_delete(ch);
        }
        for (size_t t = 0; t < fEditTags.size(); ++t) {
            lv_obj_t* chip = lv_obj_create(fInfoTagRow);
            lv_obj_set_size(chip, LV_SIZE_CONTENT, scaled(20));
            lv_obj_set_style_bg_color(chip, COL_PANEL_DARK, 0);
            lv_obj_set_style_bg_opa(chip, LV_OPA_COVER, 0);
            lv_obj_set_style_border_color(chip, COL_BORDER, 0);
            lv_obj_set_style_border_width(chip, 1, 0);
            lv_obj_set_style_radius(chip, scaled(3), 0);
            lv_obj_set_style_pad_hor(chip, scaled(5), 0);
            lv_obj_set_style_pad_ver(chip, 0, 0);
            lv_obj_set_style_pad_column(chip, scaled(4), 0);
            lv_obj_set_layout(chip, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(chip, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(chip, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(chip, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t* lbl = lv_label_create(chip);
            lv_label_set_text(lbl, fEditTags[t].c_str());
            lv_obj_set_style_text_font(lbl, getScaledSmallFont(), 0);
            lv_obj_set_style_text_color(lbl, COL_TEXT, 0);

            // A real 14x14 hit target, not a bare 5px glyph: the remove
            // control must be easy to hit at every zoom.
            lv_obj_t* x = lv_button_create(chip);
            lv_obj_set_size(x, scaled(14), scaled(14));
            lv_obj_set_style_pad_all(x, 0, 0);
            lv_obj_set_style_bg_opa(x, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(x, 0, 0);
            lv_obj_set_style_shadow_width(x, 0, 0);
            lv_obj_set_style_radius(x, LV_RADIUS_CIRCLE, 0);
            lv_obj_t* xl = lv_label_create(x);
            lv_label_set_text(xl, "x");
            lv_obj_set_style_text_font(xl, uiMicroFont(), 0);
            lv_obj_set_style_text_color(xl, COL_TEXT_DIM, 0);
            lv_obj_center(xl);
            lv_obj_set_user_data(x, (void*)(intptr_t)t);
            lv_obj_add_event_cb(x, [](lv_event_t* e) {
                PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
                lv_obj_t* tgt = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
                if (b) b->removeInfoTag((size_t)(intptr_t)lv_obj_get_user_data(tgt));
            }, LV_EVENT_CLICKED, this);

            lv_obj_move_to_index(chip, (int32_t)t);
        }
        if (fTagInput) lv_obj_move_to_index(fTagInput, -1);
    }
    void commitTagInput() {
        if (!fTagInput) return;
        std::string raw = lv_textarea_get_text(fTagInput);
        std::string token;
        auto flush = [&]() {
            const std::string t = trimTag(token);
            token.clear();
            if (t.empty()) return;
            const std::string folded = foldTag(t);
            for (const std::string& have : fEditTags)
                if (foldTag(have) == folded) return;
            if (fEditTags.size() < 12) fEditTags.push_back(t);
        };
        for (char c : raw) {
            if (c == ',' || c == ';') flush();
            else token.push_back(c);
        }
        flush();
        lv_textarea_set_text(fTagInput, "");
        rebuildInfoTagChips();
        updateProfilePanel();
    }
    void removeInfoTag(size_t index) {
        if (index >= fEditTags.size()) return;
        fEditTags.erase(fEditTags.begin() + (std::ptrdiff_t)index);
        rebuildInfoTagChips();
        updateProfilePanel();
    }

    // --- library mini-card (round-4) ---
    // 16px chips: one per bank family with the preset count, led by the total.
    // Rebuilt with the list so it tracks a refresh/import.
    void rebuildBankChips() {
        if (!fBankChips) return;
        lv_obj_clean(fBankChips);

        struct Bank { std::string label; int count; };
        std::vector<Bank> banks;
        int rootPresets = 0;
        for (const PresetListEntry& e : fEntries) {
            const size_t slash = e.info.folder.find('/');
            const std::string root = (slash == std::string::npos)
                ? e.info.folder : e.info.folder.substr(0, slash);
            if (root.empty()) { ++rootPresets; continue; }
            Bank b;
            b.label = root;
            for (char& c : b.label) c = (char)std::toupper((unsigned char)c);
            b.count = 1;
            bool found = false;
            for (Bank& existing : banks) {
                if (existing.label == b.label) { ++existing.count; found = true; break; }
            }
            if (!found) banks.push_back(b);
        }

        std::vector<Bank> ordered;
        if (rootPresets > 0) ordered.push_back(Bank{"ROOT", rootPresets});
        for (const std::string& s : skeletonRoots()) {
            std::string want = s;
            for (char& c : want) c = (char)std::toupper((unsigned char)c);
            for (const Bank& b : banks) {
                if (b.label == want) { ordered.push_back(b); break; }
            }
        }
        std::vector<Bank> extras;
        for (const Bank& b : banks) {
            bool known = false;
            for (const Bank& o : ordered) {
                if (o.label == b.label) { known = true; break; }
            }
            if (!known) extras.push_back(b);
        }
        std::sort(extras.begin(), extras.end(),
                  [](const Bank& a, const Bank& b) { return a.label < b.label; });
        for (const Bank& b : extras) ordered.push_back(b);

        char total[48];
        std::snprintf(total, sizeof(total), "%zu PRESETS", fEntries.size());
        PresetRowGlyphs::createMiniChip(fBankChips, total, COL_HIGHLIGHT);
        // Cap the chips so the card can never push the Ready line off panel.
        const size_t kMaxBankChips = 5;
        size_t shown = 0, hidden = 0;
        for (const Bank& b : ordered) {
            if (shown >= kMaxBankChips) { hidden += 1; continue; }
            char text[48];
            std::snprintf(text, sizeof(text), "%s %d", b.label.c_str(), b.count);
            PresetRowGlyphs::createMiniChip(fBankChips, text, PresetRowGlyphs::kindColourFor(
                PresetBrowserLogic::kindFromBank(b.label == "ROOT" ? "" : b.label)));
            ++shown;
        }
        if (hidden > 0) {
            char text[24];
            std::snprintf(text, sizeof(text), "+%zu", hidden);
            PresetRowGlyphs::createMiniChip(fBankChips, text, COL_TEXT_DIM);
        }
    }

private:
    struct PresetListEntry {
        PluginCymbals::PresetFileInfo info;
        std::string tagsCsv;
        std::string searchable;  // name + desc + usage + author + tagsCsv
        std::string relKey;      // lowercased POSIX rel path
        std::filesystem::file_time_type mtime{};  // file mtime for Date/Recent sort
    };

    enum class ViewKind { All, Favourites, Folder };
    struct ViewState {
        ViewKind kind = ViewKind::All;
        std::string folder;  // relPath; "" = root (used when kind == Folder)
    };

    struct TreeNode {
        bool smart = false;
        int smartKind = 0;       // 0 = All Presets, 1 = Favourites, 2 = Tags
        std::string relPath;     // folder relPath ("" = root); smart nodes use smartKind
        std::string label;
        int depth = 0;
        lv_obj_t* btn = nullptr;
    };

    struct DragState {
        bool active = false;
        int sourceNode = -1;
        lv_point_t pressPoint{0, 0};
        std::vector<std::string> files;
        lv_obj_t* highlight = nullptr;
        int highlightNode = -1;
    };

    // --- fixed skeleton dirs (created on demand) ---
    static std::string joinTagsCsv(const std::vector<std::string>& tags) {
        std::string csv;
        for (size_t i = 0; i < tags.size(); ++i) {
            if (i > 0) csv += ",";
            csv += tags[i];
        }
        return csv;
    }
    // Re-serialise a parsed query back to a space-separated string. Used by
    // the live-typing path so applyFilter() (which expects a const char*
    // raw query) gets the same text the user typed.
    static std::string joinQueryTokens(const std::vector<QueryToken>& tokens) {
        std::string out;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (i > 0) out += ' ';
            const bool quote = tokens[i].isPhrase ||
                (tokens[i].isTag && tokens[i].text.find(' ') != std::string::npos);
            if (tokens[i].isTag) out += '#';
            if (quote) out += '"';
            out += tokens[i].text;
            if (quote) out += '"';
        }
        return out;
    }


    static const std::vector<std::string>& skeletonRoots() {
        static const std::vector<std::string> k = {"basses", "leads", "pads", "effects", "cymbals", "percussion"};
        return k;
    }
    static const std::vector<std::string>& cymbalsChildren() {
        static const std::vector<std::string> k = {"crashes", "orchestral", "hi-hat", "bell", "ride", "sizzle", "splash", "swish"};
        return k;
    }

    static void ensureSkeletonDirs(const std::string& dir) {
        try {
            const std::filesystem::path base = std::filesystem::u8path(dir);
            for (const std::string& r : skeletonRoots()) {
                std::filesystem::create_directories(base / r);
                if (r == "cymbals") {
                    for (const std::string& c : cymbalsChildren()) {
                        std::filesystem::create_directories(base / r / c);
                    }
                }
            }
        } catch (...) { /* never block the browser on dir errors */ }
    }

    void rebuildEntries(const std::vector<PluginCymbals::PresetFileInfo>& files) {
        fEntries.clear();
        fEntries.reserve(files.size());
        for (const auto& f : files) {
            PresetListEntry e;
            e.info = f;
            e.tagsCsv = joinTagsCsv(f.tags);
            e.searchable = f.name + " " + f.description + " " + f.usage + " " + f.author + " " + e.tagsCsv;
            e.relKey = relKey(fPresetDir, f.path);
            try {
                e.mtime = std::filesystem::last_write_time(std::filesystem::u8path(f.path));
            } catch (...) {
                e.mtime = std::filesystem::file_time_type::min();
            }
            fEntries.push_back(std::move(e));
        }
    }

    void reloadFavouritesAndRebuild() {
        fFavourites = readFavourites(fPresetDir);
        rebuildTree();
        rebuildList();
        rebuildBankChips();
        syncFavouriteSwatches();
        updateProfilePanel();
    }

    // --- left column: DIRECTORY ---
    void buildLeftColumn(lv_obj_t* body) {
        lv_obj_t* col = lv_obj_create(body);
        lv_obj_set_size(col, scaled(250), lv_pct(100));
        lv_obj_add_style(col, &fStyles->panel, 0);
        lv_obj_set_layout(col, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(col, scaled(6), 0);
        lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* title = lv_label_create(col);
        lv_label_set_text(title, "DIRECTORY");
        lv_obj_add_style(title, &fStyles->title, 0);

        // Search row: textarea + TAGS picker button.
        lv_obj_t* searchRow = lv_obj_create(col);
        lv_obj_set_size(searchRow, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(searchRow, 0, 0);
        lv_obj_set_style_border_width(searchRow, 0, 0);
        lv_obj_set_layout(searchRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(searchRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(searchRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(searchRow, scaled(4), 0);
        lv_obj_clear_flag(searchRow, LV_OBJ_FLAG_SCROLLABLE);

        fSearchArea = lv_textarea_create(searchRow);
        uiBindTextInputToOwnIndev(fSearchArea);
        lv_obj_set_size(fSearchArea, 0, scaled(32));
        lv_obj_set_flex_grow(fSearchArea, 1);
        lv_textarea_set_one_line(fSearchArea, true);
        lv_textarea_set_placeholder_text(fSearchArea, "Search presets...");
        lv_obj_add_event_cb(fSearchArea, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (!b) return;
            b->fQuery = parseQuery(lv_textarea_get_text(b->fSearchArea));
            b->rebuildList();
        }, LV_EVENT_VALUE_CHANGED, this);

        lv_obj_t* tagsBtn = lv_btn_create(searchRow);
        lv_obj_set_size(tagsBtn, scaled(56), scaled(32));
        fStyles->applyMomentaryButton(tagsBtn);
        lv_obj_add_event_cb(tagsBtn, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (b) b->openTagPicker();
        }, LV_EVENT_CLICKED, this);
        lv_obj_t* tagsLbl = lv_label_create(tagsBtn);
        lv_label_set_text(tagsLbl, "TAGS");
        lv_obj_set_style_text_font(tagsLbl, getScaledSmallFont(), 0);
        lv_obj_center(tagsLbl);

        lv_obj_t* refreshBtn = lv_btn_create(col);
        lv_obj_set_size(refreshBtn, lv_pct(100), scaled(28));
        fStyles->applyMomentaryButton(refreshBtn);
        lv_obj_add_event_cb(refreshBtn, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (!b) return;
            b->refresh(true);
            b->setStatus("Preset list refreshed", false);
        }, LV_EVENT_CLICKED, this);
        lv_obj_t* refreshLbl = lv_label_create(refreshBtn);
        lv_label_set_text(refreshLbl, "REFRESH");
        lv_obj_set_style_text_font(refreshLbl, getScaledSmallFont(), 0);
        lv_obj_center(refreshLbl);

        fTree = lv_obj_create(col);
        lv_obj_set_size(fTree, lv_pct(100), 0);
        lv_obj_set_flex_grow(fTree, 1);
        lv_obj_set_style_bg_opa(fTree, 0, 0);
        lv_obj_set_style_border_width(fTree, 0, 0);
        lv_obj_set_style_pad_all(fTree, 0, 0);
        lv_obj_set_layout(fTree, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(fTree, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(fTree, scaled(3), 0);
        lv_obj_set_scrollbar_mode(fTree, LV_SCROLLBAR_MODE_AUTO);
        UIWidgets::attachScrollbarDrag(fTree);
        lv_obj_add_flag(fTree, LV_OBJ_FLAG_SCROLLABLE);
    }

    // --- center column: PRESETS list ---
    // Round-6: the list must FILL the panel. A fresh install has five presets and
    // the old column ended at the last card, leaving a dead dark void under it,
    // so the frame is now a recessed well that grows with the column and the
    // cards stay a fixed height at its top. A hairline divider separates the
    // well from a fixed footer band that carries the legend, so the panel
    // bottom is reached and the column reads as finished at any list length.
    void buildCenterColumn(lv_obj_t* body) {
        lv_obj_t* col = lv_obj_create(body);
        lv_obj_set_size(col, 0, lv_pct(100));
        lv_obj_set_flex_grow(col, 1);
        lv_obj_add_style(col, &fStyles->panel, 0);
        lv_obj_set_layout(col, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(col, scaled(4), 0);
        lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* title = lv_label_create(col);
        lv_label_set_text(title, "Presets");
        lv_obj_add_style(title, &fStyles->title, 0);

        lv_obj_t* hint = lv_label_create(col);
        // U+2014 (em dash) is NOT in the shipped faces: montserrat_10/12/14
        // carry 0x20-0x7F plus 0xB0 and 0x2022 (bullet) only, so the dash drew
        // as a missing-glyph box in this line. The hyphen is the same width and
        // the bullet is the only other punctuation the face guarantees.
        lv_label_set_text(hint, "Click a card to load it - tap the star to favourite.");
        lv_obj_set_style_text_color(hint, COL_TEXT_DIM, 0);
        lv_obj_set_style_text_font(hint, getScaledSmallFont(), 0);

        buildFilterBar(col);

        // Recessed list well (COL_PANEL_DARK on the plate). It grows to the
        // column and scrolls internally; the divider and the band under it are
        // real children (draw callbacks never render reliably here).
        fList = lv_obj_create(col);
        lv_obj_set_size(fList, lv_pct(100), 0);
        lv_obj_set_flex_grow(fList, 1);
        lv_obj_set_style_bg_color(fList, COL_PANEL_DARK, 0);
        lv_obj_set_style_bg_opa(fList, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(fList, 0, 0);
        lv_obj_set_style_radius(fList, scaled(4), 0);
        lv_obj_set_style_pad_all(fList, scaled(8), 0);
        lv_obj_set_layout(fList, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(fList, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(fList, scaled(6), 0);
        lv_obj_set_scrollbar_mode(fList, LV_SCROLLBAR_MODE_AUTO);
        UIWidgets::attachScrollbarDrag(fList);
        // The list is one of the two sanctioned scroll surfaces (unbounded
        // preset libraries); the panel and the band never scroll.
        lv_obj_add_flag(fList, LV_OBJ_FLAG_SCROLLABLE);

        // Divider between the well and the footer band.
        lv_obj_t* rule = lv_obj_create(col);
        lv_obj_set_size(rule, lv_pct(100), scaled(1));
        lv_obj_set_style_bg_color(rule, COL_HAIRLINE, 0);
        lv_obj_set_style_bg_opa(rule, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(rule, 0, 0);
        lv_obj_set_style_border_width(rule, 0, 0);
        lv_obj_set_style_pad_all(rule, 0, 0);
        lv_obj_clear_flag(rule, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(rule, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(rule, LV_SCROLLBAR_MODE_OFF);

        // Footer band: a fixed strip that closes the column. It is NOT a
        // growing sibling - the well above already takes the leftover - and it
        // holds three lines of the micro face (11px line height each) inside
        // its 6px pads. The legend is written by rebuildList().
        fSpaceBand = lv_obj_create(col);
        lv_obj_set_size(fSpaceBand, lv_pct(100), scaled(56));
        lv_obj_set_flex_grow(fSpaceBand, 0);
        lv_obj_set_layout(fSpaceBand, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(fSpaceBand, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(fSpaceBand, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
        lv_obj_set_style_bg_opa(fSpaceBand, 0, 0);
        lv_obj_set_style_border_width(fSpaceBand, 0, 0);
        lv_obj_set_style_pad_all(fSpaceBand, scaled(6), 0);
        lv_obj_clear_flag(fSpaceBand, LV_OBJ_FLAG_SCROLLABLE);

        fSpaceLegend = lv_label_create(fSpaceBand);
        lv_label_set_text(fSpaceLegend, "");
        lv_obj_set_width(fSpaceLegend, lv_pct(100));
        lv_label_set_long_mode(fSpaceLegend, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_color(fSpaceLegend, COL_TEXT_DIM, 0);
        lv_obj_set_style_text_font(fSpaceLegend, uiMicroFont(), 0);
        lv_obj_set_style_text_letter_space(fSpaceLegend, scaled(1), 0);
    }


    // --- round-5: filter bar ---
    // Row 1: TYPE label + the type chips, WRAPPED. Row 2: SORT dropdown + COUNT
    // chip + the active tag chips. Round-6: the bar was one crowded row, so the
    // last TYPE chip ("HATS") was clipped to "HA" and the SORT label sat jammed
    // against it. Flex row-wrap with two stacked rows (pad_row) fits the chips
    // at any column width and reads as one control group; nothing scrolls.
    // Budget: the tab viewport is ~574px at scale 1.0, so the two rows cost
    // ~48px (24px row 1 + 4px pad_row + ~20px row 2) against the old ~24px -
    // the list well absorbs it. The chips keep the tighter 6px pad_hor (was
    // 8px) so the first row stays close to one line at 1280.
    void buildFilterBar(lv_obj_t* col) {
        fFilterBar = lv_obj_create(col);
        lv_obj_set_size(fFilterBar, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(fFilterBar, 0, 0);
        lv_obj_set_style_bg_opa(fFilterBar, 0, 0);
        lv_obj_set_style_border_width(fFilterBar, 0, 0);
        lv_obj_set_layout(fFilterBar, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(fFilterBar, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(fFilterBar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(fFilterBar, scaled(4), 0);
        lv_obj_clear_flag(fFilterBar, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(fFilterBar, LV_SCROLLBAR_MODE_OFF);

        // Row 1: TYPE label + chips. The chips wrap onto a second line rather
        // than overflowing, so the last one is always whole.
        lv_obj_t* typeRow = lv_obj_create(fFilterBar);
        lv_obj_set_size(typeRow, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(typeRow, 0, 0);
        lv_obj_set_style_bg_opa(typeRow, 0, 0);
        lv_obj_set_style_border_width(typeRow, 0, 0);
        lv_obj_set_layout(typeRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(typeRow, LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_flex_align(typeRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_column(typeRow, scaled(4), 0);
        lv_obj_set_style_pad_row(typeRow, scaled(4), 0);
        lv_obj_clear_flag(typeRow, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(typeRow, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t* typeLbl = lv_label_create(typeRow);
        lv_label_set_text(typeLbl, "TYPE");
        lv_obj_set_style_text_font(typeLbl, uiMicroFont(), 0);
        lv_obj_set_style_text_color(typeLbl, COL_TEXT_DIM, 0);
        lv_obj_set_style_text_letter_space(typeLbl, scaled(1), 0);

        static const char* kTypeLabels[7] = {"ALL","CYMBALS","PERCUSSION","FX","CRASH","RIDE","HATS"};
        for (int i = 0; i < 7; ++i) {
            lv_obj_t* chip = lv_btn_create(typeRow);
            lv_obj_set_size(chip, LV_SIZE_CONTENT, scaled(24));
            lv_obj_add_flag(chip, LV_OBJ_FLAG_CHECKABLE);
            fStyles->applyMomentaryButton(chip);
            lv_obj_set_style_pad_hor(chip, scaled(6), 0);
            lv_obj_set_style_pad_ver(chip, 0, 0);
            lv_obj_set_style_radius(chip, scaled(4), 0);
            lv_obj_set_user_data(chip, (void*)(intptr_t)i);
            lv_obj_add_event_cb(chip, [](lv_event_t* e) {
                PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
                if (!b) return;
                lv_obj_t* tgt = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
                const int idx = (int)(intptr_t)lv_obj_get_user_data(tgt);
                b->fTypeFilter = idx;
                // Uncheck all other chips
                for (int j = 0; j < 7; ++j) {
                    if (j != idx && b->fTypeChips[j]) lv_obj_clear_state(b->fTypeChips[j], LV_STATE_CHECKED);
                }
                b->rebuildList();
            }, LV_EVENT_CLICKED, this);
            lv_obj_t* lbl = lv_label_create(chip);
            lv_label_set_text(lbl, kTypeLabels[i]);
            lv_obj_set_style_text_font(lbl, uiMicroFont(), 0);
            lv_obj_set_style_text_letter_space(lbl, scaled(1), 0);
            lv_obj_center(lbl);
            fTypeChips[i] = chip;
        }
        // Check ALL by default
        lv_obj_add_state(fTypeChips[0], LV_STATE_CHECKED);

        // Row 2: SORT dropdown | COUNT chip | active tag chips. Right-aligned
        // so the group keeps its own edge and never overlaps the TYPE row.
        lv_obj_t* rightGroup = lv_obj_create(fFilterBar);
        lv_obj_set_size(rightGroup, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(rightGroup, 0, 0);
        lv_obj_set_style_bg_opa(rightGroup, 0, 0);
        lv_obj_set_style_border_width(rightGroup, 0, 0);
        lv_obj_set_layout(rightGroup, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(rightGroup, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(rightGroup, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(rightGroup, scaled(6), 0);
        lv_obj_clear_flag(rightGroup, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(rightGroup, LV_SCROLLBAR_MODE_OFF);

        // SORT dropdown
        lv_obj_t* sortLbl = lv_label_create(rightGroup);
        lv_label_set_text(sortLbl, "SORT");
        lv_obj_set_style_text_font(sortLbl, uiMicroFont(), 0);
        lv_obj_set_style_text_color(sortLbl, COL_TEXT_DIM, 0);
        lv_obj_set_style_text_letter_space(sortLbl, scaled(1), 0);

        fSortDropdown = lv_dropdown_create(rightGroup);
        lv_obj_set_width(fSortDropdown, scaled(90));
        lv_obj_add_style(fSortDropdown, &fStyles->compactSelectMain, LV_PART_MAIN);
        lv_obj_add_style(fSortDropdown, &fStyles->compactSelectIndicator, LV_PART_INDICATOR);
        uiEnableDropdownListScroll(fSortDropdown);
        uiFixDropdownPopup(fSortDropdown);
        lv_dropdown_set_options(fSortDropdown, "Name\nDate\nRecent");
        lv_obj_add_event_cb(fSortDropdown, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (!b) return;
            b->fSortOrder = (int)lv_dropdown_get_selected(b->fSortDropdown);
            b->rebuildList();
        }, LV_EVENT_VALUE_CHANGED, this);

        // COUNT chip
        fCountChip = lv_label_create(rightGroup);
        lv_label_set_text(fCountChip, "0 PRESETS");
        lv_obj_set_style_text_font(fCountChip, uiMicroFont(), 0);
        lv_obj_set_style_text_color(fCountChip, COL_HIGHLIGHT, 0);
        lv_obj_set_style_text_letter_space(fCountChip, scaled(1), 0);
        lv_obj_set_style_bg_color(fCountChip, COL_PANEL_DARK, 0);
        lv_obj_set_style_bg_opa(fCountChip, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(fCountChip, 1, 0);
        lv_obj_set_style_border_color(fCountChip, COL_HAIRLINE, 0);
        lv_obj_set_style_radius(fCountChip, scaled(4), 0);
        lv_obj_set_style_pad_hor(fCountChip, scaled(8), 0);
        lv_obj_set_style_pad_ver(fCountChip, scaled(4), 0);

        // Tag chips container (rebuilt on query change)
        fTagChips = lv_obj_create(rightGroup);
        lv_obj_set_size(fTagChips, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(fTagChips, 0, 0);
        lv_obj_set_style_bg_opa(fTagChips, 0, 0);
        lv_obj_set_style_border_width(fTagChips, 0, 0);
        lv_obj_set_layout(fTagChips, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(fTagChips, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(fTagChips, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(fTagChips, scaled(4), 0);
        lv_obj_clear_flag(fTagChips, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(fTagChips, LV_SCROLLBAR_MODE_OFF);

        // The row hands the tag-chip space to the group (the widest tag chip is
        // ~= 75px), so the SORT control is never shoved off the right edge when
        // the user filters by several tags. The dropdown's own 4px pads + the
        // 24px type chips made the row 32px tall; 2px keeps both rows level.
        lv_obj_set_flex_grow(fTagChips, 1);
        lv_obj_set_style_pad_top(fSortDropdown, scaled(2), 0);
        lv_obj_set_style_pad_bottom(fSortDropdown, scaled(2), 0);
    }

    // Rebuild the active tag chips from fQuery (tag tokens only).
    void rebuildTagChips() {
        if (!fTagChips) return;
        lv_obj_clean(fTagChips);
        for (const QueryToken& tok : fQuery) {
            if (!tok.isTag) continue;
            lv_obj_t* chip = lv_btn_create(fTagChips);
            lv_obj_set_size(chip, LV_SIZE_CONTENT, scaled(20));
            fStyles->applyMomentaryButton(chip);
            lv_obj_set_style_pad_hor(chip, scaled(6), 0);
            lv_obj_set_style_pad_ver(chip, 0, 0);
            lv_obj_set_style_radius(chip, scaled(4), 0);
            lv_obj_set_user_data(chip, nullptr);  // TODO: remove-tag wiring
            lv_obj_t* lbl = lv_label_create(chip);
            lv_label_set_text(lbl, ("#" + tok.text).c_str());
            lv_obj_set_style_text_font(lbl, uiMicroFont(), 0);
            lv_obj_set_style_text_color(lbl, COL_ACCENT, 0);
            lv_obj_set_style_text_letter_space(lbl, scaled(1), 0);
            lv_obj_center(lbl);
        }
    }

    // Update the count chip text from fFilteredEntries.
    void updateCountChip() {
        if (!fCountChip) return;
        char buf[48];
        std::snprintf(buf, sizeof(buf), "%zu PRESETS", fFilteredEntries.size());
        lv_label_set_text(fCountChip, buf);
    }

    // --- right column: PRESET INFO ---
    void buildRightColumn(lv_obj_t* body) {
        lv_obj_t* col = lv_obj_create(body);
        lv_obj_set_size(col, scaled(300), lv_pct(100));
        lv_obj_add_style(col, &fStyles->panel, 0);

        lv_obj_set_layout(col, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(col, scaled(4), 0);
        lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* title = lv_label_create(col);
        lv_label_set_text(title, "PRESET INFO");
        lv_obj_add_style(title, &fStyles->title, 0);

        fFolderLabel = lv_label_create(col);
        lv_label_set_text(fFolderLabel, "");
        lv_obj_set_style_text_color(fFolderLabel, COL_TEXT_DIM, 0);
        lv_obj_set_style_text_font(fFolderLabel, getScaledSmallFont(), 0);
        lv_label_set_long_mode(fFolderLabel, LV_LABEL_LONG_DOT);
        lv_obj_set_width(fFolderLabel, lv_pct(100));

        // Favourite swatches for the currently loaded preset.
        lv_obj_t* favRow = lv_obj_create(col);
        lv_obj_set_size(favRow, lv_pct(100), LV_SIZE_CONTENT);
        // Theme's default 20px card padding triples this slim swatch row.
        lv_obj_set_style_pad_all(favRow, 0, 0);
        lv_obj_set_style_bg_opa(favRow, 0, 0);
        lv_obj_set_style_border_width(favRow, 0, 0);
        lv_obj_set_layout(favRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(favRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(favRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(favRow, scaled(4), 0);
        lv_obj_clear_flag(favRow, LV_OBJ_FLAG_SCROLLABLE);

        // Form field: tracked-caps label above a COL_PANEL_DARK inset with a
        // #505050 bezel (critic closeout — replaces the flat wireframe rows).
        auto addField = [&](const char* label, lv_obj_t** out, lv_obj_t* parent) {
            lv_obj_t* lbl = lv_label_create(parent);
            lv_label_set_text(lbl, label);
            lv_obj_add_style(lbl, &fStyles->title, 0);  // tracked caps tier
            lv_obj_set_style_text_font(lbl, getScaledSmallFont(), 0);  // 12px
            lv_obj_t* area = lv_textarea_create(parent);
            uiBindTextInputToOwnIndev(area);
            lv_obj_set_width(area, lv_pct(100));
            lv_textarea_set_one_line(area, true);
            // one_line() re-derives the height from padding — with the theme's
            // default pads that balloons to ~42px per field and pushes the
            // column past the panel. Slim pads keep the whole form on screen.
            lv_obj_set_style_pad_top(area, scaled(4), LV_PART_MAIN);
            lv_obj_set_style_pad_bottom(area, scaled(4), LV_PART_MAIN);
            lv_obj_set_style_pad_left(area, scaled(6), LV_PART_MAIN);
            lv_obj_set_style_pad_right(area, scaled(6), LV_PART_MAIN);
            lv_obj_set_style_bg_color(area, COL_PANEL_DARK, LV_PART_MAIN);  // inset well
            lv_obj_set_style_border_color(area, COL_BORDER, LV_PART_MAIN);  // #505050 bezel
            lv_obj_set_style_border_width(area, 1, LV_PART_MAIN);
            lv_obj_set_style_radius(area, scaled(3), LV_PART_MAIN);
            lv_obj_set_height(area, scaled(24));
            // Any text the user types pins this field until a preset load
            // pushes fresh metadata over it (see syncInfoPanelFromPlugin).
            lv_obj_add_event_cb(area, [](lv_event_t* e) {
                PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
                if (!b || b->fApplyingInfo) return;
                lv_obj_t* t = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
                if (t == b->fNameArea) b->fNameEdited = true;
                else if (t == b->fAuthorArea) b->fAuthorEdited = true;
                else if (t == b->fDescArea) b->fDescEdited = true;
                else if (t == b->fUsageArea) b->fUsageEdited = true;
                // The tonal profile reads the live text: repaint as the user
                // edits any metadata field, not only after a load.
                b->updateProfilePanel();
            }, LV_EVENT_VALUE_CHANGED, this);
            *out = area;
        };
        addField("Name", &fNameArea, col);

        // Author + Tags share a two-column row (both are short fields).
        lv_obj_t* pairRow = lv_obj_create(col);
        lv_obj_set_size(pairRow, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(pairRow, 0, 0);
        lv_obj_set_style_bg_opa(pairRow, 0, 0);
        lv_obj_set_style_border_width(pairRow, 0, 0);
        lv_obj_set_layout(pairRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(pairRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_column(pairRow, scaled(6), 0);
        lv_obj_clear_flag(pairRow, LV_OBJ_FLAG_SCROLLABLE);
        addField("Author", &fAuthorArea, pairRow);
        lv_obj_set_width(fAuthorArea, 0);
        lv_obj_set_flex_grow(fAuthorArea, 1);

        // --- Tags: caption + wrapping chip row + inline "add tag" input -----
        // The preset format already stores a string ARRAY ("tags": [...]), so
        // the panel edits chips, not a comma blob: Enter (or a typed comma)
        // commits a tag, each chip carries an x to remove it, and SAVE writes
        // every chip as its own array element.
        lv_obj_t* tagsRow = lv_obj_create(col);
        lv_obj_set_size(tagsRow, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(tagsRow, 0, 0);
        lv_obj_set_style_pad_column(tagsRow, scaled(6), 0);
        lv_obj_set_style_bg_opa(tagsRow, 0, 0);
        lv_obj_set_style_border_width(tagsRow, 0, 0);
        lv_obj_set_layout(tagsRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(tagsRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(tagsRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(tagsRow, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* tagsCap = lv_label_create(tagsRow);
        lv_label_set_text(tagsCap, "Tags");
        lv_obj_add_style(tagsCap, &fStyles->title, 0);
        lv_obj_set_style_text_font(tagsCap, getScaledSmallFont(), 0);

        fInfoTagRow = lv_obj_create(tagsRow);
        lv_obj_set_size(fInfoTagRow, 0, LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(fInfoTagRow, 1);
        lv_obj_set_style_bg_opa(fInfoTagRow, 0, 0);
        lv_obj_set_style_border_width(fInfoTagRow, 0, 0);
        lv_obj_set_style_pad_all(fInfoTagRow, 0, 0);
        lv_obj_set_style_pad_gap(fInfoTagRow, scaled(4), 0);
        lv_obj_set_layout(fInfoTagRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(fInfoTagRow, LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_flex_align(fInfoTagRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(fInfoTagRow, LV_OBJ_FLAG_SCROLLABLE);

        fTagInput = lv_textarea_create(fInfoTagRow);
        uiBindTextInputToOwnIndev(fTagInput);
        lv_obj_set_size(fTagInput, scaled(86), scaled(22));
        lv_textarea_set_one_line(fTagInput, true);
        lv_textarea_set_max_length(fTagInput, 24);
        lv_textarea_set_placeholder_text(fTagInput, "add tag");
        lv_obj_set_style_pad_top(fTagInput, scaled(2), LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(fTagInput, scaled(2), LV_PART_MAIN);
        lv_obj_set_style_pad_left(fTagInput, scaled(5), LV_PART_MAIN);
        lv_obj_set_style_pad_right(fTagInput, scaled(5), LV_PART_MAIN);
        lv_obj_set_style_radius(fTagInput, scaled(3), LV_PART_MAIN);
        lv_obj_set_style_bg_color(fTagInput, COL_PANEL_DARK, LV_PART_MAIN);
        lv_obj_set_style_border_color(fTagInput, COL_BORDER, LV_PART_MAIN);
        lv_obj_set_style_border_width(fTagInput, 1, LV_PART_MAIN);
        lv_obj_set_style_text_font(fTagInput, getScaledSmallFont(), 0);
        lv_obj_add_event_cb(fTagInput, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (b) b->commitTagInput();
        }, LV_EVENT_READY, this);
        // Comma completes a tag too: typing "bright," leaves a clean input for
        // the next one instead of accumulating a CSV blob in the box.
        lv_obj_add_event_cb(fTagInput, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (!b || !b->fTagInput) return;
            const char* txt = lv_textarea_get_text(b->fTagInput);
            if (txt && txt[0] && (txt[std::strlen(txt) - 1] == ',' || txt[std::strlen(txt) - 1] == ';'))
                b->commitTagInput();
        }, LV_EVENT_VALUE_CHANGED, this);

        addField("Description", &fDescArea, col);
        addField("Usage", &fUsageArea, col);

        static const lv_color_t kSwatchColours[6] = {
            COL_FAV_RED, COL_FAV_ORANGE, COL_FAV_YELLOW,
            COL_FAV_GREEN, COL_FAV_BLUE, COL_FAV_PURPLE,
        };
        for (int i = 0; i < 6; ++i) {
            lv_obj_t* sw = lv_btn_create(favRow);
            lv_obj_set_size(sw, scaled(18), scaled(18));
            lv_obj_set_style_radius(sw, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(sw, kSwatchColours[i], 0);
            lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(sw, 1, 0);
            lv_obj_set_style_border_color(sw, COL_HAIRLINE, 0);
            lv_obj_set_style_pad_all(sw, 0, 0);
            lv_obj_set_user_data(sw, (void*)(intptr_t)i);
            lv_obj_add_event_cb(sw, [](lv_event_t* e) {
                PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
                lv_obj_t* tgt = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
                if (b) b->setLoadedFavourite((int)(intptr_t)lv_obj_get_user_data(tgt), true);
            }, LV_EVENT_CLICKED, this);
            fSwatches[i] = sw;
        }
        lv_obj_t* offBtn = lv_btn_create(favRow);
        lv_obj_set_size(offBtn, scaled(40), scaled(18));
        fStyles->applyMomentaryButton(offBtn);
        lv_obj_set_style_pad_all(offBtn, 0, 0);
        lv_obj_add_event_cb(offBtn, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (b) b->setLoadedFavourite(-1, false);
        }, LV_EVENT_CLICKED, this);
        lv_obj_t* offLbl = lv_label_create(offBtn);
        lv_label_set_text(offLbl, "OFF");
        lv_obj_set_style_text_font(offLbl, uiMicroFont(), 0);
        lv_obj_center(offLbl);

        // Action row: SAVE | math mode | IR length | EXPORT IR.
        lv_obj_t* actionRow = lv_obj_create(col);
        lv_obj_set_size(actionRow, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(actionRow, 0, 0);
        lv_obj_set_style_bg_opa(actionRow, 0, 0);
        lv_obj_set_style_border_width(actionRow, 0, 0);
        lv_obj_set_layout(actionRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(actionRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(actionRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(actionRow, scaled(6), 0);
        lv_obj_clear_flag(actionRow, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* saveBtn = lv_btn_create(actionRow);
        lv_obj_set_size(saveBtn, scaled(78), scaled(30));
        fStyles->applyMomentaryButton(saveBtn);
        lv_obj_add_event_cb(saveBtn, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (b) b->saveFromFields();
        }, LV_EVENT_CLICKED, this);
        lv_obj_t* saveLbl = lv_label_create(saveBtn);
        lv_label_set_text(saveLbl, "SAVE");
        lv_obj_center(saveLbl);


        // IR controls join the action row (critic closeout): one grouped
        // raised-action row instead of two orphaned rows.
        lv_obj_t* irLbl = lv_label_create(actionRow);
        lv_label_set_text(irLbl, "IR");
        lv_obj_add_style(irLbl, &fStyles->title, 0);
        lv_obj_set_style_text_font(irLbl, uiMicroFont(), 0);

        fIrDropdown = lv_dropdown_create(actionRow);
        lv_obj_set_width(fIrDropdown, scaled(78));
        // Raised machined select face (matches Sound Select) — an unstyled
        // dropdown renders invisible on the transparent action row.
        lv_obj_add_style(fIrDropdown, &fStyles->compactSelectMain, LV_PART_MAIN);
        lv_obj_add_style(fIrDropdown, &fStyles->compactSelectIndicator, LV_PART_INDICATOR);
        // Long option lists must scroll (sanctioned exemption); panels stay locked.
        uiEnableDropdownListScroll(fIrDropdown);
        uiFixDropdownPopup(fIrDropdown);
        lv_dropdown_set_options(fIrDropdown, "0.5s\n1.0s\n2.0s\n3.0s\n5.0s");
        // Processing-mode toggle: its own slim row (the grouped action row is
        // at column width with SAVE + IR controls).
        if (fWidgets && fLabels && fParams && fValueCb && fUi) {
            lv_obj_t* modeRow = lv_obj_create(col);
            lv_obj_set_size(modeRow, lv_pct(100), LV_SIZE_CONTENT);
            lv_obj_set_style_pad_all(modeRow, 0, 0);
            lv_obj_set_style_bg_opa(modeRow, 0, 0);
            lv_obj_set_style_border_width(modeRow, 0, 0);
            lv_obj_set_layout(modeRow, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(modeRow, LV_FLEX_FLOW_ROW);
            lv_obj_clear_flag(modeRow, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_t* modeBtn = lv_btn_create(modeRow);
            lv_obj_set_size(modeBtn, scaled(120), scaled(30));
            lv_obj_add_flag(modeBtn, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_user_data(modeBtn, reinterpret_cast<void*>((intptr_t)PluginCymbals::kParamProcessingMode));
            lv_obj_add_event_cb(modeBtn, fValueCb, LV_EVENT_VALUE_CHANGED, fUi);
            lv_obj_t* modeLbl = lv_label_create(modeBtn);
            lv_label_set_text(modeLbl, fParams[PluginCymbals::kParamProcessingMode] > 0.5f ? "FAST MATH" : "SAFE MATH");
            lv_obj_center(modeLbl);
            fStyles->applyToggleButton(modeBtn, fParams[PluginCymbals::kParamProcessingMode] > 0.5f);
            fWidgets[PluginCymbals::kParamProcessingMode] = modeBtn;
            fLabels[PluginCymbals::kParamProcessingMode] = modeLbl;
        }

        lv_obj_t* irBtn = lv_btn_create(actionRow);
        lv_obj_set_size(irBtn, scaled(92), scaled(30));
        fStyles->applyMomentaryButton(irBtn);
        lv_obj_add_event_cb(irBtn, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (b) b->fHost->requestIrExport(b->selectedIrSeconds());
        }, LV_EVENT_CLICKED, this);
        lv_obj_t* irBtnLbl = lv_label_create(irBtn);
        lv_label_set_text(irBtnLbl, "EXPORT IR");
        lv_obj_set_style_text_font(irBtnLbl, getScaledSmallFont(), 0);
        lv_obj_center(irBtnLbl);

        // --- tonal-profile preview (critic closeout) ---
        // The loaded preset's reference spectral slope: dB against
        // log-frequency, derived from the preset's own metadata (the same
        // profile the per-row thumbnails draw, so a row's glyph is the curve
        // you get when you load it). 0 dB datum + decade grid inside the
        // raster; no preset JSON key or sidecar entry is involved.
        lv_obj_t* profileCapRow = lv_obj_create(col);
        lv_obj_set_size(profileCapRow, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(profileCapRow, 0, 0);
        lv_obj_set_style_bg_opa(profileCapRow, 0, 0);
        lv_obj_set_style_border_width(profileCapRow, 0, 0);
        lv_obj_set_layout(profileCapRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(profileCapRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(profileCapRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(profileCapRow, scaled(6), 0);
        lv_obj_clear_flag(profileCapRow, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* profileCap = lv_label_create(profileCapRow);
        lv_label_set_text(profileCap, "TONAL PROFILE");
        lv_obj_add_style(profileCap, &fStyles->title, 0);
        lv_obj_set_style_text_font(profileCap, getScaledSmallFont(), 0);

        fProfileCharLbl = lv_label_create(profileCapRow);
        lv_label_set_text(fProfileCharLbl, "UNTAGGED");
        lv_obj_set_style_text_font(fProfileCharLbl, uiMicroFont(), 0);
        lv_obj_set_style_text_color(fProfileCharLbl, COL_TEXT_DIM, 0);
        lv_obj_set_style_text_letter_space(fProfileCharLbl, scaled(1), 0);
        lv_obj_set_flex_grow(fProfileCharLbl, 1);
        lv_obj_set_width(fProfileCharLbl, 0);
        lv_obj_set_style_text_align(fProfileCharLbl, LV_TEXT_ALIGN_RIGHT, 0);

        fProfileCanvas = lv_canvas_create(col);
        lv_canvas_set_buffer(fProfileCanvas, PresetRowGlyphs::sharedPlotRaster().px,
                             PresetRowGlyphs::kPlotW, PresetRowGlyphs::kPlotH,
                             LV_COLOR_FORMAT_ARGB8888);
        lv_obj_set_size(fProfileCanvas, scaled(PresetRowGlyphs::kPlotW),
                        scaled(PresetRowGlyphs::kPlotH));
        lv_obj_set_style_radius(fProfileCanvas, scaled(3), 0);
        lv_obj_set_style_border_width(fProfileCanvas, 1, 0);
        lv_obj_set_style_border_color(fProfileCanvas, COL_BORDER, 0);
        lv_obj_clear_flag(fProfileCanvas, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(fProfileCanvas, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(fProfileCanvas, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t* profileScaleRow = lv_obj_create(col);
        lv_obj_set_size(profileScaleRow, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(profileScaleRow, 0, 0);
        lv_obj_set_style_bg_opa(profileScaleRow, 0, 0);
        lv_obj_set_style_border_width(profileScaleRow, 0, 0);
        lv_obj_set_layout(profileScaleRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(profileScaleRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(profileScaleRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(profileScaleRow, scaled(6), 0);
        lv_obj_clear_flag(profileScaleRow, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_t* hzLo = lv_label_create(profileScaleRow);
        lv_label_set_text(hzLo, "48 HZ");
        lv_obj_t* hzHi = lv_label_create(profileScaleRow);
        lv_label_set_text(hzHi, "15 KHZ");
        for (lv_obj_t* l : {hzLo, hzHi}) {
            lv_obj_set_style_text_font(l, uiMicroFont(), 0);
            lv_obj_set_style_text_color(l, COL_TEXT_DIM, 0);
        }
        lv_obj_set_flex_grow(hzLo, 1);
        lv_obj_set_width(hzLo, 0);
        lv_obj_set_style_text_align(hzHi, LV_TEXT_ALIGN_RIGHT, 0);

        // --- library mini-card (critic closeout) ---
        // The space beside the Ready line used to be empty. One 16px chip per
        // bank family with the preset count, plus a total, so the shape of the
        // library is visible without opening the directory tree.
        fBankChips = lv_obj_create(col);
        lv_obj_set_size(fBankChips, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(fBankChips, 0, 0);
        lv_obj_set_style_bg_opa(fBankChips, 0, 0);
        lv_obj_set_style_border_width(fBankChips, 0, 0);
        lv_obj_set_layout(fBankChips, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(fBankChips, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(fBankChips, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(fBankChips, scaled(4), 0);
        lv_obj_clear_flag(fBankChips, LV_OBJ_FLAG_SCROLLABLE);

        fStatusLabel = lv_label_create(col);
        lv_label_set_text(fStatusLabel, "Ready");
        lv_obj_set_style_text_color(fStatusLabel, COL_TEXT_DIM, 0);
        lv_obj_set_style_text_font(fStatusLabel, getScaledSmallFont(), 0);
        lv_label_set_long_mode(fStatusLabel, LV_LABEL_LONG_DOT);
        lv_obj_set_width(fStatusLabel, lv_pct(100));
    }

    // --- tree ---
    void rebuildTree() {
        if (!fTree) return;
        lv_obj_clean(fTree);
        fNodes.clear();

        // Smart nodes pinned above the folders.
        addTreeNode(true, 0, "", "All Presets", 0);
        addTreeNode(true, 1, "", "Favourites (" + std::to_string(fFavourites.size()) + ")", 0);
        addTreeNode(true, 2, "", "Tags", 0);

        // Collect every folder prefix (so deep folders still render at depth 2).
        std::map<std::string, std::string> folderDisplay;  // lowercase -> display
        for (const PresetListEntry& e : fEntries) {
            std::string p = e.info.folder;
            while (!p.empty()) {
                const std::string low = toLower(p);
                if (folderDisplay.find(low) == folderDisplay.end()) folderDisplay[low] = p;
                const size_t slash = p.find('/');
                p = (slash == std::string::npos) ? "" : p.substr(slash + 1);
            }
        }

        // Root nodes: skeleton order first, then extras alphabetically.
        std::vector<std::string> extraRoots;
        for (const auto& kv : folderDisplay) {
            const std::string& disp = kv.second;
            if (disp.find('/') != std::string::npos) continue;
            bool known = false;
            for (const std::string& s : skeletonRoots()) {
                if (toLower(s) == kv.first) { known = true; break; }
            }
            if (!known) extraRoots.push_back(disp);
        }
        std::sort(extraRoots.begin(), extraRoots.end());

        std::vector<std::string> roots = skeletonRoots();
        for (const std::string& r : extraRoots) roots.push_back(r);

        for (const std::string& root : roots) {
            const std::string rootLow = toLower(root);
            // Always show skeleton roots (ensureSkeletonDirs creates them on disk).
            // Only skip non-skeleton roots that have no presets.
            bool isSkeleton = false;
            for (const std::string& s : skeletonRoots()) {
                if (toLower(s) == rootLow) { isSkeleton = true; break; }
            }
            if (!isSkeleton && folderDisplay.find(rootLow) == folderDisplay.end()) continue;
            const int rootNode = addTreeNode(false, 0, root, root, 0);

            // Children of this root (depth-1 nodes). Fold deeper components
            // into the label: x/y/z renders as y (z) under x.
            std::vector<std::string> childDirs;
            std::map<std::string, std::string> childLabel;  // low(second) -> display label
            const std::string prefix = root + "/";
            const std::string prefixLow = rootLow + "/";
            for (const auto& kv : folderDisplay) {
                if (kv.first.rfind(prefixLow, 0) != 0) continue;
                const std::string rest = kv.second.substr(prefix.size());
                const size_t slash = rest.find('/');
                const std::string second = (slash == std::string::npos) ? rest : rest.substr(0, slash);
                const std::string secondLow = toLower(second);
                if (std::find(childDirs.begin(), childDirs.end(), secondLow) == childDirs.end())
                    childDirs.push_back(secondLow);
                std::string label = second;
                if (slash != std::string::npos) {
                    std::string deeper = rest.substr(slash + 1);
                    std::replace(deeper.begin(), deeper.end(), '/', ' ');
                    label = second + " (" + deeper + ")";
                }
                childLabel[secondLow] = label;
            }

            std::vector<std::string> ordered;
            if (rootLow == "cymbals") {
                // Always show cymbals children (skeleton dirs exist on disk).
                for (const std::string& c : cymbalsChildren()) {
                    const std::string cl = toLower(c);
                    if (std::find(ordered.begin(), ordered.end(), cl) == ordered.end())
                        ordered.push_back(cl);
                }
                std::vector<std::string> extra;
                for (const std::string& c : childDirs) {
                    if (std::find(ordered.begin(), ordered.end(), c) == ordered.end()) extra.push_back(c);
                }
                std::sort(extra.begin(), extra.end());
                for (const std::string& c : extra) ordered.push_back(c);
            } else {
                ordered = childDirs;
                std::sort(ordered.begin(), ordered.end());
            }

            for (const std::string& child : ordered) {
                const std::string rel = root + "/" + child;
                addTreeNode(false, 0, rel, childLabel.count(child) ? childLabel[child] : child, 1);
                (void)rootNode;
            }
        }
    }

    int addTreeNode(bool smart, int smartKind, const std::string& relPath,
                    const std::string& label, int depth) {
        TreeNode node;
        node.smart = smart;
        node.smartKind = smartKind;
        node.relPath = relPath;
        node.label = label;
        node.depth = depth;

        lv_obj_t* btn = lv_btn_create(fTree);
        lv_obj_set_size(btn, lv_pct(100), scaled(26));
        fStyles->applyMomentaryButton(btn);
        // Keep the scroll container from taking the press before the node's
        // own PRESSING handler has a chance to arm the folder drag.
        lv_obj_add_flag(btn, LV_OBJ_FLAG_PRESS_LOCK);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_set_style_pad_left(btn, scaled(8 + 14 * depth), 0);
        lv_obj_set_style_radius(btn, scaled(3), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_10, 0);
        lv_obj_set_style_border_opa(btn, LV_OPA_0, 0);
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, label.c_str());
        lv_obj_set_style_text_font(lbl, getScaledSmallFont(), 0);
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_style_text_color(lbl, smart ? COL_HIGHLIGHT : COL_TEXT, 0);
        lv_obj_center(lbl);
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_style_pad_left(lbl, scaled(8), 0);
        lv_obj_set_width(lbl, lv_pct(100));

        const int idx = (int)fNodes.size();
        lv_obj_set_user_data(btn, (void*)(intptr_t)idx);
        lv_obj_add_event_cb(btn, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (!b) return;
            lv_obj_t* tgt = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
            const int idx = (int)(intptr_t)lv_obj_get_user_data(tgt);
            if (idx >= 0 && idx < (int)b->fNodes.size()) b->selectNode(idx);
        }, LV_EVENT_CLICKED, this);

        // Drag'n'drop folder copy (step 8). LVGL 9 has no DRAGGABLE flag or
        // DRAG_BEGIN event: a drag is a press that moves beyond a threshold
        // (same pattern as the comb editor's dragPanelByHeaderCb). The source
        // node stays put and dims; the pointer position drives the highlight.
        lv_obj_add_event_cb(btn, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (b) b->onNodePressed(e);
        }, LV_EVENT_PRESSED, this);
        lv_obj_add_event_cb(btn, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (b) b->onNodePressing(e);
        }, LV_EVENT_PRESSING, this);
        lv_obj_add_event_cb(btn, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (b) b->onDragRelease(e);
        }, LV_EVENT_RELEASED, this);
        lv_obj_add_event_cb(btn, [](lv_event_t* e) {
            PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
            if (b) b->onDragRelease(e);
        }, LV_EVENT_PRESS_LOST, this);

        node.btn = btn;
        fNodes.push_back(node);
        return idx;
    }

    void selectNode(int idx) {
        if (idx < 0 || idx >= (int)fNodes.size()) return;
        const TreeNode& node = fNodes[(size_t)idx];
        if (node.smart) {
            if (node.smartKind == 0) {
                fView.kind = ViewKind::All;
                fView.folder.clear();
            } else if (node.smartKind == 1) {
                fView.kind = ViewKind::Favourites;
                fView.folder.clear();
            } else {
                openTagPicker();
                return;
            }
        } else {
            fView.kind = ViewKind::Folder;
            fView.folder = node.relPath;
        }
        if (fFolderLabel) {
            std::string txt = (fView.kind == ViewKind::Folder && !fView.folder.empty())
                ? fView.folder : "";
            lv_label_set_text(fFolderLabel, txt.c_str());
        }
        rebuildList();
    }

    // Live-typing search cap: keeps the LVGL list rebuild instant even on
    // a large preset directory. The 200 figure is from the round-2 brief
    // (piece 3). All matching entries past the cap are NOT shown in the
    // list, but the full-text search still considers them when navStep()
    // walks the filtered set.
    static constexpr size_t kMaxFilterResults = 200;
    // Colour a row star applies when a preset is first favourited from the
    // list. kFavColours[2] == "yellow" (the sidecar's default), so a star
    // toggled on from a row reads as "yours" without extra clicks.
    static constexpr int kFavDefaultColour = 2;
    // --- filtering + list ---
    bool entryMatches(const PresetListEntry& e, const ViewState& view,
                      const std::vector<QueryToken>& tokens) const {
        if (view.kind == ViewKind::Folder) {
            const std::string& f = view.folder;
            if (!f.empty()) {
                // Case-insensitive: tree relPaths come from the skeleton spellings
                // while info.folder is the on-disk case (std::filesystem::relative).
                const std::string ef = toLower(e.info.folder);
                const std::string vf = toLower(f);
                if (ef != vf && ef.rfind(vf + "/", 0) != 0) return false;
            }
        } else if (view.kind == ViewKind::Favourites) {
            if (fFavourites.find(e.relKey) == fFavourites.end()) return false;
        }
        if (!tokens.empty()) {
            PresetSearchFields fields;
            fields.name = e.info.name;
            fields.description = e.info.description;
            fields.usage = e.info.usage;
            fields.author = e.info.author;
            fields.tags = e.info.tags;
            if (!queryMatches(fields, tokens)) return false;
        }
        return true;
    }

    // Build a PresetSearchFields from a list entry (cheap, returns by value).
    PresetSearchFields entryFields(const PresetListEntry& e) const {
        PresetSearchFields f;
        f.name = e.info.name;
        f.description = e.info.description;
        f.usage = e.info.usage;
        f.author = e.info.author;
        f.tags = e.info.tags;
        return f;
    }

    // Per-entry live-typing predicate (the round-2 brief API). Fast
    // case-insensitive substring match against name/desc/use/author/tags.
    // Out-of-range index returns false.
    bool matchesPreset(int idx, const char* query) const {
        if (idx < 0 || (size_t)idx >= fEntries.size()) return false;
        return PresetBrowserLogic::matchesPreset(entryFields(fEntries[(size_t)idx]), query);
    }

    // Live-typing filter: walks the visible-list source (which honours the
    // current folder/favourites view) and writes matching indices into
    // fFilteredEntries, capped at kMaxFilterResults. fNavPaths is filled
    // with the SAME surviving indices (so navStep still walks the visible
    // set, not the full unfiltered directory).
    //
    // Empty/whitespace-only query => match everything (no cap harm — the
    // cap only bites when the user has typed something that matches many
    // entries).
    void applyFilter(const char* query) {
        fFilteredEntries.clear();
        fNavPaths.clear();
        fNavIndex = -1;

        // Build the source index list (respects the current view).
        std::vector<size_t> source;
        source.reserve(fEntries.size());
        for (size_t i = 0; i < fEntries.size(); ++i) {
            // View filter: folder/favourites pre-filter, query handled below.
            PresetSearchFields ignored;
            (void)ignored;
            // We re-use entryMatches's view-only branch by passing an empty
            // query, which makes the token-based check short-circuit true.
            if (entryMatches(fEntries[i], fView, /*tokens*/ {})) {
                source.push_back(i);
            }
        }

        if (!query || !*query) {
            // No query => show everything from source, no cap.
            fFilteredEntries = std::move(source);
        } else {
            // Build fields only for the source window (cheap, capped).
            std::vector<PresetSearchFields> sourceFields;
            sourceFields.reserve(source.size());
            for (size_t idx : source) sourceFields.push_back(entryFields(fEntries[idx]));
            PresetBrowserLogic::applyFilter(sourceFields, query, source, fFilteredEntries, kMaxFilterResults);
        }

        for (size_t idx : fFilteredEntries) {
            fNavPaths.push_back(fEntries[idx].info.path);
        }
    }

    void rebuildList() {
        if (!fList) return;
        lv_obj_clean(fList);
        // The list cleanup deletes every row canvas. Drop the thumbnail queue
        // in the SAME step, before any early return (an empty filter result
        // used to return with the old pointers still queued, and the thumbnail
        // timer then called lv_canvas_get_buf() on freed objects).
        fRowThumbs.clear();
        fNextThumb = 0;
        fFilteredEntries.clear();
        fNavPaths.clear();
        fNavIndex = -1;

        // When a search query is active, the live-typing path is the
        // source of truth (parses tokens, applies the 200-result cap, and
        // honours the current view). Otherwise we keep the legacy
        // full-scan that just calls entryMatches() per entry.
        if (!fQuery.empty()) {
            std::string q = joinQueryTokens(fQuery);
            applyFilter(q.c_str());
        } else {
            for (size_t i = 0; i < fEntries.size(); ++i) {
                if (entryMatches(fEntries[i], fView, fQuery)) {
                    fFilteredEntries.push_back(i);
                    fNavPaths.push_back(fEntries[i].info.path);
                }
            }
        }

        // Round-5: type filter (applied after view+query filter)
        if (fTypeFilter > 0) {
            std::vector<size_t> filtered;
            filtered.reserve(fFilteredEntries.size());
            for (size_t idx : fFilteredEntries) {
                const PresetBrowserLogic::PresetKind kind =
                    PresetBrowserLogic::inferPresetKind(fEntries[idx].info.name, fEntries[idx].info.folder);
                bool match = false;
                switch (fTypeFilter) {
                    case 1: match = (kind == PresetBrowserLogic::PresetKind::Cymbal); break;
                    case 2: match = (kind == PresetBrowserLogic::PresetKind::Percussion); break;
                    case 3: match = (fEntries[idx].info.folder.find("effects") != std::string::npos); break;
                    case 4: match = (kind == PresetBrowserLogic::PresetKind::Crash); break;
                    case 5: match = (kind == PresetBrowserLogic::PresetKind::Ride); break;
                    case 6: match = (kind == PresetBrowserLogic::PresetKind::Hat); break;
                    default: match = true; break;
                }
                if (match) filtered.push_back(idx);
            }
            fFilteredEntries = std::move(filtered);
        }

        // Round-5: sort order
        if (fSortOrder == 0) {
            // Name (case-insensitive)
            std::sort(fFilteredEntries.begin(), fFilteredEntries.end(),
                [this](size_t a, size_t b) {
                    return toLower(fEntries[a].info.name) < toLower(fEntries[b].info.name);
                });
        } else if (fSortOrder == 1) {
            // Date (newest first)
            std::sort(fFilteredEntries.begin(), fFilteredEntries.end(),
                [this](size_t a, size_t b) {
                    return fEntries[a].mtime > fEntries[b].mtime;
                });
        } else {
            // Recent (same as Date for now — mtime is the only signal)
            std::sort(fFilteredEntries.begin(), fFilteredEntries.end(),
                [this](size_t a, size_t b) {
                    return fEntries[a].mtime > fEntries[b].mtime;
                });
        }

        // The header arrows must walk exactly what the list shows: rebuild
        // fNavPaths from the FINAL (filtered + sorted) indices. The earlier
        // fill happened before the type filter/sort, so it could contain
        // hidden entries and the wrong order.
        fNavPaths.clear();
        for (size_t idx : fFilteredEntries) fNavPaths.push_back(fEntries[idx].info.path);

        // Round-5: update count chip + tag chips
        updateCountChip();
        rebuildTagChips();
        // Round-6: the legend lives in the footer band under the list, so it
        // sits in the same place whatever the library size. The band is a fixed
        // 56px strip and the label WRAPs: it holds three lines of the micro
        // face (11px line height each) inside its 6px pads. A fourth line would
        // clip at the panel bottom edge, so the copy is three lines and no more.
        if (fSpaceLegend) {
            std::string legend;
            char total[32];
            std::snprintf(total, sizeof(total), "%zu presets", fEntries.size());
            legend += total;
            if (!fFavourites.empty()) {
                legend += "  -  " + std::to_string(fFavourites.size()) + " favourited";
            }
            legend += "  -  a word matches name, description, usage, author and tags";
            legend += "\nLeft: a folder or a smart view.  Drag a folder onto another to copy its presets.";
            lv_label_set_text(fSpaceLegend, legend.c_str());
        }
        // Empty-state message + cap-overflow hint.
        if (fFilteredEntries.empty()) {
            lv_obj_t* emptyLbl = lv_label_create(fList);
            const char* msg = "No presets in this folder.";
            if (fView.kind == ViewKind::Favourites) msg = "No favourites yet - tap a star on any preset.";
            else if (!fQuery.empty()) msg = "No presets match this search.";
            lv_label_set_text(emptyLbl, msg);
            lv_obj_set_style_text_color(emptyLbl, COL_TEXT_DIM, 0);
            return;
        }
        if (fFilteredEntries.size() >= kMaxFilterResults) {
            char buf[96];
            std::snprintf(buf, sizeof(buf), "Showing %zu of %zu+ (cap %zu) - refine search.",
                          fFilteredEntries.size(), fEntries.size(), kMaxFilterResults);
            lv_obj_t* hintLbl = lv_label_create(fList);
            lv_label_set_text(hintLbl, buf);
            lv_obj_set_style_text_color(hintLbl, COL_TEXT_DIM, 0);
            lv_obj_set_style_text_font(hintLbl, getScaledSmallFont(), 0);
        }

        for (size_t k = 0; k < fFilteredEntries.size(); ++k) {
            const PresetListEntry& e = fEntries[fFilteredEntries[k]];
            const int favSlot = [&]() {
                const auto it = fFavourites.find(e.relKey);
                return (it != fFavourites.end()) ? it->second : (int)PresetRowGlyphs::kStarSlots - 1;
            }();

            lv_obj_t* card = lv_btn_create(fList);
            lv_obj_set_size(card, lv_pct(100), scaled(56));
            fStyles->applyMomentaryButton(card);
            lv_obj_set_style_pad_all(card, scaled(3), 0);
            lv_obj_set_style_pad_hor(card, scaled(8), 0);
            lv_obj_set_style_radius(card, scaled(4), 0);
            lv_obj_set_layout(card, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_style_pad_column(card, scaled(6), 0);
            lv_obj_set_user_data(card, (void*)(intptr_t)k);
            lv_obj_add_event_cb(card, [](lv_event_t* e) {
                PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
                if (!b) return;
                lv_obj_t* tgt = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
                const int k = (int)(intptr_t)lv_obj_get_user_data(tgt);
                if (k < 0 || (size_t)k >= b->fFilteredEntries.size()) return;
                b->fHost->requestPresetLoad(b->fEntries[b->fFilteredEntries[(size_t)k]].info.path);
            }, LV_EVENT_CLICKED, this);

            // Favourite STAR (toggle): filled in the preset's favourite colour,
            // or a dim chrome star when not favourited. Clicking toggles the
            // favourites sidecar entry; it must not fall through to the card's
            // load handler.
            lv_obj_t* starBox = PresetRowGlyphs::createRowStar(card, favSlot);
            lv_obj_set_user_data(starBox, (void*)(intptr_t)k);
            lv_obj_add_event_cb(starBox, [](lv_event_t* e) {
                PresetBrowser* b = static_cast<PresetBrowser*>(lv_event_get_user_data(e));
                if (!b) return;
                lv_obj_t* tgt = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
                const int k = (int)(intptr_t)lv_obj_get_user_data(tgt);
                if (k < 0 || (size_t)k >= b->fFilteredEntries.size()) return;
                b->toggleFavourite(b->fFilteredEntries[(size_t)k]);
                lv_event_stop_bubbling(e);
            }, LV_EVENT_CLICKED, this);

            // Type (category) badge: fixed-width lane-coloured glyph column so
            // a row's family reads without opening it.
            const PresetBrowserLogic::PresetKind kind =
                PresetBrowserLogic::inferPresetKind(e.info.name, e.info.folder);
            const PresetBrowserLogic::PresetKind bankKind =
                PresetBrowserLogic::kindFromBank(e.info.folder);
            PresetRowGlyphs::createRowKindBadge(card, kind);

            // Text stack: name + bank chip on the top line, author + tag pills
            // on the second. Explicit height (a nested flex COLUMN's auto
            // height mis-measures inside a flex ROW: LVGL sized it 130px and
            // cross-centred it off the card). 15px name line + 2px gap + 13px
            // meta line = 30, +2 slack below inside the 56px card.
            lv_obj_t* textStack = lv_obj_create(card);
            lv_obj_set_size(textStack, 0, scaled(40));
            lv_obj_set_flex_grow(textStack, 1);
            lv_obj_set_style_bg_opa(textStack, 0, 0);
            lv_obj_set_style_border_width(textStack, 0, 0);
            lv_obj_set_style_pad_all(textStack, 0, 0);
            lv_obj_set_style_pad_row(textStack, scaled(3), 0);
            lv_obj_set_layout(textStack, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(textStack, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(textStack, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
            lv_obj_clear_flag(textStack, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_scrollbar_mode(textStack, LV_SCROLLBAR_MODE_OFF);

            lv_obj_t* headRow = lv_obj_create(textStack);
            lv_obj_set_size(headRow, lv_pct(100), LV_SIZE_CONTENT);
            lv_obj_set_style_bg_opa(headRow, 0, 0);
            lv_obj_set_style_border_width(headRow, 0, 0);
            lv_obj_set_style_pad_all(headRow, 0, 0);
            lv_obj_set_style_pad_column(headRow, scaled(8), 0);
            lv_obj_set_layout(headRow, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(headRow, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(headRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(headRow, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_scrollbar_mode(headRow, LV_SCROLLBAR_MODE_OFF);

            lv_obj_t* nameLbl = lv_label_create(headRow);
            lv_label_set_text(nameLbl, e.info.name.c_str());
            lv_obj_add_style(nameLbl, &fStyles->title, 0);
            lv_obj_set_style_text_font(nameLbl, getScaledFont(), 0);
            lv_label_set_long_mode(nameLbl, LV_LABEL_LONG_DOT);
            lv_obj_set_flex_grow(nameLbl, 1);
            lv_obj_set_width(nameLbl, 0);

            // Bank (type) chip: right-aligned, lane-coloured.
            const std::string bank = PresetRowGlyphs::bankLabelFor(e.info.folder);
            lv_obj_t* bankPill = PresetRowGlyphs::createRowPill(
                headRow, bank.c_str(), PresetRowGlyphs::bankColourFor(e.info.folder));
            if (e.info.folder.empty())
                uiAttachTooltip(bankPill,
                    "ROOT: this preset is stored at the preset library root, not in a bank folder");

            // Classification-source chip: OVR / FAV only (nullptr for plain
            // bank-derived rows — the bank pill already carries the folder).
            const char* srcChip = PresetRowGlyphs::sourceChipText(
                kind, bankKind, fFavourites.find(e.relKey) != fFavourites.end());
            if (srcChip) {
                PresetRowGlyphs::createRowPill(headRow, srcChip,
                                               PresetRowGlyphs::sourceChipColour(srcChip));
            }

            lv_obj_t* metaRow = lv_obj_create(textStack);
            lv_obj_set_size(metaRow, lv_pct(100), LV_SIZE_CONTENT);
            lv_obj_set_style_bg_opa(metaRow, 0, 0);
            lv_obj_set_style_border_width(metaRow, 0, 0);
            lv_obj_set_style_pad_all(metaRow, 0, 0);
            lv_obj_set_style_pad_column(metaRow, scaled(4), 0);
            lv_obj_set_layout(metaRow, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(metaRow, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(metaRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(metaRow, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_scrollbar_mode(metaRow, LV_SCROLLBAR_MODE_OFF);

            // Designer credit (grows, ellipsised against the pills that
            // follow). A real author reads in the standard text tier; only the
            // missing-author placeholder stays dim.
            lv_obj_t* authorLbl = lv_label_create(metaRow);
            lv_label_set_text(authorLbl, e.info.author.empty() ? "UNKNOWN AUTHOR" : e.info.author.c_str());
            lv_obj_set_style_text_font(authorLbl, getScaledSmallFont(), 0);
            lv_obj_set_style_text_color(authorLbl, e.info.author.empty() ? COL_TEXT_DIM : COL_KNOB_LABEL, 0);
            lv_obj_set_style_text_letter_space(authorLbl, scaled(1), 0);
            lv_label_set_long_mode(authorLbl, LV_LABEL_LONG_DOT);
            lv_obj_set_flex_grow(authorLbl, 1);
            lv_obj_set_width(authorLbl, 0);

            // Tag pills (at-a-glance classification): first two tags, then a
            // "+N" overflow chip.
            const size_t kMaxTagPills = 2;
            for (size_t t = 0; t < e.info.tags.size() && t < kMaxTagPills; ++t) {
                PresetRowGlyphs::createRowPill(metaRow, e.info.tags[t].c_str(), COL_ACCENT);
            }
            if (e.info.tags.size() > kMaxTagPills) {
                PresetRowGlyphs::createRowPill(
                    metaRow, ("+" + std::to_string(e.info.tags.size() - kMaxTagPills)).c_str(),
                    COL_ACCENT);
            }

            // Preview thumbnail: the metadata estimate is the instant
            // placeholder; the timer upgrades it to the preset's REAL IR
            // spectrum (same curve the PRESET INFO plot draws) once the
            // background-free one-per-tick renderer has produced it.
            const lv_color_t thumbColour = PresetRowGlyphs::kindColourFor(kind);
            lv_obj_t* thumb = PresetRowGlyphs::createRowCurve(
                card, PresetBrowserLogic::profileFromMetadata(
                          {e.info.name, e.info.description, e.info.usage, e.tagsCsv}),
                thumbColour);
            std::vector<float> cached;
            if (fHost && fHost->getCachedPresetProfile(e.info.path, cached) && cached.size() >= 2) {
                PresetRowGlyphs::redrawRowCurve(thumb, cached, thumbColour);
            } else {
                fRowThumbs.push_back({e.info.path, thumb, thumbColour});
            }

            // The whole card loads the preset: child containers/pills/labels
            // must not swallow the press, only the star keeps its own click.
            PresetRowGlyphs::makeRowClickThrough(card, starBox);
        }

    }

    // --- favourites ---
    // Row STAR toggle: absent -> favourited in the default (yellow) colour;
    // present -> removed. Colour choice stays in the PRESET INFO swatches for
    // the loaded preset; the sidecar format (favourites.json) is unchanged.
    void toggleFavourite(size_t entryIdx) {
        if (entryIdx >= fEntries.size()) return;
        const std::string key = fEntries[entryIdx].relKey;
        if (key.empty()) return;
        const auto it = fFavourites.find(key);
        if (it != fFavourites.end()) {
            fFavourites.erase(it);
        } else {
            fFavourites[key] = kFavDefaultColour;
        }
        writeFavourites(fPresetDir, fFavourites);
        reloadFavouritesAndRebuild();
    }

    void setLoadedFavourite(int colourIdx, bool enable) {
        if (fLastLoadedPath.empty()) {
            setStatus("Load a preset first", true);
            return;
        }
        const std::string key = relKey(fPresetDir, fLastLoadedPath);
        if (!enable || colourIdx < 0) fFavourites.erase(key);
        else fFavourites[key] = colourIdx;
        writeFavourites(fPresetDir, fFavourites);
        reloadFavouritesAndRebuild();
    }

    void syncFavouriteSwatches() {
        if (!fAttached) return;
        int active = -1;
        if (!fLastLoadedPath.empty()) {
            const auto it = fFavourites.find(relKey(fPresetDir, fLastLoadedPath));
            if (it != fFavourites.end()) active = it->second;
        }
        for (int i = 0; i < 6; ++i) {
            if (!fSwatches[i]) continue;
            const bool on = (i == active);
            lv_obj_set_style_border_width(fSwatches[i], on ? 2 : 1, 0);
            lv_obj_set_style_border_color(fSwatches[i], on ? COL_HIGHLIGHT : COL_HAIRLINE, 0);
        }
    }

    // --- save ---
    void saveFromFields() {
        if (!fHost) return;
        const std::string name = fNameArea ? lv_textarea_get_text(fNameArea) : "";
        const std::string author = fAuthorArea ? lv_textarea_get_text(fAuthorArea) : "";
        const std::string desc = fDescArea ? lv_textarea_get_text(fDescArea) : "";
        const std::string usage = fUsageArea ? lv_textarea_get_text(fUsageArea) : "";
        commitTagInput();  // a half-typed tag still saves with the preset
        const std::string tags = tagsCsv();
        fHost->setPresetMetadata(name, desc, author, tags, usage);

        // Save into the currently selected folder: pass an ABSOLUTE folder path
        // (PresetPolicy::canonicalizePresetSavePath uses the requested path's
        // parent as the directory and never joins the preset root, so a relative
        // "basses/" would resolve against the host CWD).
        std::string requested;
        if (fView.kind == ViewKind::Folder && !fView.folder.empty()) {
            requested = (std::filesystem::u8path(fPresetDir) / std::filesystem::u8path(fView.folder)).u8string();
        }
        std::string err;
        if (!fHost->savePreset(requested, err)) {
            setStatus(err, true);
        } else {
            setStatus("Saved", false);
        }
        refresh(true);
    }

    float selectedIrSeconds() {
        switch (fIrDropdown ? (int)lv_dropdown_get_selected(fIrDropdown) : 2) {
            case 0: return 0.5f;
            case 1: return 1.0f;
            case 2: return 2.0f;
            case 3: return 3.0f;
            default: return 5.0f;
        }
    }

    // --- tag picker modal (overlay on the top layer, UIPluginPicker pattern) ---
    void openTagPicker() {
        if (!fAttached) return;
        struct State {
            lv_obj_t* overlay = nullptr;
            PresetBrowser* browser = nullptr;
            std::vector<std::string> tags;
            std::vector<bool> selected;
            std::vector<lv_obj_t*> chips;
            lv_obj_t* countLbl = nullptr;
            lv_obj_t* matchLbl = nullptr;
            void refreshMatches() {
                if (!matchLbl || !browser) return;
                std::vector<QueryToken> toks;
                for (size_t i = 0; i < tags.size() && i < selected.size(); ++i) {
                    if (!selected[i]) continue;
                    QueryToken t;
                    t.isTag = true;
                    t.text = toLower(tags[i]);
                    toks.push_back(t);
                }
                size_t matches = 0;
                for (const PresetListEntry& e : browser->fEntries) {
                    PresetSearchFields f;
                    f.name = e.info.name;
                    f.description = e.info.description;
                    f.usage = e.info.usage;
                    f.author = e.info.author;
                    f.tags = e.info.tags;
                    if (queryMatches(f, toks)) ++matches;
                }
                char buf[40];
                std::snprintf(buf, sizeof(buf), "%zu OF %zu MATCH", matches, browser->fEntries.size());
                lv_label_set_text(matchLbl, buf);
                lv_obj_set_style_text_color(matchLbl, matches ? COL_TEXT : COL_TEXT_DIM, 0);
            }
            void refreshCount() {
                if (!countLbl) return;
                int n = 0;
                for (bool on : selected) n += on ? 1 : 0;
                char buf[24];
                std::snprintf(buf, sizeof(buf), "%d SELECTED", n);
                lv_label_set_text(countLbl, buf);
                lv_obj_set_style_text_color(countLbl, n ? COL_HIGHLIGHT : COL_TEXT_DIM, 0);
                refreshMatches();
            }
        };
        State* st = new State();
        st->browser = this;

        // Unique tags + how many presets carry each (case-insensitive merge).
        std::vector<std::pair<std::string, int>> tagCounts;
        for (const PresetListEntry& e : fEntries) {
            std::vector<std::string> seen;
            for (const std::string& raw : e.info.tags) {
                if (raw.empty()) continue;
                const std::string folded = toLower(raw);
                if (std::find(seen.begin(), seen.end(), folded) != seen.end()) continue;
                seen.push_back(folded);
                auto it = std::find_if(tagCounts.begin(), tagCounts.end(),
                    [&](const std::pair<std::string, int>& tc) { return toLower(tc.first) == folded; });
                if (it == tagCounts.end()) tagCounts.push_back({raw, 1});
                else ++it->second;
            }
        }
        std::sort(tagCounts.begin(), tagCounts.end(),
            [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                return toLower(a.first) < toLower(b.first);
            });
        for (const auto& tc : tagCounts) {
            st->tags.push_back(tc.first);
            bool on = false;
            for (const QueryToken& tok : fQuery)
                if (tok.isTag && tok.text == toLower(tc.first)) { on = true; break; }
            st->selected.push_back(on);
        }

        lv_obj_t* host = uiOverlayRootFor(fPanel);
        if (host == nullptr) host = uiLayerTopFor(fPanel);
        lv_obj_t* overlay = lv_obj_create(host);
        // Explicit host size: percentage sizing resolves only on the next
        // layout, and the modal is centered against it immediately.
        const lv_coord_t hostW = lv_obj_get_width(host);
        const lv_coord_t hostH = lv_obj_get_height(host);
        lv_obj_set_size(overlay, hostW > 0 ? hostW : lv_pct(100),
                        hostH > 0 ? hostH : lv_pct(100));
        // The content root is a FLEX container: without FLOATING the overlay
        // is placed in flow after the stage panels (pushed off-screen).
        lv_obj_add_flag(overlay, LV_OBJ_FLAG_FLOATING);
        lv_obj_set_pos(overlay, 0, 0);
        lv_obj_set_style_bg_color(overlay, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(overlay, LV_OPA_60, 0);
        lv_obj_set_style_pad_all(overlay, 0, 0);
        lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(overlay, [](lv_event_t* e) {
            State* s = static_cast<State*>(lv_event_get_user_data(e));
            lv_obj_t* tgt = static_cast<lv_obj_t*>(lv_event_get_target(e));
            if (s && s->overlay == tgt) lv_obj_delete_async(s->overlay);
        }, LV_EVENT_CLICKED, st);
        lv_obj_add_event_cb(overlay, [](lv_event_t* e) {
            State* s = static_cast<State*>(lv_event_get_user_data(e));
            delete s;
        }, LV_EVENT_DELETE, st);
        st->overlay = overlay;
        lv_obj_move_foreground(overlay);


        const lv_coord_t kW = scaled(560);
        const lv_coord_t kH = scaled(430);
        lv_obj_t* modal = lv_obj_create(overlay);
        // Content-height, capped: a handful of tags gives a compact dialog,
        // a large library scrolls inside the chip well instead of leaving a
        // tall empty panel.
        lv_obj_set_size(modal, kW, LV_SIZE_CONTENT);
        lv_obj_set_style_max_height(modal, kH, 0);
        lv_obj_center(modal);
        lv_obj_add_style(modal, &fStyles->panel, 0);
        lv_obj_set_style_pad_all(modal, scaled(14), 0);
        lv_obj_set_style_pad_row(modal, scaled(8), 0);
        lv_obj_set_layout(modal, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(modal, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(modal, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_clear_flag(modal, LV_OBJ_FLAG_SCROLLABLE);

        // Header: title + live selection count + close X.
        lv_obj_t* head = lv_obj_create(modal);
        lv_obj_set_size(head, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(head, 0, 0);
        lv_obj_set_style_bg_opa(head, 0, 0);
        lv_obj_set_style_border_width(head, 0, 0);
        lv_obj_set_style_pad_column(head, scaled(8), 0);
        lv_obj_set_layout(head, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(head, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(head, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(head, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* title = lv_label_create(head);
        lv_label_set_text(title, "TAG FILTER");
        lv_obj_add_style(title, &fStyles->title, 0);
        lv_obj_set_style_text_font(title, getScaledFont(), 0);
        lv_obj_set_flex_grow(title, 1);

        st->countLbl = lv_label_create(head);
        lv_obj_set_style_text_font(st->countLbl, getScaledSmallFont(), 0);
        lv_obj_set_style_text_color(st->countLbl, COL_TEXT_DIM, 0);
        lv_label_set_text(st->countLbl, "0 SELECTED");

        lv_obj_t* closeBtn = lv_btn_create(head);
        lv_obj_set_size(closeBtn, scaled(28), scaled(28));
        fStyles->applyMomentaryButton(closeBtn);
        lv_obj_set_style_pad_all(closeBtn, 0, 0);
        lv_obj_add_event_cb(closeBtn, [](lv_event_t* e) {
            State* s = static_cast<State*>(lv_event_get_user_data(e));
            if (s && s->overlay) lv_obj_delete_async(s->overlay);
        }, LV_EVENT_CLICKED, st);
        lv_obj_t* xLbl = lv_label_create(closeBtn);
        lv_label_set_text(xLbl, "x");
        lv_obj_set_style_text_font(xLbl, getScaledSmallFont(), 0);
        lv_obj_center(xLbl);

        lv_obj_t* sub = lv_label_create(modal);
        lv_label_set_text(sub, "Tap tags to include. A preset must match every selected tag.");
        lv_obj_set_style_text_font(sub, getScaledSmallFont(), 0);
        lv_obj_set_style_text_color(sub, COL_TEXT_DIM, 0);
        lv_label_set_long_mode(sub, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(sub, lv_pct(100));

        // Chip well: recessed, wrapping, scrolls when the library has many tags.
        lv_obj_t* well = lv_obj_create(modal);
        lv_obj_set_size(well, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_min_height(well, scaled(72), 0);
        lv_obj_set_style_max_height(well, scaled(300), 0);
        lv_obj_set_style_bg_color(well, COL_PANEL_DARK, 0);
        lv_obj_set_style_bg_opa(well, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(well, COL_BORDER, 0);
        lv_obj_set_style_border_width(well, 1, 0);
        lv_obj_set_style_radius(well, scaled(5), 0);
        lv_obj_set_style_pad_all(well, scaled(8), 0);
        lv_obj_set_style_pad_gap(well, scaled(6), 0);
        lv_obj_set_layout(well, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(well, LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_flex_align(well, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_scrollbar_mode(well, LV_SCROLLBAR_MODE_AUTO);
        lv_obj_add_flag(well, LV_OBJ_FLAG_SCROLLABLE);
        UIWidgets::attachScrollbarDrag(well);

        if (st->tags.empty()) {
            lv_obj_t* empty = lv_label_create(well);
            lv_label_set_text(empty, "No tags in this library yet.\nAdd tags in PRESET INFO and SAVE.");
            lv_obj_set_style_text_font(empty, getScaledSmallFont(), 0);
            lv_obj_set_style_text_color(empty, COL_TEXT_DIM, 0);
            lv_label_set_long_mode(empty, LV_LABEL_LONG_WRAP);
            lv_obj_set_width(empty, lv_pct(100));
        } else {
            for (size_t i = 0; i < st->tags.size(); ++i) {
                char txt[96];
                std::snprintf(txt, sizeof(txt), "%s (%d)", st->tags[i].c_str(),
                              tagCounts[i].second);
                lv_obj_t* chip = lv_btn_create(well);
                lv_obj_set_size(chip, LV_SIZE_CONTENT, scaled(30));
                lv_obj_set_style_pad_hor(chip, scaled(10), 0);
                lv_obj_set_style_pad_ver(chip, 0, 0);
                fStyles->applyToggleButton(chip, st->selected[i]);
                lv_obj_set_user_data(chip, (void*)(intptr_t)i);
                lv_obj_t* lbl = lv_label_create(chip);
                lv_label_set_text(lbl, txt);
                lv_obj_set_style_text_font(lbl, getScaledSmallFont(), 0);
                lv_obj_center(lbl);
                st->chips.push_back(chip);
                lv_obj_add_event_cb(chip, [](lv_event_t* e) {
                    State* s = static_cast<State*>(lv_event_get_user_data(e));
                    lv_obj_t* tgt = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
                    const size_t idx = (size_t)(intptr_t)lv_obj_get_user_data(tgt);
                    if (!s || idx >= s->selected.size()) return;
                    s->selected[idx] = !s->selected[idx];
                    static_cast<PresetBrowser*>(s->browser)->fStyles->applyToggleButton(
                        tgt, s->selected[idx]);
                    s->refreshCount();
                }, LV_EVENT_CLICKED, st);
            }
        }
        st->refreshCount();

        // Footer: CLEAR (ghost) + APPLY (primary).
        lv_obj_t* foot = lv_obj_create(modal);
        lv_obj_set_size(foot, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(foot, 0, 0);
        lv_obj_set_style_bg_opa(foot, 0, 0);
        lv_obj_set_style_border_width(foot, 0, 0);
        lv_obj_set_style_pad_column(foot, scaled(8), 0);
        lv_obj_set_layout(foot, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(foot, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(foot, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(foot, LV_OBJ_FLAG_SCROLLABLE);

        st->matchLbl = lv_label_create(foot);
        lv_obj_set_style_text_font(st->matchLbl, getScaledSmallFont(), 0);
        lv_obj_set_style_text_color(st->matchLbl, COL_TEXT, 0);
        lv_obj_set_style_text_letter_space(st->matchLbl, scaled(1), 0);
        lv_obj_set_flex_grow(st->matchLbl, 1);
        lv_obj_set_width(st->matchLbl, 0);

        lv_obj_t* clearBtn = lv_btn_create(foot);
        lv_obj_set_size(clearBtn, scaled(92), scaled(32));
        fStyles->applyMomentaryButton(clearBtn);
        lv_obj_add_event_cb(clearBtn, [](lv_event_t* e) {
            State* s = static_cast<State*>(lv_event_get_user_data(e));
            if (!s) return;
            for (size_t i = 0; i < s->selected.size(); ++i) s->selected[i] = false;
            for (lv_obj_t* chip : s->chips) {
                if (chip)
                    static_cast<PresetBrowser*>(s->browser)->fStyles->applyToggleButton(chip, false);
            }
            s->refreshCount();
        }, LV_EVENT_CLICKED, st);
        lv_obj_t* clearLbl = lv_label_create(clearBtn);
        lv_label_set_text(clearLbl, "CLEAR");
        lv_obj_center(clearLbl);

        lv_obj_t* applyBtn = lv_btn_create(foot);
        lv_obj_set_size(applyBtn, scaled(120), scaled(32));
        fStyles->applyMomentaryButton(applyBtn);
        lv_obj_set_style_bg_color(applyBtn, COL_HIGHLIGHT, 0);
        lv_obj_set_style_bg_grad_dir(applyBtn, LV_GRAD_DIR_NONE, 0);
        lv_obj_add_event_cb(applyBtn, [](lv_event_t* e) {
            State* s = static_cast<State*>(lv_event_get_user_data(e));
            if (!s || !s->browser) return;
            PresetBrowser* b = s->browser;
            std::string query;
            for (size_t i = 0; i < s->tags.size(); ++i) {
                if (!s->selected[i]) continue;
                if (!query.empty()) query += " ";
                query += "#";
                // Quote tags containing spaces so parseQuery keeps one token.
                bool needsQuote = s->tags[i].find(' ') != std::string::npos;
                if (needsQuote) query += "\"" + s->tags[i] + "\"";
                else query += s->tags[i];
            }
            if (b->fSearchArea) {
                lv_textarea_set_text(b->fSearchArea, query.c_str());
                b->fQuery = parseQuery(query);
                b->rebuildList();
            }
            lv_obj_delete_async(s->overlay);
        }, LV_EVENT_CLICKED, st);
        lv_obj_t* applyLbl = lv_label_create(applyBtn);
        lv_label_set_text(applyLbl, "APPLY");
        lv_obj_set_style_text_color(applyLbl, COL_BG, 0);
        lv_obj_center(applyLbl);

        st->refreshMatches();
    }

    // --- drag'n'drop folder copy ---
    void onNodePressed(lv_event_t* /*e*/) {
        lv_indev_t* indev = lv_indev_active();
        if (!indev) return;
        lv_indev_get_point(indev, &fDrag.pressPoint);
        // Freeze tree scrolling for the whole gesture. LVGL starts a scroll
        // after its own slop (10 px) and then sends PRESS_LOST to the node,
        // which used to kill the folder drag before onNodePressing could pass
        // its threshold. That is especially easy to hit at gUIScale > 1,
        // where the scaled threshold is > 10 px. The wheel and the explicit
        // scrollbar drag still scroll the tree; only dragging a node is
        // reserved for folder copy.
        if (fTree) lv_obj_clear_flag(fTree, LV_OBJ_FLAG_SCROLLABLE);
    }

    void onNodePressing(lv_event_t* e) {
        if (!fHost) return;
        lv_indev_t* indev = lv_indev_active();
        if (!indev) return;

        if (!fDrag.active) {
            lv_point_t p;
            lv_indev_get_point(indev, &p);
            const lv_coord_t dx = p.x - fDrag.pressPoint.x;
            const lv_coord_t dy = p.y - fDrag.pressPoint.y;
            if (dx * dx + dy * dy < (lv_coord_t)(scaled(8) * scaled(8))) return;

            lv_obj_t* tgt = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
            const int idx = (int)(intptr_t)lv_obj_get_user_data(tgt);
            if (idx < 0 || idx >= (int)fNodes.size()) return;
            const TreeNode& node = fNodes[(size_t)idx];

            fDrag.files.clear();
            if (node.smart) {
                if (node.smartKind == 0) {  // All Presets -> every entry
                    for (const PresetListEntry& en : fEntries) fDrag.files.push_back(en.info.path);
                } else if (node.smartKind == 1) {  // Favourites -> entries with a fav key
                    for (const PresetListEntry& en : fEntries) {
                        if (fFavourites.find(en.relKey) != fFavourites.end()) fDrag.files.push_back(en.info.path);
                    }
                } else {  // Tags -> entries matching current query (no view filter)
                    for (const PresetListEntry& en : fEntries) {
                        if (entryMatches(en, ViewState{}, fQuery)) fDrag.files.push_back(en.info.path);
                    }
                }
            } else {  // Real folder -> entries under this relPath
                const std::string& folder = node.relPath;
                for (const PresetListEntry& en : fEntries) {
                    if (folder.empty() || en.info.folder == folder || en.info.folder.rfind(folder + "/", 0) == 0) {
                        fDrag.files.push_back(en.info.path);
                    }
                }
            }

            if (fDrag.files.empty()) {
                setStatus("Nothing to drag here", true);
                return;
            }

            fDrag.active = true;
            fDrag.sourceNode = idx;
            lv_obj_set_style_opa(node.btn, LV_OPA_50, 0);
            // Claim the gesture so the scrollable tree does not steal it.
            if (fTree) lv_obj_clear_flag(fTree, LV_OBJ_FLAG_SCROLLABLE);
        }

        dragTick();
    }

    void dragTick() {
        if (!fDrag.active) return;
        lv_indev_t* indev = lv_indev_active();
        if (!indev) return;
        lv_point_t p;
        lv_indev_get_point(indev, &p);
        // Hit-test against the TRANSFORMED node rectangles. The browser lives
        // on the fixed 1280x720 content canvas, which applyContentFit() scales;
        // indev points are raw window pixels. lv_indev_search_obj() can miss
        // those rectangles at non-1.0 fit scales, leaving a drag with no drop
        // target even though the pointer is visibly over a folder.
        int nodeIdx = -1;
        if (fTree) {
            lv_area_t treeArea;
            lv_obj_get_coords(fTree, &treeArea);
            lv_obj_get_transformed_area(fTree, &treeArea, LV_OBJ_POINT_TRANSFORM_FLAG_RECURSIVE);
            if (p.x >= treeArea.x1 && p.x <= treeArea.x2 &&
                p.y >= treeArea.y1 && p.y <= treeArea.y2) {
                for (int i = (int)fNodes.size() - 1; i >= 0; --i) {
                    lv_obj_t* btn = fNodes[(size_t)i].btn;
                    if (!btn || !lv_obj_is_visible(btn)) continue;
                    lv_area_t a;
                    lv_obj_get_coords(btn, &a);
                    lv_obj_get_transformed_area(btn, &a, LV_OBJ_POINT_TRANSFORM_FLAG_RECURSIVE);
                    if (p.x >= a.x1 && p.x <= a.x2 && p.y >= a.y1 && p.y <= a.y2) {
                        nodeIdx = i;
                        break;
                    }
                }
            }
        }

        lv_obj_t* newHighlight = nullptr;
        int newHighlightNode = -1;
        if (nodeIdx >= 0 && nodeIdx != fDrag.sourceNode) {
            const TreeNode& node = fNodes[(size_t)nodeIdx];
            const bool validTarget = !node.smart || node.smartKind == 0;
            if (validTarget) {
                // Consistent with onDragRelease's per-file filter: a target is
                // invalid if it is the source, a descendant of the source
                // (dropping into its own subtree), or an ancestor of the source
                // (every source file already lives under it -> nothing to copy).
                // The root ("") is always a valid target.
                const std::string srcFolder = fNodes[(size_t)fDrag.sourceNode].relPath;
                const std::string dstFolder = node.relPath;
                const bool dstInsideSrc = !srcFolder.empty() && dstFolder.rfind(srcFolder + "/", 0) == 0;
                const bool srcInsideDst = !srcFolder.empty() && !dstFolder.empty() &&
                                          srcFolder.rfind(dstFolder + "/", 0) == 0;
                if (!dstInsideSrc && !srcInsideDst) {
                    newHighlight = node.btn;
                    newHighlightNode = nodeIdx;
                }
            }
        }

        if (newHighlight != fDrag.highlight) {
            if (fDrag.highlight) {
                lv_obj_set_style_bg_opa(fDrag.highlight, LV_OPA_10, 0);
                lv_obj_set_style_border_opa(fDrag.highlight, LV_OPA_0, 0);
            }
            fDrag.highlight = newHighlight;
            fDrag.highlightNode = newHighlightNode;
            if (newHighlight) {
                lv_obj_set_style_bg_opa(newHighlight, LV_OPA_20, 0);
                lv_obj_set_style_border_opa(newHighlight, LV_OPA_40, 0);
            }
        }
    }

    void onDragRelease(lv_event_t* /*e*/) {
        // Always restore the tree's scrollability: onNodePressed freezes it
        // for every press, including a plain click that never becomes a drag.
        if (fTree) lv_obj_add_flag(fTree, LV_OBJ_FLAG_SCROLLABLE);
        if (!fDrag.active) return;
        fDrag.active = false;
        if (fDrag.sourceNode >= 0 && fDrag.sourceNode < (int)fNodes.size()) {
            lv_obj_set_style_opa(fNodes[(size_t)fDrag.sourceNode].btn, LV_OPA_COVER, 0);
        }
        lv_obj_t* hl = fDrag.highlight;
        const int hlNode = fDrag.highlightNode;
        fDrag.highlight = nullptr;
        fDrag.highlightNode = -1;
        if (hl) {
            lv_obj_set_style_bg_opa(hl, LV_OPA_10, 0);
            lv_obj_set_style_border_opa(hl, LV_OPA_0, 0);
        }
        fDrag.sourceNode = -1;
        if (hlNode < 0 || hlNode >= (int)fNodes.size() || fDrag.files.empty()) return;

        const TreeNode& target = fNodes[(size_t)hlNode];
        // Drop onto a smart node other than All Presets is not a valid target.
        if (target.smart && target.smartKind != 0) return;

        // Never copy a source whose folder equals or starts with the target.
        const std::string targetFolder = target.relPath;
        std::vector<std::string> files;
        for (const std::string& f : fDrag.files) {
            const std::string key = relKey(fPresetDir, f);
            const size_t slash = key.find_last_of('/');
            const std::string folder = (slash == std::string::npos) ? "" : key.substr(0, slash);
            if (targetFolder.empty() || (folder != targetFolder && folder.rfind(targetFolder + "/", 0) != 0)) {
                files.push_back(f);
            }
        }
        fDrag.files.clear();

        if (files.empty()) {
            setStatus("Nothing to copy", true);
            return;
        }
        std::string targetAbs = fPresetDir;
        if (!targetFolder.empty()) {
            std::filesystem::path p = std::filesystem::u8path(fPresetDir) / std::filesystem::u8path(targetFolder);
            targetAbs = p.u8string();
        }
        std::string err;
        const int n = copyPresetsInto(files, targetAbs, err);
        if (!err.empty()) {
            setStatus(err, true);
        } else {
            setStatus("Copied " + std::to_string(n) + " preset(s) to " + target.label, false);
        }
        refresh(true);
    }

    // --- state ---
    lv_obj_t* fPanel = nullptr;
    PresetBrowserHost* fHost = nullptr;
    UIStyles* fStyles = nullptr;
    lv_obj_t** fWidgets = nullptr;
    lv_obj_t** fLabels = nullptr;
    float* fParams = nullptr;
    lv_event_cb_t fValueCb = nullptr;
    AbstractCymbalsUI* fUi = nullptr;
    bool fAttached = false;

    lv_obj_t* fSearchArea = nullptr;
    lv_obj_t* fTree = nullptr;
    lv_obj_t* fList = nullptr;
    lv_obj_t* fFolderLabel = nullptr;
    lv_obj_t* fNameArea = nullptr;
    lv_obj_t* fAuthorArea = nullptr;
    lv_obj_t* fDescArea = nullptr;
    lv_obj_t* fUsageArea = nullptr;
    lv_obj_t* fTagInput = nullptr;          // "add tag" textarea (Enter commits)
    lv_obj_t* fInfoTagRow = nullptr;        // chips + input, wraps
    lv_obj_t* fIrDropdown = nullptr;
    lv_obj_t* fStatusLabel = nullptr;
    // Round-4 info panel: tonal-profile plot + its character caption, and the
    // bank mini-chip strip.
    lv_obj_t* fProfileCanvas = nullptr;
    lv_obj_t* fProfileCharLbl = nullptr;
    lv_obj_t* fBankChips = nullptr;
    // Round-5: filter bar widgets
    lv_obj_t* fFilterBar = nullptr;
    lv_obj_t* fTypeChips[7] = {nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr}; // All,Cymbal,Percussion,FX,Crash,Ride,Hat
    lv_obj_t* fSortDropdown = nullptr;
    lv_obj_t* fCountChip = nullptr;
    lv_obj_t* fTagChips = nullptr;
    // Round-6: the "space" band under the list. It owns the leftover height so
    // the panel reads as finished at any list length, and carries the legend
    // (library shape + the drag/filter hints) while there is room for it.
    lv_obj_t* fSpaceBand = nullptr;
    lv_obj_t* fSpaceLegend = nullptr;
    int fTypeFilter = 0;  // 0=All, 1=Cymbal, 2=Percussion, 3=FX, 4=Crash, 5=Ride, 6=Hat
    int fSortOrder = 0;   // 0=Name, 1=Date, 2=Recent
    lv_obj_t* fSwatches[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

    std::string fPresetDir;
    std::vector<PresetListEntry> fEntries;
    std::map<std::string, int> fFavourites;
    std::vector<TreeNode> fNodes;
    std::vector<QueryToken> fQuery;
    ViewState fView;
    std::vector<size_t> fFilteredEntries;
    std::vector<std::string> fNavPaths;
    int fNavIndex = -1;
    std::string fLastLoadedPath;
    std::string fLoadedName, fLoadedAuthor, fLoadedDesc, fLoadedUsage, fLoadedTags;
    // Tags being edited in the info panel: the preset format stores a JSON
    // array, and every chip here becomes one array element on save. Multiple
    // tags are supported end to end; the CSV is only the host interface shape.
    std::vector<std::string> fEditTags;
    // IR spectrum for the loaded preset (empty -> metadata fallback).
    std::vector<float> fIrProfileDb;
    std::string fIrProfileForPath;   // guard: state echoes must not re-render
    static constexpr int kIrProfilePoints = 160;
    // True while this code writes an info field, so the field's VALUE_CHANGED
    // handler does not mistake a programmatic push for a user edit.
    bool fApplyingInfo = false;
    // Per-field "the user owns this text" flags: set when the user types,
    // cleared when a preset load pushes fresh metadata. Without them the
    // every-idle sync rewrote each keystroke away.
    bool fNameEdited = false, fAuthorEdited = false, fDescEdited = false,
         fUsageEdited = false;
    DragState fDrag;

    struct RowThumb {
        std::string path;
        lv_obj_t* canvas = nullptr;
        lv_color_t colour{};
    };
    std::vector<RowThumb> fRowThumbs;
    size_t fNextThumb = 0;
    lv_timer_t* fThumbTimer = nullptr;
    lv_timer_t* fTagsPreviewTimer = nullptr;

    static void thumbTimerCb(lv_timer_t* t) {
        auto* b = static_cast<PresetBrowser*>(lv_timer_get_user_data(t));
        if (!b || !b->fHost) return;
        while (b->fNextThumb < b->fRowThumbs.size()) {
            const RowThumb rec = b->fRowThumbs[b->fNextThumb++];
            if (!rec.canvas || !lv_obj_is_valid(rec.canvas)) continue;
            std::vector<float> db;
            if (b->fHost->loadPresetProfileForFile(rec.path, db) && db.size() >= 2)
                PresetRowGlyphs::redrawRowCurve(rec.canvas, db, rec.colour);
            break;
        }
    }
};

END_NAMESPACE_DISTRHO

#endif  // PRESET_BROWSER_HPP
