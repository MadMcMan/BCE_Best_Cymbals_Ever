// Preset browser pure logic: search parsing, favourites sidecar
// I/O, and drag-copy semantics. Header-only, NO LVGL, NO plugin includes —
// tests/preset_browser_logic.cpp exercises this standalone with plain g++.
#ifndef PRESET_BROWSER_LOGIC_HPP
#define PRESET_BROWSER_LOGIC_HPP

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <initializer_list>
#include <fstream>
#include <map>
#include <string>
#include <vector>

// Standalone: this header must compile without DPF (tests link it with plain
// g++). When included from plugin code the macros come from DistrhoDefines.
#ifndef START_NAMESPACE_DISTRHO
#define START_NAMESPACE_DISTRHO namespace DISTRHO {
#define END_NAMESPACE_DISTRHO }
#endif

START_NAMESPACE_DISTRHO

namespace PresetBrowserLogic {

// --- search syntax ---

enum class QueryField { Name, Desc, Use, Author };

struct QueryToken {
    bool isTag = false;                    // "#Bright"
    QueryField field = QueryField::Name;
    bool isPhrase = false;                 // "\"some thing\"" -> exact phrase substring
    std::string text;                      // lowercased
};

struct PresetSearchFields {
    std::string name, description, usage, author;
    std::vector<std::string> tags;
};

inline std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

// Tokenize on whitespace; '"' starts a phrase until the next '"'; a token
// starting with '#' -> isTag (text after '#'); "key:value" with key in
// {name,desc,use,author} -> field token; anything else -> bare word.
inline std::vector<QueryToken> parseQuery(const std::string& q) {
    std::vector<QueryToken> out;
    const size_t n = q.size();
    size_t i = 0;
    while (i < n) {
        while (i < n && std::isspace((unsigned char)q[i])) ++i;
        if (i >= n) break;

        QueryToken tok;
        if (q[i] == '#') {
            // Tag token; a quoted tag keeps spaces ("#\"foo bar\"").
            tok.isTag = true;
            ++i;
            if (i < n && q[i] == '"') {
                ++i;
                const size_t start = i;
                while (i < n && q[i] != '"') ++i;
                tok.text = toLower(q.substr(start, i - start));
                if (i < n) ++i;
            } else {
                const size_t start = i;
                while (i < n && !std::isspace((unsigned char)q[i])) ++i;
                tok.text = toLower(q.substr(start, i - start));
            }
        } else if (q[i] == '"') {
            tok.isPhrase = true;
            ++i;
            const size_t start = i;
            while (i < n && q[i] != '"') ++i;
            tok.text = toLower(q.substr(start, i - start));
            if (i < n) ++i;  // consume closing quote
        } else {
            const size_t start = i;
            while (i < n && !std::isspace((unsigned char)q[i])) ++i;
            const std::string word = toLower(q.substr(start, i - start));
            if (!word.empty() && word[0] == '#') {
                tok.isTag = true;
                tok.text = word.substr(1);
            } else {
                const size_t colon = word.find(':');
                if (colon != std::string::npos) {
                    const std::string key = word.substr(0, colon);
                    const std::string val = word.substr(colon + 1);
                    if (key == "name")        { tok.field = QueryField::Name;   tok.text = val; }
                    else if (key == "desc")   { tok.field = QueryField::Desc;   tok.text = val; }
                    else if (key == "use")    { tok.field = QueryField::Use;    tok.text = val; }
                    else if (key == "author") { tok.field = QueryField::Author; tok.text = val; }
                    else tok.text = word;  // unknown key -> bare word (name match)
                } else {
                    tok.text = word;
                }
            }
        }
        if (!tok.text.empty()) out.push_back(tok);
    }
    return out;
}

inline bool queryMatches(const PresetSearchFields& f, const std::vector<QueryToken>& tokens) {
    if (tokens.empty()) return true;
    for (const QueryToken& t : tokens) {
        if (t.isTag) {
            bool any = false;
            for (const std::string& tag : f.tags) {
                if (toLower(tag) == t.text) { any = true; break; }
            }
            if (!any) return false;
        } else if (t.isPhrase) {
            std::string hay = toLower(f.name + " " + f.description + " " + f.usage + " " + f.author);
            for (const std::string& tag : f.tags) hay += " " + toLower(tag);
            if (hay.find(t.text) == std::string::npos) return false;
        } else {
            std::string hay;
            switch (t.field) {
                case QueryField::Name:   hay = f.name;        break;
                case QueryField::Desc:   hay = f.description; break;
                case QueryField::Use:    hay = f.usage;       break;
                case QueryField::Author: hay = f.author;      break;
            }
            if (toLower(hay).find(t.text) == std::string::npos) return false;
        }
    }
    return true;
}

// --- live search filter (round-2 piece 3) ---
// Live-typing predicate: a preset matches if EVERY whitespace-delimited
// token in the query is found (case-insensitive substring) somewhere in
// the preset's name/description/usage/author/tags. Unlike queryMatches(),
// this does NOT parse field/tag/phrase syntax — it is a fast "type and
// narrow" path that stays responsive while the user is still typing.
// Empty / whitespace-only query => match everything.
inline bool matchesPreset(const PresetSearchFields& f, const char* query) {
    if (!query) return true;
    std::string q(query);
    // Tokenize on whitespace into lowercased bare words.
    std::vector<std::string> tokens;
    const size_t n = q.size();
    size_t i = 0;
    while (i < n) {
        while (i < n && std::isspace((unsigned char)q[i])) ++i;
        if (i >= n) break;
        const size_t start = i;
        while (i < n && !std::isspace((unsigned char)q[i])) ++i;
        tokens.push_back(toLower(q.substr(start, i - start)));
    }
    if (tokens.empty()) return true;

    // Build the haystack once per call (name+desc+use+author+tags).
    std::string hay = toLower(f.name);
    hay += ' ';
    hay += toLower(f.description);
    hay += ' ';
    hay += toLower(f.usage);
    hay += ' ';
    hay += toLower(f.author);
    for (const std::string& tag : f.tags) {
        hay += ' ';
        hay += toLower(tag);
    }
    for (const std::string& t : tokens) {
        if (hay.find(t) == std::string::npos) return false;
    }
    return true;
}

// Convenience wrapper that builds a PresetSearchFields from a raw preset
// record and runs matchesPreset. Intended for the live-typing callback
// where the caller already has the fields in hand.
struct PresetRecord {
    std::string name, description, usage, author;
    std::vector<std::string> tags;
};

inline bool matchesPreset(const PresetRecord& r, const char* query) {
    PresetSearchFields f;
    f.name = r.name;
    f.description = r.description;
    f.usage = r.usage;
    f.author = r.author;
    f.tags = r.tags;
    return matchesPreset(f, query);
}

// applyFilter: walks `inIndices` (source entry indices), writes the
// matching subset into `outIndices`, and caps the result at maxResults.
// The cap keeps the LVGL list rebuild instant under live typing; 200 is
// what the round-2 brief specified.
inline void applyFilter(const std::vector<PresetSearchFields>& fields,
                        const char* query,
                        const std::vector<size_t>& inIndices,
                        std::vector<size_t>& outIndices,
                        size_t maxResults) {
    outIndices.clear();
    outIndices.reserve(std::min(inIndices.size(), maxResults));
    for (size_t idx : inIndices) {
        if (idx >= fields.size()) continue;
        if (matchesPreset(fields[idx], query)) {
            outIndices.push_back(idx);
            if (outIndices.size() >= maxResults) break;
        }
    }
}

// --- round-4: row type classification + metadata tonal profile ---
//
// The preset list carries a per-row type badge and a tonal-profile thumbnail
// so a card reads at a glance without being opened. NOTHING here is
// persisted: the preset JSON format is untouched and no sidecar key is added.
// The kind is inferred from the preset's own name first (the author's own
// wording), then from its bank folder leaf ("cymbals/ride" -> Ride), then
// from the bank root ("percussion" -> Drum).

enum class PresetKind {
    Generic = 0, Cymbal, Crash, Ride, Hat, Sizzle, Bell, Splash,
    Orchestral, Drum, Crumpet, Percussion, kKindCount
};

inline const char* kindLabel(PresetKind k) {
    switch (k) {
        case PresetKind::Cymbal:     return "Cymbal";
        case PresetKind::Crash:      return "Crash";
        case PresetKind::Ride:       return "Ride";
        case PresetKind::Hat:        return "Hi-Hat";
        case PresetKind::Sizzle:     return "Sizzle";
        case PresetKind::Bell:       return "Bell";
        case PresetKind::Splash:     return "Splash";
        case PresetKind::Orchestral: return "Orchestral";
        case PresetKind::Drum:       return "Drum";
        case PresetKind::Crumpet:    return "Crumpet";
        case PresetKind::Percussion: return "Percussion";
        case PresetKind::Generic:
        default:                     return "Unclassified";
    }
}

// Two-letter badge text: the type column stays a fixed-width glyph block, so
// a 2-letter tag is the longest that fits without per-row width jitter.
inline const char* kindTag(PresetKind k) {
    switch (k) {
        case PresetKind::Cymbal:     return "CY";
        case PresetKind::Crash:      return "CR";
        case PresetKind::Ride:       return "RD";
        case PresetKind::Hat:        return "HT";
        case PresetKind::Sizzle:     return "SZ";
        case PresetKind::Bell:       return "BL";
        case PresetKind::Splash:     return "SP";
        case PresetKind::Orchestral: return "OR";
        case PresetKind::Drum:       return "DR";
        case PresetKind::Crumpet:    return "CP";
        case PresetKind::Percussion: return "PC";
        case PresetKind::Generic:
        default:                     return "GE";
    }
}

// Word-boundary term match (so "hat" does not fire inside "pride", but does
// fire in "hi-hat" and "Hi Hat"). Input must already be lowercased.
inline bool hasTerm(const std::string& hay, const char* term) {
    const std::string w(term);
    if (w.empty() || hay.empty()) return false;
    size_t pos = 0;
    while ((pos = hay.find(w, pos)) != std::string::npos) {
        const bool beforeOk = (pos == 0) || !std::isalnum((unsigned char)hay[pos - 1]);
        const size_t after = pos + w.size();
        const bool afterOk = (after >= hay.size()) || !std::isalnum((unsigned char)hay[after]);
        if (beforeOk && afterOk) return true;
        pos += 1;
    }
    return false;
}

// Any-of helper: singular/plural pairs are spelled out because hasTerm is a
// word-boundary match ("crash" does not match "crashes").
inline bool hasAnyTerm(const std::string& hay, std::initializer_list<const char*> terms) {
    for (const char* t : terms) {
        if (hasTerm(hay, t)) return true;
    }
    return false;
}

// Substring match for controlled, short strings (folder leaves).
inline bool hasFragment(const std::string& hay, const char* frag) {
    return hay.find(frag) != std::string::npos;
}

// Same keyword table for both surfaces, different match strictness.
inline PresetKind kindFromText(const std::string& lowerText, bool wordBoundary) {
    auto hit = [&](std::initializer_list<const char*> terms) {
        if (wordBoundary) return hasAnyTerm(lowerText, terms);
        for (const char* t : terms) {
            if (hasFragment(lowerText, t)) return true;
        }
        return false;
    };
    if (hit({"crumpet"}))                            return PresetKind::Crumpet;
    if (hit({"crash", "crashes"}))                   return PresetKind::Crash;
    if (hit({"sizzle", "sizzles"}))                  return PresetKind::Sizzle;
    if (hit({"splash", "splashes", "swish", "china", "chinas"})) return PresetKind::Splash;
    if (hit({"bell", "bells"}))                      return PresetKind::Bell;
    if (hit({"hat", "hats", "hihat", "hi-hat"}))     return PresetKind::Hat;
    if (hit({"ride", "rides"}))                      return PresetKind::Ride;
    if (hit({"cymbal", "cymbals"}))                  return PresetKind::Cymbal;
    if (hit({"drum", "drums", "kick", "snare", "tom", "toms"}))
                                                     return PresetKind::Drum;
    if (hit({"percussion", "perc"}))                 return PresetKind::Percussion;
    if (hit({"orchestral", "orch"}))                 return PresetKind::Orchestral;
    return PresetKind::Generic;
}

inline PresetKind kindFromName(const std::string& name) {
    return kindFromText(toLower(name), true);
}

inline PresetKind kindFromBank(const std::string& folder) {
    if (folder.empty()) return PresetKind::Generic;
    const size_t slash = folder.find_last_of('/');
    std::string leaf = (slash == std::string::npos) ? folder : folder.substr(slash + 1);
    if (leaf.empty()) return PresetKind::Generic;
    if (PresetKind k = kindFromText(toLower(leaf), false); k != PresetKind::Generic) return k;
    // Fall back to the bank root ("percussion", "basses", ...).
    const std::string root = toLower(folder.substr(0, slash));
    if (hasFragment(root, "perc") || hasFragment(root, "drum")) return PresetKind::Drum;
    if (hasFragment(root, "cymbal")) return PresetKind::Cymbal;
    return PresetKind::Generic;
}

// Name wins over bank: the author named the preset.
inline PresetKind inferPresetKind(const std::string& name, const std::string& folder) {
    if (PresetKind k = kindFromName(name); k != PresetKind::Generic) return k;
    return kindFromBank(folder);
}

// --- metadata tonal profile ---
// A reference spectral slope: dB against log-frequency, derived from the
// preset's own words. Deterministic, side-effect-free, and used for BOTH the
// per-row thumbnail and the PRESET INFO plot, so a row's glyph is the same
// curve the info panel shows once the preset is loaded.
inline constexpr float kProfileMinHz = 48.0f;
inline constexpr float kProfileMaxHz = 15000.0f;
inline constexpr float kProfileDbMin = -14.0f;
inline constexpr float kProfileDbMax = 14.0f;

struct PresetProfile {
    float slopeDbPerOct = 0.0f;  // brightness tilt
    float bodyDb = 0.0f;         // low shelf, ~110 Hz
    float airDb = 0.0f;          // high shelf, ~8 kHz
    float levelDb = 0.0f;        // overall offset
    int hits = 0;                // keyword hits; 0 = no metadata to read
};

struct PresetKeyword { const char* term; float weight; };

inline const PresetKeyword kSlopeKw[] = {
    {"bright", 1.0f}, {"brilliant", 1.0f}, {"shiny", 0.8f}, {"silvery", 0.8f},
    {"crisp", 0.5f}, {"ping", 0.5f}, {"sizzle", 0.5f}, {"wash", 0.5f},
    {"open", 0.4f}, {"metallic", 0.5f}, {"metalic", 0.5f}, {"metal", 0.5f},
    {"air", 0.4f}, {"airy", 0.4f},
    {"dark", -1.0f}, {"deep", -0.6f}, {"low", -0.4f}, {"boom", -0.5f},
    {"mellow", -0.6f}, {"round", -0.5f}, {"warm", -0.4f}, {"dull", -1.0f},
};
inline const PresetKeyword kBodyKw[] = {
    {"boom", 1.0f}, {"thud", 1.0f}, {"deep", 0.6f}, {"low", 0.4f},
    {"round", 0.6f}, {"full", 0.4f}, {"tight", 0.4f}, {"thin", -0.6f},
};
inline const PresetKeyword kAirKw[] = {
    {"wash", 1.0f}, {"sizzle", 0.6f}, {"shiny", 0.6f}, {"bright", 0.6f},
    {"air", 0.6f}, {"airy", 0.6f}, {"silvery", 0.6f}, {"dark", -0.4f},
    {"mellow", -0.4f},
};
inline const PresetKeyword kLevelKw[] = {
    {"loud", 1.0f}, {"crash", 0.6f}, {"huge", 1.0f}, {"big", 0.6f},
    {"cut", 0.4f}, {"gentle", -0.8f}, {"soft", -0.6f},
};

template <size_t N>
inline float keywordScore(const std::string& hay, const PresetKeyword (&table)[N], int& hits) {
    float sum = 0.0f;
    int count = 0;
    for (size_t i = 0; i < N; ++i) {
        if (hasTerm(hay, table[i].term)) { sum += table[i].weight; ++count; }
    }
    hits = count;
    return sum;
}

inline PresetProfile profileFromMetadata(const std::vector<std::string>& texts) {
    std::string hay;
    for (const std::string& t : texts) {
        if (t.empty()) continue;
        hay += ' ';
        hay += toLower(t);
    }
    PresetProfile p;
    int ns = 0, nb = 0, na = 0, nl = 0;
    const float slope = keywordScore(hay, kSlopeKw, ns);
    const float body = keywordScore(hay, kBodyKw, nb);
    const float air = keywordScore(hay, kAirKw, na);
    const float level = keywordScore(hay, kLevelKw, nl);
    // Mean keyword weight, scaled to dB. No keywords -> flat response, and
    // hits == 0 tells the UI there is no metadata to draw.
    auto norm = [](float sum, int count, float capDb) {
        if (count <= 0) return 0.0f;
        const float v = (sum / (float)count) * capDb;
        return std::max(-capDb, std::min(capDb, v));
    };
    p.slopeDbPerOct = norm(slope, ns, 1.5f);
    p.bodyDb = norm(body, nb, 6.0f);
    p.airDb = norm(air, na, 6.0f);
    p.levelDb = norm(level, nl, 2.5f);
    p.hits = ns + nb + na + nl;
    // Untagged presets: fall back to the instrument's reference tilt (a
    // cymbal bank rolls off with frequency) so the row thumbnail and the
    // info plot still draw a real curve instead of a flat 0 dB line.
    // hits == 0 still reports "no metadata" to the caption (UNTAGGED).
    if (p.hits == 0) p.slopeDbPerOct = -1.2f;
    return p;
}

// dB of the reference slope at hz (gaussian low/high shelves).
inline float profileDbAt(const PresetProfile& p, float hz) {
    if (hz <= 0.0f) hz = kProfileMinHz;
    const double oct = std::log2((double)hz / 500.0);
    double db = (double)p.levelDb + (double)p.slopeDbPerOct * oct;
    const double lowCentre = std::log2((double)hz / 110.0) / 1.4;
    const double highCentre = std::log2((double)hz / 8000.0) / 1.4;
    db += (double)p.bodyDb * std::exp(-lowCentre * lowCentre);
    db += (double)p.airDb * std::exp(-highCentre * highCentre);
    return (float)db;
}

// Character word derived from an IR magnitude curve (log-spaced bins between
// fMin..fMax, normalised peak 0 dB). Same vocabulary as the metadata path so
// the caption stays stable when the source of the curve changes.
inline void profileCharacterFromCurve(const std::vector<float>& db,
                                      float fMin, float fMax,
                                      char* buf, size_t n) {
    if (!buf || n == 0) return;
    if (db.size() < 8) { std::snprintf(buf, n, "UNTAGGED"); return; }
    // LINEAR power per band, not a mean of dB bins: narrow IR peaks and deep
    // notches otherwise average back to the same floor and hide the tilt.
    auto powerBetween = [&](float loHz, float hiHz) {
        const double lo = std::log((double)std::max(fMin, 1.0f));
        const double hi = std::log((double)std::max(fMax, fMin + 1.0f));
        int a = (int)std::lround((std::log((double)loHz) - lo) / (hi - lo) * (double)(db.size() - 1));
        int b = (int)std::lround((std::log((double)hiHz) - lo) / (hi - lo) * (double)(db.size() - 1));
        a = std::clamp(a, 0, (int)db.size() - 1);
        b = std::clamp(b, a + 1, (int)db.size());
        double sum = 0.0;
        for (int i = a; i < b; ++i) sum += std::pow(10.0, (double)db[(size_t)i] / 10.0);
        return sum / (double)std::max(1, b - a);
    };
    const double low  = powerBetween(48.0f, 200.0f);
    const double mid  = powerBetween(500.0f, 2000.0f);
    const double high = powerBetween(8000.0f, std::max(8000.0f, fMax));
    const double hiMid = 10.0 * std::log10(std::max(1e-12, high / std::max(1e-12, mid)));
    const double hiLo  = 10.0 * std::log10(std::max(1e-12, high / std::max(1e-12, low)));
    // Relative to the instrument's mid band: cymbals naturally roll off, so
    // the useful question is "is THIS preset brighter/darker than the bank's
    // normal balance", not absolute tilt.
    const char* slopeWord = (hiLo > -1.0) ? "BRIGHT"
                          : (hiLo < -3.0) ? "DARK" : "NEUTRAL";
    if (10.0 * std::log10(std::max(1e-12, low / std::max(1e-12, mid))) > 2.0)
        std::snprintf(buf, n, "%s + BOOMY", slopeWord);
    else if (hiMid > 2.0)
        std::snprintf(buf, n, "%s + AIR", slopeWord);
    else
        std::snprintf(buf, n, "%s", slopeWord);
}

// Human-readable character word for the info panel. Caller owns the buffer.
inline void profileCharacter(const PresetProfile& p, char* buf, size_t n) {
    if (!buf || n == 0) return;
    if (p.hits <= 0) { std::snprintf(buf, n, "UNTAGGED"); return; }
    const char* slopeWord = (p.slopeDbPerOct > 0.75f) ? "BRIGHT"
                          : (p.slopeDbPerOct < -0.75f) ? "DARK" : "NEUTRAL";
    if (p.bodyDb > 4.0f) std::snprintf(buf, n, "%s + BOOMY", slopeWord);
    else if (p.airDb > 4.0f) std::snprintf(buf, n, "%s + AIR", slopeWord);
    else std::snprintf(buf, n, "%s", slopeWord);
}

// --- favourites sidecar ---
// File: <presetDir>/favourites.json -> { "rel/path.json": "yellow", ... }
// Colours (fixed set): red, orange, yellow, green, blue, purple (indices 0..5).
constexpr const char* kFavColours[6] = {"red", "orange", "yellow", "green", "blue", "purple"};

inline int favColourIndex(const std::string& colourName) {
    const std::string c = toLower(colourName);
    for (int i = 0; i < 6; ++i) {
        if (c == kFavColours[i]) return i;
    }
    return -1;
}

// Keys: POSIX-normalized path relative to presetDir, lowercased.
inline std::string relKey(const std::string& presetDir, const std::string& absPath) {
    std::string rel;
    try {
        rel = std::filesystem::relative(std::filesystem::u8path(absPath),
                                        std::filesystem::u8path(presetDir)).u8string();
    } catch (...) {
        rel = absPath;
    }
    std::replace(rel.begin(), rel.end(), '\\', '/');
    return toLower(rel);
}

namespace detail {
inline std::string sidecarEscape(const std::string& in) {
    std::string out;
    out.reserve(in.size() + 8);
    for (char c : in) {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out;
}
}  // namespace detail

// Malformed file -> empty map. Values not in kFavColours are dropped.
inline std::map<std::string, int> readFavourites(const std::string& presetDir) {
    std::map<std::string, int> out;
    std::ifstream file(std::filesystem::u8path(presetDir) / "favourites.json");
    if (!file.is_open()) return out;
    const std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    size_t pos = 0;
    while (pos < json.size()) {
        const size_t q1 = json.find('"', pos);
        if (q1 == std::string::npos) break;
        const size_t q2 = json.find('"', q1 + 1);
        if (q2 == std::string::npos) break;
        const std::string key = json.substr(q1 + 1, q2 - q1 - 1);
        const size_t colon = json.find(':', q2);
        if (colon == std::string::npos) break;
        const size_t vq1 = json.find('"', colon);
        if (vq1 == std::string::npos) break;
        const size_t vq2 = json.find('"', vq1 + 1);
        if (vq2 == std::string::npos) break;
        const std::string val = json.substr(vq1 + 1, vq2 - vq1 - 1);
        const int idx = favColourIndex(val);
        if (idx >= 0) out[toLower(key)] = idx;
        pos = vq2 + 1;
    }
    return out;
}

inline void writeFavourites(const std::string& presetDir, const std::map<std::string, int>& favs) {
    try {
        std::filesystem::create_directories(std::filesystem::u8path(presetDir));
    } catch (...) {
        return;
    }
    std::ofstream file(std::filesystem::u8path(presetDir) / "favourites.json", std::ios::trunc);
    if (!file.is_open()) return;
    file << "{";
    bool first = true;
    for (const auto& kv : favs) {
        if (!first) file << ", ";
        first = false;
        const int idx = kv.second;
        file << "\"" << detail::sidecarEscape(kv.first) << "\": \""
             << (idx >= 0 && idx < 6 ? kFavColours[idx] : "yellow") << "\"";
    }
    file << "}\n";
}

// --- drag-copy semantics ---
// Copies src preset files into targetDir (absolute). On name collision append
// " (copy)", " (copy 2)", ... before the extension. Never overwrites.
// Returns count copied; first error aborts and is returned in outError.
inline int copyPresetsInto(const std::vector<std::string>& srcFiles,
                           const std::string& targetDir, std::string& outError) {
    outError.clear();
    int copied = 0;
    try {
        std::filesystem::create_directories(std::filesystem::u8path(targetDir));
        for (const std::string& src : srcFiles) {
            const std::filesystem::path sp = std::filesystem::u8path(src);
            std::filesystem::path dp = std::filesystem::u8path(targetDir) / sp.filename();
            if (std::filesystem::exists(dp)) {
                const std::string stem = sp.stem().u8string();
                const std::string ext = sp.extension().u8string();
                int n = 1;
                do {
                    std::string suffix = " (copy)";
                    if (n > 1) suffix = " (copy " + std::to_string(n) + ")";
                    dp = std::filesystem::u8path(targetDir) / (stem + suffix + ext);
                    ++n;
                } while (std::filesystem::exists(dp));
            }
            std::filesystem::copy_file(sp, dp, std::filesystem::copy_options::none);
            ++copied;
        }
    } catch (const std::exception& e) {
        outError = e.what();
    } catch (...) {
        outError = "unknown copy error";
    }
    return copied;
}

}  // namespace PresetBrowserLogic

END_NAMESPACE_DISTRHO

#endif  // PRESET_BROWSER_LOGIC_HPP
