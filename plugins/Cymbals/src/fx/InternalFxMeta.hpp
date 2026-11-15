#pragma once

#include <cstdio>
#include <cmath>
#include <cstddef>
#include <algorithm>
#include "../UnifiedFilter.hpp"

// Project-owned FX providers. Every EQ model and every filterbank topology is
// its own FX entry (no umbrella "model" knob), mirroring how Surge lists each
// effect separately in its FX picker.
//
// Raw slot type ids:
//   Airwindows canonical ids : 1 .. 547
//   Surge ids                : 600 .. 608
//   Cymbals EQ models        : 700 .. 732  (native UnifiedFilter models)
//   Cymbals filterbank types : 733 .. 755  (CombFilterBank::FT_* in enum order)
// All of them are classified "Filter"; HumpCrusher is "Distortion".

namespace CymbalsInternalFx
{
enum : int
{
    kBaseType = 700,
    kEQBase = 700,
    kEQCount = 33,       // native UnifiedFilter models 0..33, NoneModel skipped
    kFilterbankBase = 733,
    kFilterbankCount = 23,
    kHumpBase = 756,
    kHumpCount = 1,
    kCount = kEQCount + kFilterbankCount + kHumpCount
};
inline bool isInternalId(int id) { return id >= kBaseType && id < kBaseType + kCount; }
inline int indexOf(int id) { return isInternalId(id) ? id - kBaseType : -1; }
inline int idOf(int index) { return (index >= 0 && index < kCount) ? kBaseType + index : 0; }
inline bool isEQIndex(int index) { return index >= 0 && index < kEQCount; }
inline bool isHumpIndex(int index) { return index == kEQCount + kFilterbankCount; }

// Model ids for the EQ entries, skipping NoneModel (28) so presets keep 0..28.
inline int eqModelForIndex(int index)
{
    if (index < 28) return index;
    return index + 1; // 28..32 -> 29..33
}

inline const char* bankTypeName(int type)
{
    static const char* names[23] = {
        "Comb", "Allpass", "Notch", "ResoLP", "ResoBP", "ResoHP", "Phaser",
        "Flange", "GlitchPM", "Bitcrush", "Pop", "Stutter", "Vowel", "Vocal",
        "Talkbox", "Beam Clamp", "Beam Open", "Beating", "Marimba", "Membrane",
        "Metal", "Pan", "Tube"};
    return (type >= 0 && type < 23) ? names[type] : "";
}

inline const char* name(int index)
{
    if (isEQIndex(index))
        return UnifiedFilter::modelName(eqModelForIndex(index));
    if (index >= kEQCount && index < kEQCount + kFilterbankCount)
        return bankTypeName(index - kEQCount);
    if (isHumpIndex(index))
        return "HumpCrusher";
    return "";
}

inline const char* category(int index)
{
    if (isEQIndex(index)) return "Filter";
    if (index >= kEQCount && index < kEQCount + kFilterbankCount) return "Filter";
    if (isHumpIndex(index)) return "Distortion";
    return "";
}

inline int parameterCount(int index)
{
    if (isEQIndex(index)) return 6;                 // type, slope, cutoff, reso, gain, sat
    if (index >= kEQCount && index < kEQCount + kFilterbankCount) return 9;
    if (isHumpIndex(index)) return 9;
    return 0;
}

inline const char* parameterName(int index, int param)
{
    if (param < 0 || param >= parameterCount(index)) return "";
    static const char* eqNames[6] = {"Type", "Slope", "Cutoff", "Reso", "Gain", "Sat"};
    static const char* bankNames[9] = {"Freq", "Feedback", "LPF", "HPF", "Sat",
                                       "Sat Type", "Partials", "Mode Amt", "Mode Rate"};
    static const char* humpNames[9] = {"Mech", "Tube", "Filt On", "Cutoff", "Reso",
                                       "Comp On", "Amount", "Phat", "Volume"};
    if (isEQIndex(index)) return eqNames[param];
    if (isHumpIndex(index)) return humpNames[param];
    return bankNames[param];
}

// Defaults are normalized 0..1, matching the slot parameter contract.
inline float defaultNormalized(int index, int param)
{
    if (isEQIndex(index)) {
        switch (param) {
        case 0: return 0.0f;                    // Low-Pass
        case 1: return 1.0f / 3.0f;             // 24 dB/oct
        case 2: return 0.566f;                  // ~1 kHz
        case 3: return (0.707f - 0.1f) / 9.9f;  // Q 0.707
        case 4: return 0.5f;                    // 0 dB
        case 5: return 0.0f;
        default: return 0.0f;
        }
    }
    if (isHumpIndex(index)) {
        switch (param) {
        case 0: return 0.5f;   // Mech
        case 1: return 0.5f;   // Tube
        case 2: return 1.0f;   // filter on
        case 3: return 0.85f;  // cutoff
        case 4: return 0.2f;   // resonance
        case 5: return 1.0f;   // compressor on
        case 6: return 0.35f;  // amount
        case 7: return 0.0f;   // phat off
        case 8: return 0.85f;  // volume
        default: return 0.0f;
        }
    }
    switch (param) {
    case 0: return 0.400f;     // ~220 Hz
    case 1: return 0.501f;     // ~0.5 feedback
    case 2: return 0.801f;     // ~8 kHz
    case 3: return 0.301f;     // ~80 Hz
    case 4: return 0.2f;
    case 5: return 0.0f;       // Tanh
    case 6: return 0.5f;       // partials
    case 7: return 0.5f;       // mode amount
    case 8: return 0.5f;       // mode rate
    default: return 0.0f;
    }
}

// Human-readable slot readout, matching the units the DSP actually receives.
inline const char* formatParam(int index, int param, float norm, char* buf, std::size_t n)
{
    norm = std::clamp(norm, 0.0f, 1.0f);
    // Compact codes: the 7-arc EQ cell is ~50 px wide at 100% zoom and a
    // full "BandPass" is 50.4 px at the 10 px value tier, i.e. it clips.
    static const char* eqTypes[8] = {"LP", "BP", "HP", "Notch",
                                     "Bell", "LoShelf", "HiShelf", "AP"};
    static const char* slopes[4] = {"12 dB", "24 dB", "48 dB", "96 dB"};
    static const char* satTypes[8] = {"Tanh", "Softsign", "Arctan", "Cubic",
                                      "HardClip", "Foldback", "Asymmetric", "Sine"};
    if (isEQIndex(index)) {
        switch (param) {
        case 0: std::snprintf(buf, n, "%s", eqTypes[(int)(norm * 7.0f + 0.5f)]); return buf;
        case 1: std::snprintf(buf, n, "%s", slopes[(int)(norm * 3.0f + 0.5f)]); return buf;
        case 2: {
            const float hz = 20.0f * std::pow(1000.0f, norm);
            if (hz >= 1000.0f) std::snprintf(buf, n, "%.1fk", hz / 1000.0f);
            else std::snprintf(buf, n, "%.0f Hz", hz);
            return buf;
        }
        case 3: std::snprintf(buf, n, "Q %.2f", 0.1f + norm * 9.9f); return buf;
        case 4: std::snprintf(buf, n, "%+.1f dB", -24.0f + norm * 48.0f); return buf;
        case 5: std::snprintf(buf, n, "%.0f%%", norm * 100.0f); return buf;
        default: std::snprintf(buf, n, "%.2f", norm); return buf;
        }
    }
    if (isHumpIndex(index)) {
        switch (param) {
        case 0: std::snprintf(buf, n, "%.0f%%", norm * 100.0f); return buf;
        case 1: std::snprintf(buf, n, "%.0f%%", norm * 100.0f); return buf;
        case 2: std::snprintf(buf, n, "%s", norm > 0.5f ? "On" : "Off"); return buf;
        case 3: {
            // HumpCrusher cutoff taper: 80 Hz .. 16 kHz.
            const float c = norm;
            const float shaped = (c >= 1.0f) ? 1.0f
                : (c <= 0.0f ? 0.0f : std::clamp(-0.25f * std::log10(1.0f - c), 0.0f, 1.0f));
            const float hz = 80.0f + 15920.0f * shaped;
            if (hz >= 1000.0f) std::snprintf(buf, n, "%.1fk", hz / 1000.0f);
            else std::snprintf(buf, n, "%.0f Hz", hz);
            return buf;
        }
        case 4: std::snprintf(buf, n, "Q %.2f", 1.0f + 6.0f * norm); return buf;
        case 5: std::snprintf(buf, n, "%s", norm > 0.5f ? "On" : "Off"); return buf;
        case 6: {
            const float a = 0.99f * norm;
            const float db = 20.0f * std::log10(1.0f / std::max(1.0f - a, 1.0e-6f));
            std::snprintf(buf, n, "+%.1f dB", db);
            return buf;
        }
        case 7: std::snprintf(buf, n, "%s", norm > 0.5f ? "On" : "Off"); return buf;
        case 8: {
            const float gain = std::pow(std::clamp(norm, 0.0f, 1.0f), 2.737f);
            const float db = 20.0f * std::log10(std::max(gain, 1.0e-6f));
            std::snprintf(buf, n, "%.1f dB", db);
            return buf;
        }
        default: std::snprintf(buf, n, "%.2f", norm); return buf;
        }
    }
    switch (param) {
    case 0: {
        const float hz = 20.0f * std::pow(400.0f, norm);
        if (hz >= 1000.0f) std::snprintf(buf, n, "%.1fk", hz / 1000.0f);
        else std::snprintf(buf, n, "%.0f Hz", hz);
        return buf;
    }
    case 1: std::snprintf(buf, n, "%.0f%%", norm * 99.8f); return buf;
    case 2: {
        const float hz = 200.0f * std::pow(100.0f, norm);
        if (hz >= 1000.0f) std::snprintf(buf, n, "%.1fk", hz / 1000.0f);
        else std::snprintf(buf, n, "%.0f Hz", hz);
        return buf;
    }
    case 3: std::snprintf(buf, n, "%.0f Hz", 20.0f * std::pow(100.0f, norm)); return buf;
    case 4: std::snprintf(buf, n, "%.0f%%", norm * 100.0f); return buf;
    case 5: std::snprintf(buf, n, "%s", satTypes[(int)(norm * 7.0f + 0.5f)]); return buf;
    case 6: std::snprintf(buf, n, "%.0f", 16.0f + std::round(norm * 48.0f)); return buf;
    case 7: std::snprintf(buf, n, "%.0f%%", norm * 100.0f); return buf;
    case 8: {
        const float hz = 0.05f * std::pow(160.0f, norm);
        std::snprintf(buf, n, "%.2f Hz", hz);
        return buf;
    }
    default: std::snprintf(buf, n, "%.2f", norm); return buf;
    }
}
} // namespace CymbalsInternalFx
