#ifndef UI_PLUGIN_PICKER_HPP
#define UI_PLUGIN_PICKER_HPP

#include "UICommon.hpp"
#include "UIStyles.hpp"
#include "../airwindows/AirSuiteUniversal.hpp"
#include "../surge/SurgeFx.hpp"
#include "../fx/InternalFxMeta.hpp"
#include "../fx/VendoredFxMeta.hpp"
#include "../PluginCymbals.hpp"
#include <algorithm>
#include <string>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <set>

START_NAMESPACE_DISTRHO

/**
 * UIPluginPicker - A self-contained modal overlay for selecting AirWindows algorithms
 *
 * Features:
 * - Full-screen overlay with semi-transparent background
 * - Canonical algorithm list filterable by category (duplicates hidden via AirSuiteUniversal::isCanonicalType)
 * - Real-time text search (case-insensitive substring matching)
 * - One-click selection sets algorithm type + defaults for 10 parameters
 * - Proper memory management via LV_EVENT_DELETE handlers
 */
struct UIPluginPicker {
    struct State {
        int targetSlot;
        lv_obj_t* overlay;
        lv_obj_t* list;
        lv_obj_t* searchArea;
        lv_obj_t* catRow;
        int activeCat;      // 0 = ALL, 1..8 = category
        AbstractCymbalsUI* ui;
        UIStyles* styles;
        std::set<int> favourites;
        std::string configPath;
        std::string fallbackConfigPath;
    };

    static constexpr const char* kFavouritesFile = "cymbals_fx_favourites.cfg";

    static std::string favouritesPath(AbstractCymbalsUI* ui) {
        if (!ui) return {};
        const std::string dir = ui->getConfigDirectory();
        if (dir.empty()) return {};
        return (std::filesystem::path(dir) / kFavouritesFile).string();
    }

    // Installed CLAP/VST3 bundles often live under Program Files and are not
    // writable from a DAW process. Try next-to-plugin first (the requested
    // location), then keep the same sidecar in the user's app data so stars
    // are not silently lost.
    static std::string fallbackFavouritesPath() {
#ifdef _WIN32
        const char* base = std::getenv("APPDATA");
        if (base && base[0])
            return (std::filesystem::path(base) / "Cymbals" / kFavouritesFile).string();
#elif defined(__APPLE__)
        const char* home = std::getenv("HOME");
        if (home && home[0])
            return (std::filesystem::path(home) / "Library" / "Application Support"
                    / "Cymbals" / kFavouritesFile).string();
#else
        const char* xdg = std::getenv("XDG_CONFIG_HOME");
        if (xdg && xdg[0])
            return (std::filesystem::path(xdg) / "Cymbals" / kFavouritesFile).string();
        const char* home = std::getenv("HOME");
        if (home && home[0])
            return (std::filesystem::path(home) / ".config" / "Cymbals" / kFavouritesFile).string();
#endif
        return {};
    }

    static void loadFavourites(State* st) {
        if (!st) return;
        std::ifstream in(st->configPath);
        if (!in && !st->fallbackConfigPath.empty()) in.open(st->fallbackConfigPath);
        if (!in) return;
        std::string line;
        while (std::getline(in, line)) {
            const size_t first = line.find_first_not_of(" \t\r");
            if (first == std::string::npos || line[first] == '#') continue;
            try {
                const int id = std::stoi(line.substr(first));
                if (id > 0) st->favourites.insert(id);
            } catch (...) {
                // A hand-edited bad line must never stop the picker opening.
            }
        }
    }

    static void saveFavourites(State* st) {
        if (!st) return;
        auto write = [&](const std::string& path) -> bool {
            if (path.empty()) return false;
            std::error_code ec;
            std::filesystem::create_directories(std::filesystem::path(path).parent_path(), ec);
            std::ofstream out(path, std::ios::trunc);
            if (!out) return false;
            out << "# Cymbals FX favourites v1\n";
            for (int id : st->favourites) out << id << '\n';
            return (bool)out;
        };
        if (write(st->configPath)) return;
        write(st->fallbackConfigPath);
    }

    /**
     * Case-insensitive substring containment check
     */
    static bool caseInsensitiveContains(const char* haystack, const char* needle) {
        if (!haystack || !needle) return false;
        if (needle[0] == '\0') return true;
        
        std::string h(haystack), n(needle);
        std::transform(h.begin(), h.end(), h.begin(), 
            [](unsigned char c) { return std::tolower(c); });
        std::transform(n.begin(), n.end(), n.begin(), 
            [](unsigned char c) { return std::tolower(c); });
        return h.find(n) != std::string::npos;
    }

    // Surge categories that Airwindows does not have. Existing Surge
    // classifications reuse the Airwindows category id so they share one tab.
    static int surgeCategoryId(const char* name) {
        if (!name) return 0;
        auto eq = [&](const char* b) { return caseInsensitiveContains(name, b) && caseInsensitiveContains(b, name); };
        if (eq("Delay")) return 100;
        if (eq("Modulation")) return 101;
        if (eq("Pitch")) return 102;
        if (eq("Reverb")) return (int)AirSuiteUniversal::Category::REVERB;
        if (eq("Saturation")) return (int)AirSuiteUniversal::Category::SAT;
        if (eq("Distortion")) return (int)AirSuiteUniversal::Category::DIST;
        return 0; // unknown: skipped by filterList, never mis-filed
    }

    // Internal Cymbals filters use the same classification vocabulary and
    // share the Airwindows Filter tab.
    static int internalCategoryId(const char* name) {
        if (!name) return 0;
        auto eq = [&](const char* b) {
            return caseInsensitiveContains(name, b) && caseInsensitiveContains(b, name);
        };
        if (eq("Filter")) return (int)AirSuiteUniversal::Category::FILTER;
        if (eq("Distortion")) return (int)AirSuiteUniversal::Category::DIST;
        if (eq("Reverb")) return (int)AirSuiteUniversal::Category::REVERB;
        if (eq("Modulation")) return 101;
        if (eq("Pitch")) return 102;
        return 0;
    }

    static int vendoredCategoryId(const char* name) { return internalCategoryId(name); }

    struct StarCtx {
        int type = 0;
        bool on = false;
    };

    static void deleteStarPoints(lv_event_t* e) {
        lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
        delete[] static_cast<lv_point_precise_t*>(lv_obj_get_user_data(obj));
    }

    static void deleteStarCtx(lv_event_t* e) {
        lv_obj_t* obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
        delete static_cast<StarCtx*>(lv_obj_get_user_data(obj));
    }

    static void starPoints(lv_point_precise_t* pts, lv_coord_t cx, lv_coord_t cy,
                           lv_coord_t outer, lv_coord_t inner) {
        for (int i = 0; i < 10; ++i) {
            const float ang = (-90.0f + (float)i * 36.0f) * (float)M_PI / 180.0f;
            const lv_coord_t r = (i & 1) ? inner : outer;
            pts[i].x = (lv_value_precise_t)(cx + (lv_coord_t)std::lround(std::cos(ang) * (float)r));
            pts[i].y = (lv_value_precise_t)(cy + (lv_coord_t)std::lround(std::sin(ang) * (float)r));
        }
        pts[10] = pts[0];
    }

    static void drawFilledStar(lv_event_t* e) {
        lv_obj_t* box = static_cast<lv_obj_t*>(lv_event_get_target(e));
        StarCtx* ctx = static_cast<StarCtx*>(lv_obj_get_user_data(box));
        if (!ctx || !ctx->on) return;
        lv_layer_t* layer = lv_event_get_layer(e);
        if (!layer) return;
        lv_area_t area;
        lv_obj_get_coords(box, &area);
        const lv_coord_t cx = area.x1 + scaled(12);
        const lv_coord_t cy = area.y1 + scaled(12);
        lv_point_precise_t pts[11];
        starPoints(pts, cx, cy, scaled(9), scaled(4));
        const lv_color_t gold = lv_color_hex(0xFFC24A);

        lv_draw_triangle_dsc_t d;
        lv_draw_triangle_dsc_init(&d);
        d.color = gold;
        d.opa = LV_OPA_COVER;
        for (int i = 0; i < 10; ++i) {
            d.p[0] = {(lv_value_precise_t)cx, (lv_value_precise_t)cy};
            d.p[1] = pts[i];
            d.p[2] = pts[i + 1];
            lv_draw_triangle(layer, &d);
        }
    }

    static void refreshStarVisual(lv_obj_t* box, const StarCtx* ctx) {
        if (!box || !ctx) return;
        // Outline for not-favourited, filled gold (custom draw) when on.
        if (lv_obj_t* line = lv_obj_get_child(box, 0)) {
            if (ctx->on) lv_obj_add_flag(line, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_clear_flag(line, LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_invalidate(box);
    }

    static lv_obj_t* createStarIcon(lv_obj_t* parent, int type, bool on) {
        lv_obj_t* box = lv_obj_create(parent);
        lv_obj_set_size(box, scaled(24), scaled(24));
        lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(box, 0, 0);
        lv_obj_set_style_pad_all(box, 0, 0);
        lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(box, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(box, 6);

        auto* ctx = new StarCtx{type, on};
        lv_obj_set_user_data(box, ctx);
        lv_obj_add_event_cb(box, drawFilledStar, LV_EVENT_DRAW_MAIN, nullptr);
        lv_obj_add_event_cb(box, deleteStarCtx, LV_EVENT_DELETE, nullptr);

        const lv_coord_t c = scaled(12);
        const lv_coord_t outer = scaled(9);
        const lv_coord_t inner = scaled(4);
        auto* pts = new lv_point_precise_t[11];
        starPoints(pts, c, c, outer, inner);
        lv_obj_t* line = lv_line_create(box);
        lv_obj_set_size(line, scaled(24), scaled(24));
        lv_line_set_points(line, pts, 11);
        lv_obj_set_style_line_width(line, scaled(2), 0);
        lv_obj_set_style_line_color(line, COL_TEXT_DIM, 0);
        lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(line, pts);
        lv_obj_add_event_cb(line, deleteStarPoints, LV_EVENT_DELETE, nullptr);
        refreshStarVisual(box, ctx);
        return box;
    }

    static void onFavouriteClicked(lv_event_t* e) {
        State* st = static_cast<State*>(lv_event_get_user_data(e));
        if (!st) return;
        lv_obj_t* box = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
        StarCtx* ctx = static_cast<StarCtx*>(lv_obj_get_user_data(box));
        if (!ctx) return;
        ctx->on = !ctx->on;
        if (ctx->on) st->favourites.insert(ctx->type);
        else st->favourites.erase(ctx->type);
        saveFavourites(st);
        refreshStarVisual(box, ctx);
        lv_event_stop_bubbling(e);
    }

    /**
     * Add a single algorithm button to the list
     */
    static void addListItem(lv_obj_t* list, State* st,
                           int type,
                           const char* name, UIStyles* /*styles*/) {
        // Use LVGL v9 lv_list_add_button API
        lv_obj_t* btn = lv_list_add_button(list, nullptr, name);
        if (!btn) return;
        lv_obj_set_style_pad_right(btn, scaled(34), 0);

        lv_obj_t* star = createStarIcon(btn, type, st->favourites.count(type) != 0);
        lv_obj_align(star, LV_ALIGN_RIGHT_MID, -scaled(4), 0);
        lv_obj_add_event_cb(star, onFavouriteClicked, LV_EVENT_CLICKED, st);

        lv_obj_set_user_data(btn, (void*)(intptr_t)type);
        lv_obj_add_event_cb(btn, onAlgoSelected, LV_EVENT_CLICKED, st);
    }

    /**
     * Populate list with algorithms matching current category and search filters
     * Sorted alphabetically by display name
     */
    static void filterList(State* st, int cat, const char* search) {
        st->activeCat = cat;
        lv_obj_clean(st->list);  // Remove all children
        const bool favOnly = (cat == -1);

        // Collect matching algorithms with their names
        std::vector<std::pair<int, std::string>> matches;

        for (int t = 1; t < AirSuiteUniversal::COUNT; ++t) {
            AirSuiteUniversal::Type type = (AirSuiteUniversal::Type)t;
            
            // Phase 1: Skip non-canonical types (duplicate display names)
            if (!AirSuiteUniversal::isCanonicalType(type)) continue;
            
            const auto& identity = AirSuiteUniversal::getRuntimeIdentity(type);
            const char* name = identity.displayName;
            if (!name || name[0] == '\0') continue;  // Skip gaps

            if (favOnly && st->favourites.count((int)type) == 0) continue;

            // Category filter (0 = ALL, -1 = Favourites; Surge-only ids are handled below)
            if (cat > 0) {
                if (cat >= 100) continue;
                bool inCat = AirSuiteUniversal::isInCategory(type, (AirSuiteUniversal::Category)cat);
                if (cat == (int)AirSuiteUniversal::Category::FILTER) {
                    // One Filter tab: Airwindows FILTER + BIQUAD + XYZ.
                    inCat = inCat ||
                        AirSuiteUniversal::isInCategory(type, AirSuiteUniversal::Category::BIQUAD) ||
                        AirSuiteUniversal::isInCategory(type, AirSuiteUniversal::Category::XYZ);
                }
                if (!inCat) continue;
            }

            // Search filter (case-insensitive substring)
            if (search && search[0] != '\0') {
                if (!caseInsensitiveContains(name, search)) continue;
            }

            // Add to matches list
            matches.push_back({(int)type, std::string(name)});
        }

        // Surge source. Its Airwindows-derived types are deliberately NOT
        // duplicated here: source 1 (Airwindows) owns those algorithms.
        for (int i = 0; i < CymbalsSurgeFx::algorithmCount(); ++i) {
            const char* name = CymbalsSurgeFx::algorithmName(i);
            if (!name || !name[0]) continue;
            const int mapped = surgeCategoryId(CymbalsSurgeFx::algorithmCategory(i));
            if (mapped == 0) continue;
            if (favOnly && st->favourites.count(CymbalsSurgeFx::algorithmId(i)) == 0) continue;
            if (cat > 0 && cat != mapped) continue;
            if (search && search[0] && !caseInsensitiveContains(name, search)) continue;
            matches.push_back({CymbalsSurgeFx::algorithmId(i), std::string(name)});
        }

        // Project-owned filters already in the engine, exposed as FX slots.
        for (int i = 0; i < CymbalsInternalFx::kCount; ++i) {
            const char* name = CymbalsInternalFx::name(i);
            if (!name || !name[0]) continue;
            const int mapped = internalCategoryId(CymbalsInternalFx::category(i));
            if (mapped == 0) continue;
            if (favOnly && st->favourites.count(CymbalsInternalFx::idOf(i)) == 0) continue;
            if (cat > 0 && cat != mapped) continue;
            if (search && search[0] && !caseInsensitiveContains(name, search)) continue;
            matches.push_back({CymbalsInternalFx::idOf(i), std::string(name)});
        }

        // Vendored open-source FX (CloudSeed, Signalsmith, Soundpipe).
        for (int i = 0; i < CymbalsVendoredFx::kCount; ++i) {
            const char* name = CymbalsVendoredFx::name(i);
            if (!name || !name[0]) continue;
            const int id = CymbalsVendoredFx::idOf(i);
            const int mapped = vendoredCategoryId(CymbalsVendoredFx::category(i));
            if (mapped == 0) continue;
            if (favOnly && st->favourites.count(id) == 0) continue;
            if (cat > 0 && cat != mapped) continue;
            if (search && search[0] && !caseInsensitiveContains(name, search)) continue;
            matches.push_back({id, std::string(name)});
        }

        // Sort by display name alphabetically, case-insensitively so the
        // mixed-capitalisation Airwindows/Surge/internal names read as one
        // A-Z list instead of ASCII (uppercase block first).
        auto lower = [](std::string s) {
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c) { return (char)std::tolower(c); });
            return s;
        };
        std::sort(matches.begin(), matches.end(),
                  [&](const std::pair<int, std::string>& a,
                      const std::pair<int, std::string>& b) {
                      const std::string la = lower(a.second);
                      const std::string lb = lower(b.second);
                      if (la == lb) return a.first < b.first;
                      return la < lb;
                  });
        
        // Add sorted items to list
        for (const auto& match : matches) {
            addListItem(st->list, st, match.first, match.second.c_str(), st->styles);
        }
    }

    /**
     * Highlight the active category button and dim others
     */
    static void updateCatButtons(State* st, int activeCat) {
        if (!st || !st->catRow) return;
        
        uint32_t childCount = lv_obj_get_child_count(st->catRow);
        for (uint32_t i = 0; i < childCount; ++i) {
            lv_obj_t* btn = lv_obj_get_child(st->catRow, i);
            if (!btn) continue;
            
            int cat = (int)(intptr_t)lv_obj_get_user_data(btn);
            if (cat == activeCat) {
                lv_obj_set_style_border_color(btn, COL_HIGHLIGHT, 0);
                lv_obj_set_style_border_width(btn, 2, 0);
            } else {
                lv_obj_set_style_border_color(btn, COL_BORDER, 0);
                lv_obj_set_style_border_width(btn, 1, 0);
            }
        }
    }

    /**
     * LV_EVENT_CLICKED handler for category filter buttons
     */
    static void onCatBtnClicked(lv_event_t* e) {
        State* st = (State*)lv_event_get_user_data(e);
        if (!st) return;
        
        lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
        int cat = (int)(intptr_t)lv_obj_get_user_data(btn);
        
        const char* search = lv_textarea_get_text(st->searchArea);
        filterList(st, cat, search);
        updateCatButtons(st, cat);
    }

    /**
     * LV_EVENT_VALUE_CHANGED handler for search textarea
     */
    static void onSearchChanged(lv_event_t* e) {
        State* st = (State*)lv_event_get_user_data(e);
        if (!st) return;
        
        const char* text = lv_textarea_get_text((lv_obj_t*)lv_event_get_target(e));
        filterList(st, st->activeCat, text);
    }

    /**
     * LV_EVENT_CLICKED handler for algorithm selection
     * 
     * Phase 2: Apply extracted defaults with fallback logic
     * Priority order for each parameter default:
     *   1. Extracted default (from WinVST metadata) if available
     *   2. Spec default (from algorithm spec) if extracted unavailable
     *   3. Neutral 0.5f fallback if neither available
     * 
     * This ensures algorithm defaults match the original WinVST behavior.
     */
    static void onAlgoSelected(lv_event_t* e) {
        State* st = (State*)lv_event_get_user_data(e);
        if (!st || !st->ui) return;
        
        lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
        int typeInt = (int)(intptr_t)lv_obj_get_user_data(btn);
        const bool isInternal = CymbalsInternalFx::isInternalId(typeInt);
        const bool isSurge = !isInternal && typeInt >= CymbalsSurgeFx::kBaseType;
        AirSuiteUniversal::Type selectedType = (isSurge || isInternal)
            ? AirSuiteUniversal::NONE
            : AirSuiteUniversal::sanitizeTypeValue((float)typeInt);
        if (!isSurge && !isInternal && selectedType == AirSuiteUniversal::NONE) {
            lv_obj_delete_async(st->overlay);
            return;
        }
        
        // Calculate parameter index for this slot's type parameter
        // Layout: slot n type = kParamSlot1Type + n*16
        const int typeIdx = PluginCymbals::kParamSlot1Type + st->targetSlot * 16;
        
        // Set the algorithm type
        st->ui->editParameter(typeIdx, true);
        st->ui->setParamValue(typeIdx, (float)((isSurge || isInternal) ? typeInt : (int)selectedType));
        st->ui->editParameter(typeIdx, false);

        // Set algorithm defaults for parameters 1..13.
        const int surgeIdx = isSurge ? (typeInt - CymbalsSurgeFx::kBaseType) : -1;
        const int internalIdx = isInternal ? CymbalsInternalFx::indexOf(typeInt) : -1;
        for (int k = 0; k < 13; ++k) {
            float def = isInternal
                ? CymbalsInternalFx::defaultNormalized(internalIdx, k)
                : (isSurge
                    ? CymbalsSurgeFx::parameterDefaultNormalized(surgeIdx, k)
                    : AirSuiteUniversal::getParamDefault(selectedType, k));
            int paramIdx = typeIdx + 1 + k;
            st->ui->editParameter(paramIdx, true);
            st->ui->setParamValue(paramIdx, def);
            st->ui->editParameter(paramIdx, false);
        }
        
        // Notify UI to rebuild the slot panel visually with the new algorithm
        st->ui->notifySlotTypeChanged(st->targetSlot, typeInt);
        
        // Close modal (asynchronously safe in LVGL v9)
        lv_obj_delete_async(st->overlay);
    }

    /**
     * Create category filter buttons (21 total: ALL + 20 categories)
     * Returns the container row
     */
    static lv_obj_t* createCategoryRow(lv_obj_t* modal, State* st, UIStyles* styles) {
        lv_obj_t* catRow = lv_obj_create(modal);
        lv_obj_set_width(catRow, LV_PCT(100));
        lv_obj_set_height(catRow, LV_SIZE_CONTENT);
        lv_obj_set_style_pad_bottom(catRow, 6, 0);
        lv_obj_set_layout(catRow, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(catRow, LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_style_pad_all(catRow, 0, 0);
        lv_obj_set_style_bg_opa(catRow, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_opa(catRow, LV_OPA_TRANSP, 0);
        lv_obj_set_flex_align(catRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        
        // Category buttons: ALL + every category that still has canonical algorithms.
        // Canonicalization can empty a category (e.g. all AMBIENCE types redirect to
        // REVERB/EFFECT), so empty ones are omitted — a dead-end "Ambience" button
        // that always shows an empty list is worse than no button.
        auto categoryHasAlgorithms = [](AirSuiteUniversal::Category cat) {
            for (int t = 1; t < AirSuiteUniversal::COUNT; ++t) {
                const AirSuiteUniversal::Type type = (AirSuiteUniversal::Type)t;
                if (AirSuiteUniversal::isCanonicalType(type) &&
                    AirSuiteUniversal::isInCategory(type, cat)) {
                    return true;
                }
            }
            return false;
        };

        std::vector<std::pair<int, std::string>> categories;
        for (int c = (int)AirSuiteUniversal::Category::REVERB;
             c <= (int)AirSuiteUniversal::Category::XYZ; ++c) {
            const auto cat = (AirSuiteUniversal::Category)c;
            const char* label = AirSuiteUniversal::getCategoryName(cat);
            if (!label || label[0] == '\0') continue;
            if (cat == AirSuiteUniversal::Category::BIQUAD ||
                cat == AirSuiteUniversal::Category::XYZ) continue; // merged into Filter
            if (!categoryHasAlgorithms(cat)) continue;
            categories.push_back({c, label});
        }
        // Surge-only classifications, shown alongside the Airwindows ones.
        // Added BEFORE the sort so every group is alphabetical; previously
        // Delay/Modulation/Pitch were appended after the sorted Airwindows
        // list and always landed at the end.
        if (CymbalsSurgeFx::algorithmCount() > 0) {
            categories.push_back({100, "Delay"});
            categories.push_back({101, "Modulation"});
            categories.push_back({102, "Pitch"});
        }
        std::sort(categories.begin(), categories.end(),
                  [](const std::pair<int, std::string>& a,
                     const std::pair<int, std::string>& b) {
                      std::string la = a.second, lb = b.second;
                      std::transform(la.begin(), la.end(), la.begin(),
                                     [](unsigned char c) { return (char)std::tolower(c); });
                      std::transform(lb.begin(), lb.end(), lb.begin(),
                                     [](unsigned char c) { return (char)std::tolower(c); });
                      return la < lb;
                  });
        categories.insert(categories.begin(), {0, "ALL"});
        categories.insert(categories.begin() + 1, {-1, "Favourites"});
        // Internal EQ models, filterbank topologies and the Airwindows
        // FILTER/BIQUAD/XYZ families all live under the single Filter tab.

        for (const auto& cat : categories) {
            lv_obj_t* btn = lv_btn_create(catRow);
            lv_obj_set_width(btn, LV_SIZE_CONTENT);
            lv_obj_set_height(btn, scaled(30));
            lv_obj_set_style_pad_hor(btn, scaled(10), 0);
            lv_obj_set_style_pad_ver(btn, scaled(6), 0);
            styles->applyMomentaryButton(btn);
            // Local font: the picker lives on lv_layer_top() and survives window
            // resizes, while fStyles.init() re-initializes the shared btnMain on
            // every resize. Inheriting the mutable style leaves an open picker's
            // buttons with stale/mixed fonts and clipped labels; a local font is
            // immune (the picker simply keeps its opening-scale look).
            lv_obj_set_style_text_font(btn, getScaledSmallFont(), 0);
            lv_obj_set_style_border_width(btn, 1, 0);
            lv_obj_set_style_border_color(btn, COL_BORDER, 0);
            
            // Highlight ALL on startup
            if (cat.first == 0) {
                lv_obj_set_style_border_color(btn, COL_HIGHLIGHT, 0);
                lv_obj_set_style_border_width(btn, 2, 0);
            }
            
            lv_obj_t* lbl = lv_label_create(btn);
            lv_label_set_text(lbl, cat.second.c_str());
            lv_obj_center(lbl);
            
            lv_obj_set_user_data(btn, (void*)(intptr_t)cat.first);
            lv_obj_add_event_cb(btn, onCatBtnClicked, LV_EVENT_CLICKED, st);
        }
        
        st->catRow = catRow;
        return catRow;
    }

    /**
     * Open the FX picker modal for the given slot.
     *
     * `screen` should be the editor's CONTENT CANVAS (fContentRoot), not
     * lv_layer_top(): the canvas carries applyContentFit()'s transform, so the
     * modal scales with the rest of the UI. On the process-global overlay layer
     * it stayed at 1:1 next to a scaled interface (reported: "select plugin
     * should scale with the app").
     */
    static void open(lv_obj_t* screen, int targetSlot, AbstractCymbalsUI* ui, UIStyles& styles) {
        if (!screen || !ui) return;
        if (targetSlot < 0 || targetSlot >= 6) return;
        
        // ========== CREATE OVERLAY ==========
        lv_obj_t* overlay = lv_obj_create(screen);
        lv_obj_set_size(overlay, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_color(overlay, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(overlay, LV_OPA_70, 0);
        lv_obj_set_style_border_opa(overlay, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(overlay, 0, 0);
        lv_obj_set_layout(overlay, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(overlay, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(overlay, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_add_flag(overlay, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(overlay, LV_OBJ_FLAG_FLOATING);  // don't affect parent scroll/layout
        lv_obj_set_pos(overlay, 0, 0);  // anchor to top-left of layer_top
        
        // Allocate and attach state
        State* st = new State{targetSlot, overlay, nullptr, nullptr, nullptr, 0, ui, &styles, {}, {}, {}};
        st->configPath = favouritesPath(ui);
        st->fallbackConfigPath = fallbackFavouritesPath();
        loadFavourites(st);
        lv_obj_set_user_data(overlay, st);
        lv_obj_add_event_cb(overlay, [](lv_event_t* e) {
            State* state = (State*)lv_obj_get_user_data((lv_obj_t*)lv_event_get_target(e));
            if (state) delete state;
        }, LV_EVENT_DELETE, nullptr);
        
        // Close picker when clicking overlay backdrop (outside modal)
        lv_obj_add_event_cb(overlay, [](lv_event_t* e) {
            lv_obj_t* ovr = (lv_obj_t*)lv_event_get_target(e);
            if (ovr) lv_obj_delete_async(ovr);
        }, LV_EVENT_CLICKED, nullptr);
        
        // ========== CREATE MODAL CONTAINER ==========
        lv_obj_t* modal = lv_obj_create(overlay);
        // scaled(), like everything else in the UI: these were raw pixels, so at
        // 1920/2560 the picker stayed 860x520 while the rest of the interface
        // scaled around it, leaving the modal visually tiny and cramped. At 1280
        // scaled(x) == x, which is why the size audits never caught it - the
        // picker is a modal, not one of the 7 stage panels they sweep.
        lv_obj_set_size(modal, scaled(860), scaled(520));
        lv_obj_add_style(modal, &styles.panel, 0);
        lv_obj_set_layout(modal, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(modal, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(modal, 8, 0);
        lv_obj_add_flag(modal, LV_OBJ_FLAG_CLICKABLE);
        
        // Prevent click-through to overlay
        lv_obj_add_event_cb(modal, [](lv_event_t* e) {
            lv_event_stop_bubbling(e);
        }, LV_EVENT_CLICKED, nullptr);
        
        // ========== CREATE HEADER ROW (title + close) ==========
        lv_obj_t* hdr = lv_obj_create(modal);
        lv_obj_set_width(hdr, LV_PCT(100));
        lv_obj_set_height(hdr, scaled(40));
        lv_obj_set_layout(hdr, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_bg_opa(hdr, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_opa(hdr, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(hdr, 0, 0);
        
        lv_obj_t* title = lv_label_create(hdr);
        lv_label_set_text(title, "SELECT FX");
        lv_obj_add_style(title, &styles.title, 0);
        lv_obj_set_flex_grow(title, 1);
        
        lv_obj_t* closeBtn = lv_btn_create(hdr);
        lv_obj_set_size(closeBtn, scaled(32), scaled(32));
        styles.applyMomentaryButton(closeBtn);
        // Freeze font like the category buttons (see comment above): the modal
        // outlives fStyles.init() re-initializing btnMain on window resizes.
        lv_obj_set_style_text_font(closeBtn, getScaledSmallFont(), 0);
        lv_obj_set_user_data(closeBtn, overlay);
        lv_obj_t* closeLbl = lv_label_create(closeBtn);
        lv_label_set_text(closeLbl, LV_SYMBOL_CLOSE);
        lv_obj_center(closeLbl);
        lv_obj_add_event_cb(closeBtn, [](lv_event_t* e) {
            lv_obj_t* ovr = (lv_obj_t*)lv_obj_get_user_data((lv_obj_t*)lv_event_get_target(e));
            if (ovr) lv_obj_delete_async(ovr);
        }, LV_EVENT_CLICKED, nullptr);
        
        // ========== CREATE CATEGORY FILTER ROW ==========
        createCategoryRow(modal, st, &styles);
        
        // ========== CREATE SEARCH TEXTAREA ==========
        lv_obj_t* ta = lv_textarea_create(modal);
        uiBindTextInputToOwnIndev(ta);
        lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
        lv_obj_set_width(ta, LV_PCT(100));
        lv_obj_set_height(ta, scaled(40));
        lv_textarea_set_one_line(ta, true);
        lv_textarea_set_placeholder_text(ta, "Search...");
        lv_obj_add_event_cb(ta, onSearchChanged, LV_EVENT_VALUE_CHANGED, st);
        st->searchArea = ta;
        
        // ========== CREATE SCROLLABLE LIST ==========
        lv_obj_t* list = lv_list_create(modal);
        lv_obj_set_width(list, LV_PCT(100));
        lv_obj_set_flex_grow(list, 1);
        // Scrollable by construction, and registered so the widget's onScroll
        // can drive it: the LVGL mousewheel encoder indev does not scroll lists
        // by itself, so without this the wheel did nothing in this list while
        // working in the ones that WERE registered.
        lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);
        UIWidgets::attachScrollbarDrag(list);
        uiOpenPickerListSlot() = list;
        st->list = list;
        
        // Populate initial list (ALL category, empty search)
        filterList(st, 0, "");
        updateCatButtons(st, 0);

        // Focus LAST: creating the category/list buttons can move the keypad
        // group's focus. Focusing the search box before the list existed made
        // typing only work intermittently (it depended on which widget won
        // focus during the first layout pass).
        uiBindTextInputToOwnIndev(ta);
        if (lv_group_t* group = uiKeypadGroupForOwnDisplay(ta)) {
            if (lv_obj_get_group(ta) != group)
                lv_group_add_obj(group, ta);
            lv_group_focus_obj(ta);
        }
        lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
    }
};

END_NAMESPACE_DISTRHO

#endif // UI_PLUGIN_PICKER_HPP
