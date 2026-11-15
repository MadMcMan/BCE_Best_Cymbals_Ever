#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstddef>

// Open-source FX providers vendored under deps/vendored-fx.
//
// IDs are deliberately outside the Airwindows (1..547), Surge (600..608) and
// project-internal (700..756) ranges. Append new entries at the tail only:
// slot type ids are persisted in presets.
namespace CymbalsVendoredFx
{
enum : int
{
    kBaseType = 800,
    kCloudSeedReverb = 800,
    kSignalsmithChorus = 801,
    kSoundpipePhaser = 802,
    kSignalsmithStretch = 803,
    kCount = 4
};

inline bool isVendoredId(int id) { return id >= kBaseType && id < kBaseType + kCount; }
inline int indexOf(int id) { return isVendoredId(id) ? id - kBaseType : -1; }
inline int idOf(int index) { return (index >= 0 && index < kCount) ? kBaseType + index : 0; }

inline const char* name(int index)
{
    static const char* names[kCount] = {
        "CloudSeed Reverb",
        "Signalsmith Chorus",
        "Soundpipe Phaser",
        "Signalsmith Stretch"
    };
    return (index >= 0 && index < kCount) ? names[index] : "";
}

inline const char* category(int index)
{
    static const char* cats[kCount] = {"Reverb", "Modulation", "Modulation", "Pitch"};
    return (index >= 0 && index < kCount) ? cats[index] : "";
}

inline int parameterCount(int index)
{
    switch (index) {
    case 0: return 13; // CloudSeed
    case 1: return 4;  // Chorus
    case 2: return 7;  // Phaser
    case 3: return 3;  // Stretch
    default: return 0;
    }
}

inline const char* parameterName(int index, int param)
{
    static const char* cloud[13] = {
        "Low Cut", "High Cut", "Input Mix", "Dry", "Early", "Late",
        "Tap Decay", "Early FB", "Late Decay", "Diffuse FB",
        "Low Shelf", "High Shelf", "Line Size"
    };
    static const char* chorus[4] = {"Mix", "Depth", "Detune", "Stereo"};
    static const char* phaser[7] = {
        "Max Notch", "Min Notch", "Notch Width", "Notch Freq",
        "Vibrato", "Depth", "Feedback"
    };
    static const char* stretch[3] = {"Pitch", "Mix", "Tonality"};
    if (param < 0 || param >= parameterCount(index)) return "";
    if (index == 0) return cloud[param];
    if (index == 1) return chorus[param];
    if (index == 2) return phaser[param];
    if (index == 3) return stretch[param];
    return "";
}

inline float defaultNormalized(int index, int param)
{
    if (index == 0) {
        switch (param) {
        case 0: return 0.64f;  // low cut
        case 1: return 0.29f;  // high cut
        case 2: return 0.23f;  // input mix
        case 3: return 0.87f;  // dry
        case 4: return 0.0f;   // early
        case 5: return 0.66f;  // late
        case 6: return 1.0f;   // tap decay
        case 7: return 0.77f;  // early feedback
        case 8: return 0.63f;  // late decay
        case 9: return 0.85f;  // diffuse feedback
        case 10: return 0.56f; // low shelf
        case 11: return 0.77f; // high shelf
        case 12: return 0.47f; // line size
        default: return 0.5f;
        }
    }
    if (index == 1) {
        switch (param) {
        case 0: return 0.6f;  // mix
        case 1: return 0.3f;  // depth 15 ms of 50
        case 2: return 0.08f; // detune
        case 3: return 0.67f; // stereo
        default: return 0.5f;
        }
    }
    if (index == 2) {
        switch (param) {
        case 0: return 0.5f;
        case 1: return 0.5f;
        case 2: return 0.5f;
        case 3: return 0.5f;
        case 4: return 0.0f;
        case 5: return 1.0f;
        case 6: return 0.0f;
        default: return 0.5f;
        }
    }
    if (index == 3) {
        switch (param) {
        case 0: return 0.5f;  // no transposition
        case 1: return 1.0f;  // fully wet
        case 2: return 1.0f;  // tonality limit high
        default: return 0.5f;
        }
    }
    return 0.0f;
}

inline const char* formatParam(int index, int param, float norm, char* buf, std::size_t n)
{
    norm = std::clamp(norm, 0.0f, 1.0f);
    if (index == 0) {
        static const char* names[13] = {
            "Low Cut", "High Cut", "Input Mix", "Dry", "Early", "Late",
            "Tap Decay", "Early FB", "Late Decay", "Diffuse FB",
            "Low Shelf", "High Shelf", "Line Size"
        };
        std::snprintf(buf, n, "%s %.0f%%", names[param], norm * 100.0f);
        return buf;
    }
    if (index == 1) {
        switch (param) {
        case 0: std::snprintf(buf, n, "%.0f%%", norm * 100.0f); return buf;
        case 1: std::snprintf(buf, n, "%.1f ms", 2.0f + norm * 48.0f); return buf;
        case 2: std::snprintf(buf, n, "%.1f ct", 1.0f + norm * 49.0f); return buf;
        case 3: std::snprintf(buf, n, "%.2f", norm * 1.5f); return buf;
        default: std::snprintf(buf, n, "%.2f", norm); return buf;
        }
    }
    if (index == 2) {
        switch (param) {
        case 0: std::snprintf(buf, n, "%.0f Hz", 20.0f + norm * 9980.0f); return buf;
        case 1: std::snprintf(buf, n, "%.0f Hz", 20.0f + norm * 4980.0f); return buf;
        case 2: std::snprintf(buf, n, "%.0f Hz", 10.0f + norm * 4990.0f); return buf;
        case 3: std::snprintf(buf, n, "%.2f", 1.1f + norm * 2.9f); return buf;
        case 4: std::snprintf(buf, n, "%s", norm > 0.5f ? "On" : "Off"); return buf;
        case 5: std::snprintf(buf, n, "%.0f%%", norm * 100.0f); return buf;
        case 6: std::snprintf(buf, n, "%.0f%%", norm * 100.0f); return buf;
        default: std::snprintf(buf, n, "%.2f", norm); return buf;
        }
    }
    if (index == 3) {
        switch (param) {
        case 0: std::snprintf(buf, n, "%+.1f st", (norm - 0.5f) * 48.0f); return buf;
        case 1: std::snprintf(buf, n, "%.0f%%", norm * 100.0f); return buf;
        case 2: std::snprintf(buf, n, "%.0f%%", norm * 100.0f); return buf;
        default: std::snprintf(buf, n, "%.2f", norm); return buf;
        }
    }
    std::snprintf(buf, n, "%.2f", norm);
    return buf;
}
} // namespace CymbalsVendoredFx
