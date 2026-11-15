#include "UnifiedFilter.hpp"
#include "DSPUtils.hpp"
#include "airwindows/AirSuiteUniversal.hpp"
#include <cmath>
#include <complex>

// Feedback ceiling for the 4-pole global-feedback ladders. The resonance
// peak is at k = 4; values above it weaken the cutoff peak instead of making
// the filter scream, which reads as a reversed Q at the top of the knob.
static inline float ladderKMax(int slope) {
    return (slope == 0) ? 3.7f : (slope == 1) ? 3.85f : 4.0f;
}
#include <algorithm>
#include <cstring>
#include <cctype>

static constexpr float kMaxSafeFreqRatio = 0.45f;

static inline float jmap(float v, float inMin, float inMax, float outMin, float outMax) {
    float t = std::clamp((v - inMin) / (inMax - inMin), 0.0f, 1.0f);
    return outMin + t * (outMax - outMin);
}

static inline float jlimit(float lo, float hi, float v) {
    return std::clamp(v, lo, hi);
}

UnifiedFilter::~UnifiedFilter() = default;

UnifiedFilter::UnifiedFilter() : chL{}, chR{} {
    for (int s = 0; s < 8; ++s) {
        chL.wf_F_z1[s] = -1.0;
        chR.wf_F_z1[s] = -1.0;
    }
    chL.agcGain = 1.0f;
    chR.agcGain = 1.0f;
}

static const int kAirModelTypes[UnifiedFilter::AirwindowsCount] = {
    // Airwindows FILTER 112..122
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
    // BIQUAD 127..134
    127, 128, 129, 130, 131, 132, 133, 134,
    // XYZ 236..246
    236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246
};

int UnifiedFilter::airwindowsTypeForModel(int m) {
    if (!isAirwindowsModel(m)) return 0;
    return kAirModelTypes[m - AirwindowsFirst];
}

uint32_t UnifiedFilter::supportedTypesMask(int model) {
    constexpr uint32_t LP = 1u << 0;
    constexpr uint32_t BP = 1u << 1;
    constexpr uint32_t HP = 1u << 2;
    constexpr uint32_t NOTCH = 1u << 3;
    constexpr uint32_t ALL_FILTER = LP | BP | HP | NOTCH;
    if (isAirwindowsModel(model)) {
        if (model >= AirwindowsFirst + 11 && model < AirwindowsFirst + 19)
            return ALL_FILTER; // BIQUAD algorithms carry their own Type control
        switch (airwindowsTypeForModel(model)) {
            case (int)DISTRHO::AirSuiteUniversal::XYZ_XBANDPASS:
            case (int)DISTRHO::AirSuiteUniversal::XYZ_YBANDPASS:
            case (int)DISTRHO::AirSuiteUniversal::XYZ_ZBANDPASS:
            case (int)DISTRHO::AirSuiteUniversal::XYZ_XREGION:
                return BP;
            case (int)DISTRHO::AirSuiteUniversal::XYZ_XHIGHPASS:
            case (int)DISTRHO::AirSuiteUniversal::XYZ_YHIGHPASS:
            case (int)DISTRHO::AirSuiteUniversal::XYZ_ZHIGHPASS:
            case (int)DISTRHO::AirSuiteUniversal::FILTER_HIGHPASS:
            case (int)DISTRHO::AirSuiteUniversal::FILTER_HIGHPASS2:
                return HP;
            case (int)DISTRHO::AirSuiteUniversal::XYZ_XNOTCH:
            case (int)DISTRHO::AirSuiteUniversal::XYZ_YNOTCH:
                return NOTCH;
            default:
                return ALL_FILTER;
        }
    }
    switch (model) {
        case 0: return ALL_FILTER | EQ;
        case 3: case 4: case 6: case 7:
        case 9: case 16: case 25: return ALL_FILTER;
        case 1: case 12: return LP | BP | HP;
        case 2: case 13: case 15: case 21: case 27: return LP;
        case 5:  return BP;
        case 8:  return LP | BP;
        case 10: return LP;
        case 11: return LP;
        case 14: return LP | HP;
        case 17: case 18: case 19: case 20: return ALL_FILTER;
        case 22: return BP;
        case 23: return LP | BP;
        case 24: return LP;
        case 26: return LP | HP;
        case 28: return 0;
        case 29: return LP | HP;
        case 30: return LP;
        case 31: return ALL_FILTER;
        case 32: return LP | BP | HP;
        case 33: return LP;
        default: return ALL_FILTER;
    }
}

void UnifiedFilter::setModel(int m) {
    const int newModel = std::clamp(m, 0, (int)Count - 1);
    if (newModel == m_model) return;
    m_model = newModel;
    // Model 0 skips applyAGC entirely; start any other model from a clean AGC
    // state so a live model switch cannot inherit stale gain/RMS.
    chL.agcGain = 1.0f;
    chR.agcGain = 1.0f;
    chL.rmsOut = 0.0f;
    chR.rmsOut = 0.0f;
}
void UnifiedFilter::setType(int t) { m_type = std::clamp(t, 0, 7); }
void UnifiedFilter::setCutoffHz(float hz) { m_cutoffHz = jlimit(20.0f, 20000.0f, hz); }
void UnifiedFilter::setResonance(float r) { m_resonance = jlimit(0.1f, 10.0f, r); }
void UnifiedFilter::setGainDb(float db) { m_gainDb = std::clamp(db, -48.0f, 48.0f); }
void UnifiedFilter::setSampleRate(float sr) { m_sampleRate = sr; }

void UnifiedFilter::setSlope(int s) {
    setSlopeInternal(s);
}

void UnifiedFilter::setSlopeInternal(int s) {
    m_slope = std::clamp(s, 0, 3);
    const int m = m_model;
    if (m == 2) {
        m_stages = 1; m_order = 4;
    } else if (m == 5 || m == 10 || m == 21 || m == 22 || m == 24 || m == 25) {
        m_stages = 1; m_order = 2;
    } else if (m == 27) {
        m_stages = (m_slope == 0) ? 2 : (m_slope == 1) ? 4 : (m_slope == 2) ? 6 : 8;
        m_order = m_stages * 2;
    } else if (m == NoneModel || isAirwindowsModel(m) ||
               (m >= SstKorg35 && m <= SstCytomicTilt)) {
        m_stages = 1;
        m_order = (m == SstTriPole) ? 6 : (m == SstDiodeLadder) ? 8 : 4;
    } else if (m == 8 || m == 11 || m == 26) {
        m_stages = (m_slope == 0) ? 2 : (m_slope == 1) ? 4 : (m_slope == 2) ? 8 : 16;
        m_order = m_stages * 2;
    } else if (m >= 17 && m <= 20) {
        m_order = (m_slope == 0) ? 2 : (m_slope == 1) ? 4 : (m_slope == 2) ? 8 : 16;
        m_stages = m_order / 2;
    } else if (m == 23) {
        m_stages = (m_slope == 0) ? 1 : (m_slope == 1) ? 2 : (m_slope == 2) ? 4 : 8;
        m_order = m_stages * 2;
    } else if (m == 0 || m == 3 || m == 4 || m == 6 || m == 7 || m == 9 || m == 14 || m == 16) {
        m_stages = (m_slope == 0) ? 1 : (m_slope == 1) ? 2 : (m_slope == 2) ? 4 : 8;
        m_order = m_stages * 2;
    } else {
        m_stages = (m_slope == 0) ? 1 : (m_slope == 1) ? 1 : (m_slope == 2) ? 2 : 4;
        m_order = m_stages * 4;
    }
    m_scalerSVF = (m_stages > 1) ? std::pow(0.6f, std::log2((float)m_stages)) : 1.0f;
    m_scalerMoog = (m_stages > 1) ? std::pow(0.7f, std::log2((float)m_stages)) : 1.0f;
}

void UnifiedFilter::prepare() {
    updateCoefficients();
    if (m_type >= 4) updateBiquadCoeffs();
    if (m_model >= SstKorg35 && m_model <= SstCytomicTilt) prepareSstModels();
    if (isAirwindowsModel(m_model)) prepareAirwindowsModel();
}

void UnifiedFilter::reset() {
    chL = ChannelState{};
    chR = ChannelState{};
    for (int s = 0; s < 8; ++s) {
        chL.wf_F_z1[s] = -1.0;
        chR.wf_F_z1[s] = -1.0;
    }
    chL.agcGain = 1.0f;
    chR.agcGain = 1.0f;
    if (m_air) m_air->reset();
    m_bqStateL[0] = m_bqStateL[1] = 0.0f;
    m_bqStateR[0] = m_bqStateR[1] = 0.0f;
}

void UnifiedFilter::updateBiquadCoeffs() {
    if (m_model == NoneModel) return;
    const float w0 = 2.0f * DSPUtils::kPi * m_cutoffHz / m_sampleRate;
    const float cosW0 = std::cos(w0);
    const float sinW0 = std::sin(w0);
    const float A = std::pow(10.0f, m_gainDb / 40.0f);
    const float alpha = sinW0 / (2.0f * std::max(m_resonance, 1.0e-6f));
    float b0r = 0.0f, b1r = 0.0f, b2r = 0.0f, a0 = 1.0f, a1r = 0.0f, a2r = 0.0f;

    switch (m_type) {
        case 4: // Bell
            b0r = 1.0f + alpha * A;
            b1r = -2.0f * cosW0;
            b2r = 1.0f - alpha * A;
            a0 = 1.0f + alpha / A;
            a1r = -2.0f * cosW0;
            a2r = 1.0f - alpha / A;
            break;
        case 5: // LowShelf
            {
                float sqrtA2a = 2.0f * std::sqrt(A) * alpha;
                b0r = A * ((A + 1.0f) - (A - 1.0f) * cosW0 + sqrtA2a);
                b1r = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosW0);
                b2r = A * ((A + 1.0f) - (A - 1.0f) * cosW0 - sqrtA2a);
                a0 = (A + 1.0f) + (A - 1.0f) * cosW0 + sqrtA2a;
                a1r = -2.0f * ((A - 1.0f) + (A + 1.0f) * cosW0);
                a2r = (A + 1.0f) + (A - 1.0f) * cosW0 - sqrtA2a;
            }
            break;
        case 6: // HighShelf
            {
                float sqrtA2a = 2.0f * std::sqrt(A) * alpha;
                b0r = A * ((A + 1.0f) + (A - 1.0f) * cosW0 + sqrtA2a);
                b1r = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosW0);
                b2r = A * ((A + 1.0f) + (A - 1.0f) * cosW0 - sqrtA2a);
                a0 = (A + 1.0f) - (A - 1.0f) * cosW0 + sqrtA2a;
                a1r = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosW0);
                a2r = (A + 1.0f) - (A - 1.0f) * cosW0 - sqrtA2a;
            }
            break;
        case 7: // AllPass
            b0r = 1.0f - alpha;
            b1r = -2.0f * cosW0;
            b2r = 1.0f + alpha;
            a0 = 1.0f + alpha;
            a1r = -2.0f * cosW0;
            a2r = 1.0f - alpha;
            break;
        default:
            b0r = 1.0f; b1r = 0.0f; b2r = 0.0f; a1r = 0.0f; a2r = 0.0f;
            break;
    }
    m_bqB0 = b0r / a0;
    m_bqB1 = b1r / a0;
    m_bqB2 = b2r / a0;
    m_bqA1 = a1r / a0;
    m_bqA2 = a2r / a0;
}

void UnifiedFilter::updateCoefficients() {
    if (m_model == NoneModel) return;
    float maxSafeFreq = m_sampleRate * kMaxSafeFreqRatio;
    float safeCutoff = jlimit(20.0f, maxSafeFreq, m_cutoffHz);
    float wd = DSPUtils::kPi * safeCutoff / m_sampleRate;
    m_g = std::tan(wd);
    m_ladderG = m_g / (1.0f + m_g);

    const int m = m_model;
    float adjRes = m_resonance;
    if (m == 0 || m == 6 || m == 9 || m == 14 || m == 16 || m == 23 || m == 26 || m == 27) {
        adjRes = m_resonance * ((m != 14) ? m_scalerSVF : 1.0f);
        m_R = 1.0f / (2.0f * adjRes);
        m_h = 1.0f / (1.0f + 2.0f * m_R * m_g + m_g * m_g);
    } else if (m == 3) {
        m_R = 1.0f / (2.0f * m_resonance * m_scalerSVF);
        m_h = 1.0f / (1.0f + m_g * m_g);
    } else if (m == 7) {
        m_R = 1.0f / (2.0f * m_resonance);
        m_h = 1.0f / (1.0f + 2.0f * m_R * m_g + m_g * m_g);
    } else if (m == 10) {
        m_R = 1.0f / (2.0f * 1.2f);
        m_h = 1.0f / (1.0f + 2.0f * m_R * m_g + m_g * m_g);
    } else if (m == 1 || m == 12 || m == 13 || m == 15) {
        float r_scale = (m == 13) ? 5.0f : 4.5f;
        m_ladderRes = jmap(m_resonance, 0.1f, 10.0f, 0.0f, r_scale) * m_scalerMoog;
        m_R = 1.0f / (2.0f * m_resonance);
        m_h = 1.0f / (1.0f + 2.0f * m_R * m_g + m_g * m_g);
    } else if (m == 2) {
        m_R = 1.0f / (2.0f * m_resonance);
        m_h = 1.0f / (1.0f + 2.0f * m_R * m_g + m_g * m_g);
    } else if (m == 4) {
        float bits = jmap(m_resonance, 0.1f, 10.0f, 16.0f, 2.0f);
        chL.srrBitSteps = std::pow(2.0f, bits);
        chR.srrBitSteps = chL.srrBitSteps;
        m_R = 1.0f / (2.0f * m_resonance * m_scalerSVF);
        m_h = 1.0f / (1.0f + 2.0f * m_R * m_g + m_g * m_g);
    } else if (m == 5 || m == 22) {
        float adj = m_resonance;
        m_R = 1.0f / (2.0f * adj);
        m_h = 1.0f / (1.0f + 2.0f * m_R * m_g + m_g * m_g);
    } else if (m == 21) {
        m_R = 1.0f / (2.0f * 0.707f);
        m_h = 1.0f / (1.0f + 2.0f * m_R * m_g + m_g * m_g);
    } else if (m >= 17 && m <= 20) {
        for (int k = 0; k < m_stages; ++k) {
            dpCoeffs[k].g = m_g;
            dpCoeffs[k].R = (m == 17) ? 0.70710678f : (m == 19) ? 0.57735027f : 1.0f / (2.0f * m_resonance);
            dpCoeffs[k].h = 1.0f / (1.0f + 2.0f * dpCoeffs[k].R * dpCoeffs[k].g + dpCoeffs[k].g * dpCoeffs[k].g);
        }
    } else if (m == 11 || m == 25) {
        m_R = 1.0f / (2.0f * m_resonance);
        m_h = 1.0f / (1.0f + 2.0f * m_R * m_g + m_g * m_g);
    } else {
        m_R = 1.0f / (2.0f * m_resonance);
        m_h = 1.0f / (1.0f + 2.0f * m_R * m_g + m_g * m_g);
    }
}

float UnifiedFilter::processSampleL(float input) { return processSampleImpl(input, chL); }
float UnifiedFilter::processSampleR(float input) { return processSampleImpl(input, chR); }

void UnifiedFilter::processStereoSample(float& inOutL, float& inOutR) {
    if (isAirwindowsModel(m_model)) {
        if (m_air) m_air->process(inOutL, inOutR, m_airParams);
        return;
    }
    inOutL = processSampleImpl(inOutL, chL);
    inOutR = processSampleImpl(inOutR, chR);
}

float UnifiedFilter::processSampleImpl(float x, ChannelState& st) {
    const int m = m_model;
    const int t = m_type;

    // NoneModel: clean bypass, no processing
    if (m == NoneModel) return x;

    // Saturation drive: pre-filter gain boost
    if (m_saturation > 0.0f)
        x *= 1.0f + m_saturation * 3.0f;

    // Ported sst-filters models own their gain staging.
    if (m >= SstKorg35 && m <= SstCytomicTilt) return processSstModel(x, st);

    // EQ types (Bell, LowShelf, HighShelf, AllPass) use biquad regardless of model
    if (t >= 4) {
        float* state = (&st == &chL) ? m_bqStateL : m_bqStateR;
        float out = m_bqB0 * x + state[0];
        state[0] = m_bqB1 * x - m_bqA1 * out + state[1];
        state[1] = m_bqB2 * x - m_bqA2 * out;
        return out;
    }

    float comp = 1.0f;
    if (m == 0 || m == 4 || m == 5 || m == 6 || m == 9 || m == 11 || m == 16 || m == 26)
        comp = 1.0f + m_resonance * 0.1f;
    else if (m == 1 || m == 12 || m == 13 || m == 15)
        comp = 1.0f + 0.5f * m_ladderRes;
    else if (m == 2) {
        const float k_max = ladderKMax(m_slope);
        const float k_val = jmap(m_resonance, 0.1f, 10.0f, 0.0f, k_max);
        comp = 1.0f + 0.2f * k_val;
    } else if (m == 7) {
        comp = 1.0f + m_resonance * 0.15f;
    }
    x *= comp;

    if (m == 4) {
        float phaseInc = m_cutoffHz / m_sampleRate;
        st.srrPhase += phaseInc;
        if (st.srrPhase >= 1.0f) {
            st.srrPhase -= std::floor(st.srrPhase);
            st.srrHeld = std::round(x * st.srrBitSteps) / st.srrBitSteps;
        }
        x = st.srrHeld;
    }

    st.rmsIn = (1.0f - 0.005f) * st.rmsIn + 0.005f * (x * x);

    float out = 0.0f;
    int ch = (&st == &chL) ? 0 : 1;

    switch (m) {
        case 0: case 4:
            out = processSVF(ch, x, st); break;
        case 1:
            out = processLadder(ch, x, st); break;
        case 2:
            out = processTB303(ch, x, st); break;
        case 3:
            out = processSEM(ch, x, st); break;
        case 5: case 21: case 22: case 27:
            out = processAnalogEmulation(ch, x, st); break;
        case 6:
            out = processComb(ch, x, st); break;
        case 7:
            out = processMS20(ch, x, st); break;
        case 8: case 11: case 24: case 25: case 26:
            out = processSpectral(ch, x, st); break;
        case 9:
            out = processWavefolder(ch, x, st); break;
        case 10:
            out = processFDNReverb(ch, x, st); break;
        case 12:
            out = processCEM3320(ch, x, st); break;
        case 13:
            out = processSSM2040(ch, x, st); break;
        case 14:
            out = processCS80(ch, x, st); break;
        case 15:
            out = processJupiter(ch, x, st); break;
        case 16:
            out = processPhasedArray(ch, x, st); break;
        case 17: case 18: case 19: case 20:
            out = processDigitalPrecision(ch, x, st); break;
        case 23:
            out = processWaveguideMesh(ch, x, st); break;
        default:
            out = x; break;
    }

    return applyAGC(ch, out, st);
}

float UnifiedFilter::applyAGC(int /*ch*/, float out, ChannelState& st) {
    // CleanSVF forces finalGain to 1.0 below; the whole AGC pass would be dead
    // weight. setModel() resets the AGC state when leaving/entering model 0.
    if (m_model == 0) return out;
    st.rmsOut = (1.0f - 0.005f) * st.rmsOut + 0.005f * (out * out);
    float targetGain = 1.0f;
    if (st.rmsOut > 1e-8f) {
        targetGain = std::sqrt((st.rmsIn + 1e-8f) / (st.rmsOut + 1e-8f));
        targetGain = jlimit(0.1f, 15.0f, targetGain);
    }
    st.agcGain = (1.0f - 0.0005f) * st.agcGain + 0.0005f * targetGain;
    float finalGain = st.agcGain;
    if (m_model == 0) {
        finalGain = 1.0f;
    } else if (m_model == 1 || m_model == 2 || m_model == 12 || m_model == 13 || m_model == 15) {
        float k_norm;
        if (m_model == 2) {
            k_norm = jmap(m_resonance, 0.1f, 10.0f, 0.0f, 1.0f);
        } else {
            float r_scale = (m_model == 13) ? 5.0f : 4.5f;
            k_norm = (r_scale > 0.0f) ? (m_ladderRes / r_scale) : 0.0f;
        }
        k_norm = jlimit(0.0f, 1.0f, k_norm);
        if (k_norm > 0.85f) {
            float bypassFactor = jlimit(0.0f, 1.0f, (k_norm - 0.85f) * 10.0f);
            st.agcGain = st.agcGain * (1.0f - bypassFactor * 0.01f) + 1.0f * (bypassFactor * 0.01f);
            finalGain = st.agcGain * (1.0f - bypassFactor) + 1.0f * bypassFactor;
        }
    }
    return out * finalGain;
}

// ==== sst-filters ports (GPL-3.0-or-later, Surge Synth Team) ================
// Scalar per-sample adaptations of sst-filters' K35, diode ladder, OB-Xd,
// TriPole and Cytomic tilt. Coefficient formulas and topology are kept
// identical; the SIMD/interpolation scaffolding is not needed here because
// UnifiedFilter calls one sample at a time.

void UnifiedFilter::prepareSstModels() {
    const float sr = std::max(1.0f, m_sampleRate);
    const float cutoff = std::clamp(m_cutoffHz, 5.0f, sr * 0.3f);
    const float res01 = std::clamp((m_resonance - 0.1f) / 9.9f, 0.0f, 1.0f);
    const float g = std::tan(DSPUtils::kPi * cutoff / sr);

    if (m_model == SstKorg35) {
        const float gp1 = 1.0f + g;
        const float G = g / gp1;
        const float k = std::clamp(res01 * 1.96f, 0.01f, 1.96f);
        m_k35G = G;
        m_k35K = k;
        if (m_type == 2) { // HP
            m_k35Lb = 1.0f / gp1;
            m_k35Hb = -G / gp1;
        } else { // LP
            m_k35Lb = (k - k * G) / gp1;
            m_k35Hb = -1.0f / gp1;
        }
        m_k35Alpha = 1.0f / (1.0f - k * G + k * G * G);
    } else if (m_model == SstDiodeLadder) {
        m_diodeG = g;
        const float G4 = 0.5f * g / (1.0f + g);
        const float G3 = 0.5f * g / (1.0f + g - 0.5f * g * G4);
        const float G2 = 0.5f * g / (1.0f + g - 0.5f * g * G3);
        const float G1 = g / (1.0f + g - g * G2);
        m_diodeG1 = G1; m_diodeG2 = G2; m_diodeG3 = G3; m_diodeG4 = G4;
        m_diodeGamma = G4 * G3 * G2 * G1;
        m_diodeAlpha = g / (1.0f + g);
        m_diodeKm = std::clamp(res01 * 16.0f, 0.0f, 16.0f);
    } else if (m_model == SstOBXd) {
        const float rcrate = std::sqrt(44000.0f / sr);
        const float rad = std::min(cutoff, 22000.0f) / sr * DSPUtils::kPi;
        m_obxdG12 = std::tan(rad);
        m_obxdR12 = 1.0f - res01;
        m_obxdG24 = m_obxdG12;
        m_obxdR24 = 3.5f * res01;
        m_obxdRcor = (970.0f / 44000.0f) * rcrate;
        m_obxdRcorInv = 1.0f / m_obxdRcor;
    } else if (m_model == SstTriPole) {
        constexpr float capVal = 220e-12f;
        const float T = 1.0f / sr;
        const float wc = 2.0f * DSPUtils::kPi * cutoff / sr;
        const float gg = (capVal / T) * (std::exp(wc) - 1.0f);
        m_triB[0] = 0.998f * T * gg / capVal;
        m_triA[0] = 1.0f / (1.0f + m_triB[0]);
        m_triB[1] = 1.0012f * T * gg / capVal;
        m_triA[1] = 1.0f / (1.0f + m_triB[1]);
        m_triB[2] = T * gg / capVal;
        m_triA[2] = 1.0f / (1.0f + m_triB[2]);
        constexpr float resFactorDb = 3.245f; // log10(Iabc*Rload) = log10(1760)
        m_triK = -(std::pow(10.0f, resFactorDb * res01) + 1.0f);
    } else if (m_model == SstCytomicTilt) {
        const float conorm = std::clamp(cutoff / sr, 0.0f, 0.499f);
        const float res = std::clamp(res01, 0.0f, 0.98f);
        const float tg = std::tan(DSPUtils::kPi * conorm);
        const float k = 2.0f - 2.0f * res;
        const float gk = tg + k;
        m_tiltA1 = 1.0f / (1.0f + tg * gk);
        m_tiltA2 = tg * m_tiltA1;
        m_tiltA3 = tg * m_tiltA2;
        const float A = std::pow(10.0f, m_gainDb / 40.0f);
        const float invA = 1.0f / A;
        m_tiltM0Lo = 1.0f;
        m_tiltM1Lo = k * (invA - 1.0f);
        m_tiltM2Lo = invA * invA - 1.0f;
        m_tiltM0Hi = A * A;
        m_tiltM1Hi = k * (1.0f - A) * A;
        m_tiltM2Hi = 1.0f - m_tiltM0Hi;
    }
}

static inline float sstK35Lpf(float G, float in, float& z) {
    const float v = (in - z) * G;
    const float res = v + z;
    z = v + res;
    return res;
}
static inline float sstK35Hpf(float G, float in, float& z) {
    return in - sstK35Lpf(G, in, z);
}

float UnifiedFilter::processKorg35(float x, ChannelState& st) {
    if (m_k35K <= 1.0e-6f) return 0.0f;
    if (m_type == 2) {
        const float y1 = sstK35Hpf(m_k35G, x, st.k35_hz);
        const float s35 = m_k35Hb * st.k35_2z + m_k35Lb * st.k35_lz;
        const float u = m_k35Alpha * (y1 + s35);
        const float y = m_k35K * u;
        sstK35Lpf(m_k35G, sstK35Hpf(m_k35G, y, st.k35_2z), st.k35_lz);
        return y / m_k35K;
    }
    const float y1 = sstK35Lpf(m_k35G, x, st.k35_lz);
    const float s35 = m_k35Lb * st.k35_2z + m_k35Hb * st.k35_hz;
    const float u = m_k35Alpha * (y1 + s35);
    const float y = m_k35K * sstK35Lpf(m_k35G, u, st.k35_2z);
    sstK35Hpf(m_k35G, y, st.k35_hz);
    return y / m_k35K;
}

static inline float diodeLpf(float in, float alpha, float gamma, float epsilon, float ma0,
                             float feedback, float fbOut, float& z) {
    const float i = in * gamma + feedback + epsilon * fbOut;
    const float v = (ma0 * i - z) * alpha;
    const float res = v + z;
    z = v + res;
    return res;
}

float UnifiedFilter::processDiodeLadder(float x, ChannelState& st) {
    const float g = m_diodeG, hg = 0.5f * g, gp1 = 1.0f + g;
    const float beta1 = 1.0f / (gp1 - g * m_diodeG2);
    const float beta2 = 1.0f / (gp1 - hg * m_diodeG3);
    const float beta3 = 1.0f / (gp1 - hg * m_diodeG4);
    const float beta4 = 1.0f / gp1;
    const float gamma1 = m_diodeG1 * m_diodeG2 + 1.0f;
    const float gamma2 = m_diodeG2 * m_diodeG3 + 1.0f;
    const float gamma3 = m_diodeG3 * m_diodeG4 + 1.0f;
    auto getFO = [](float beta, float delta, float fb, float z) { return (fb * delta + z) * beta; };
    const float fb3 = getFO(beta4, 0.0f, 0.0f, st.diode_z[3]);
    const float fb2 = getFO(beta3, hg, st.diode_fb[2], st.diode_z[2]);
    const float fb1 = getFO(beta2, hg, st.diode_fb[1], st.diode_z[1]);
    const float sigma = m_diodeG4 * getFO(beta1, g, fb1, st.diode_z[0])
                      + (m_diodeG4 * m_diodeG3) * getFO(beta2, hg, fb2, st.diode_z[1])
                      + (m_diodeG4 * m_diodeG3 * m_diodeG2) * getFO(beta3, hg, fb3, st.diode_z[2])
                      + getFO(beta4, 0.0f, 0.0f, st.diode_z[3]);
    st.diode_fb[2] = fb3;
    st.diode_fb[1] = fb2;
    st.diode_fb[0] = fb1;
    const float comp = (0.3f * m_diodeKm + 1.0f) * x;
    const float u = (comp - m_diodeKm * sigma) / (m_diodeKm * m_diodeGamma + 1.0f);
    const float r1 = diodeLpf(u, m_diodeAlpha, gamma1, m_diodeG2, 1.0f, fb1,
                              getFO(beta1, g, fb1, st.diode_z[0]), st.diode_z[0]);
    const float r2 = diodeLpf(r1, m_diodeAlpha, gamma2, m_diodeG3, 0.5f, fb2,
                              getFO(beta2, hg, fb2, st.diode_z[1]), st.diode_z[1]);
    const float r3 = diodeLpf(r2, m_diodeAlpha, gamma3, m_diodeG4, 0.5f, fb3,
                              getFO(beta3, hg, fb3, st.diode_z[2]), st.diode_z[2]);
    const float r4 = diodeLpf(r3, m_diodeAlpha, 1.0f, 0.0f, 0.5f, 0.0f,
                              getFO(beta4, 0.0f, 0.0f, st.diode_z[3]), st.diode_z[3]);
    switch (std::clamp(m_slope, 0, 3)) {
        case 0: return r1 * 0.125f;
        case 1: return r2 * 0.3f;
        case 2: return r3 * 0.6f;
        default: return r4 * 1.2f;
    }
}

static inline float obxdDiodeResistance(float x) {
    return (((((0.0103592f * x) + 0.00920833f) * x + 0.185f) * x + 0.05f) * x + 1.0f);
}

float UnifiedFilter::processOBXd(float x, ChannelState& st) {
    const bool fourPole = (m_slope > 0 && m_type == 0);
    if (fourPole) {
        const float lpc = m_obxdG24 / (1.0f + m_obxdG24);
        const float ml = 1.0f / (1.0f + m_obxdG24);
        const float S = (lpc * (lpc * (lpc * st.obxd_s[0] + st.obxd_s[1]) + st.obxd_s[2])
                         + st.obxd_s[3]) * ml;
        const float G = lpc * lpc * lpc * lpc;
        const float y0 = (x - m_obxdR24 * S) / (1.0f + m_obxdR24 * G);
        const float v = (y0 - st.obxd_s[0]) * lpc;
        const float res = v + st.obxd_s[0];
        st.obxd_s[0] = res + v;
        st.obxd_s[0] = std::atan(st.obxd_s[0] * m_obxdRcor) * m_obxdRcorInv;
        auto tptpc = [&](float& state, float in) {
            const float vv = (in - state) * m_obxdG24 / (1.0f + m_obxdG24);
            const float rr = vv + state;
            state = rr + vv;
            return rr;
        };
        const float y1 = res;
        const float y2 = tptpc(st.obxd_s[1], y1);
        const float y3 = tptpc(st.obxd_s[2], y2);
        const float y4 = tptpc(st.obxd_s[3], y3);
        return y4;
    }
    const float tCfb = obxdDiodeResistance(st.obxd_s[0] * 0.0876f) - 1.0f;
    const float y = (x - 2.0f * (st.obxd_s[0] * (m_obxdR12 + tCfb)) - m_obxdG12 * st.obxd_s[0]
                     - st.obxd_s[1])
                  / (1.0f + m_obxdG12 * (2.0f * (m_obxdR12 + tCfb) + m_obxdG12));
    const float y1 = y * m_obxdG12 + st.obxd_s[0];
    st.obxd_s[0] = y * m_obxdG12 + y1;
    const float y2 = y1 * m_obxdG12 + st.obxd_s[1];
    st.obxd_s[1] = y1 * m_obxdG12 + y2;
    float multimode = 0.0f, bandpass = 0.0f;
    if (m_type == 1) { multimode = 0.5f; bandpass = 1.0f; }
    else if (m_type == 2) multimode = 1.0f;
    else if (m_type == 3) multimode = 0.5f;
    float mc;
    if (bandpass != 0.0f) {
        const float val1 = (0.5f - multimode) * y2 + multimode * y1;
        const float val2 = (1.0f - multimode) * y1 + (multimode - 0.5f) * y;
        mc = (multimode < 0.5f) ? val1 : val2;
    } else {
        mc = (1.0f - multimode) * y2 + multimode * y;
    }
    return mc * 0.74f;
}

static inline float triSigmoid(float x, float beta) {
    return x / std::sqrt(x * x + beta);
}
static inline float triLpLin(float tx, float z, float b, float a) { return a * (b * tx + z); }
[[maybe_unused]] static inline float triHpLin(float xmz, float a) { return a * xmz; }
static inline float triSech2(float t) { return 1.0f - t * t; }
static inline float triLpStep(float tx, float z, float est, float b, float a, float beta) {
    est = triLpLin(tx, z, b, a);
    const float ty = triSigmoid(est, beta);
    const float residue = (b * (tx - ty) + z) - est;
    const float deriv = -b * triSech2(ty) - 1.0f;
    return est - residue / deriv;
}
static inline float triLpFbStep(float tx, float z, float fb, float fb1, float est,
                                float b, float a, float bx, float beta) {
    const float zmf = z - fb + fb1;
    est = a * (bx + zmf);
    const float ty = triSigmoid(est, beta);
    const float residue = (b * (tx - ty) + zmf) - est;
    const float deriv = -b * triSech2(ty) - 1.0f;
    return est - residue / deriv;
}
static inline float triHpStep(float x, float x1, float z, float est, float b, float a, float beta) {
    const float xmz = (x - x1) + z;
    est = a * xmz;
    const float ty = triSigmoid(est, beta);
    const float residue = (-b * ty + xmz) - est;
    const float deriv = -b * triSech2(ty) - 1.0f;
    return est - residue / deriv;
}
static inline float triHpFbStep(float xmz, float tanhFb, float est, float b, float a, float beta) {
    est = a * (b * tanhFb + xmz);
    const float ty = triSigmoid(est, beta);
    const float residue = (b * (tanhFb - ty) + xmz) - est;
    const float deriv = -b * triSech2(ty) - 1.0f;
    return est - residue / deriv;
}
static inline float triResFunc(float x) {
    x *= 10.0f;
    if (std::fabs(x) < 7.5f) return x * 0.099f;
    constexpr float beta = 9.03240196f;
    constexpr float alpha = 1.0168177f;
    constexpr float c = 0.222161f;
    constexpr float bias = 8.2f;
    const float e = std::exp(-(beta * std::log(alpha)) * std::fabs(x + c)) + bias;
    return (x < 0.0f ? -1.0f : 1.0f) * e * 0.099f;
}
static inline float triResDeriv(float x) {
    x *= 10.0f;
    if (std::fabs(x) < 7.5f) return 1.0f;
    constexpr float beta = 9.03240196f;
    constexpr float alpha = 1.0168177f;
    constexpr float c = 0.222161f;
    const float e = std::exp(-(beta * std::log(alpha)) * std::fabs(x + c));
    return e + (beta * std::log(alpha)) / 10.0f;
}

float UnifiedFilter::processTriPole(float x, ChannelState& st) {
    constexpr float inGain = 4.0f, outGain = 0.25f, resGain = 1.5f;
    constexpr float ota1bp = 0.88f, ota1bn = 1.0f;
    constexpr float ota2bn = 0.97f, ota3bn = 1.025f;
    x *= inGain;
    const bool hp = (m_type == 2);
    const bool bp = (m_type == 1); // middle tap
    float tx0 = 0.0f, tanhFb = 0.0f, bx = 0.0f, hpfIn = 0.0f;
    if (!hp) {
        tx0 = triSigmoid(x, ota1bp);
        bx = m_triB[0] * tx0;
    } else {
        hpfIn = (x - st.tri_x[0]) + st.tri_z[0];
    }
    float estimate = st.tri_fb;
    float est0 = st.tri_z[0], est1 = st.tri_z[1], est2 = st.tri_z[2];
    float resOut = st.tri_x[2];
    for (int i = 0; i < 3; ++i) {
        if (!hp)
            est0 = triLpFbStep(tx0, st.tri_z[0], estimate, st.tri_fb1, est0, m_triB[0], m_triA[0], bx, ota1bn);
        else {
            tanhFb = triSigmoid(estimate, ota1bp);
            est0 = triHpFbStep(hpfIn, tanhFb, est0, m_triB[0], m_triA[0], ota1bn);
        }
        if (!hp)
            est1 = triLpStep(triSigmoid(est0, 0.9f), st.tri_z[1], est1, m_triB[1], m_triA[1], ota2bn);
        else
            est1 = triHpStep(est0, st.tri_x[1], st.tri_z[1], est1, m_triB[1], m_triA[1], ota2bn);
        const float kf = m_triK * est1;
        resOut = triResFunc(kf) / resGain;
        const float rd = triResDeriv(kf);
        if (!hp)
            est2 = triLpStep(triSigmoid(resOut, 0.95f), st.tri_z[2], est2, m_triB[2], m_triA[2], ota3bn);
        else
            est2 = triHpStep(resOut, st.tri_x[2], st.tri_z[2], est2, m_triB[2], m_triA[2], ota3bn);
        const float f0 = 2.0f, f1 = 2.0f, f2 = 2.0f;
        const float den = 1.0f - m_triK * rd * f0 * f1 * f2;
        estimate = estimate - (estimate - est2) / den;
    }
    st.tri_z[0] = est0; st.tri_x[0] = x;
    st.tri_z[1] = est1; st.tri_x[1] = est0;
    st.tri_z[2] = est2; st.tri_x[2] = resOut;
    st.tri_fb1 = st.tri_fb;
    st.tri_fb = estimate;
    const float out = bp ? est1 : (hp ? est2 : estimate);
    return out * outGain;
}

float UnifiedFilter::processCytomicTilt(float x, ChannelState& st) {
    const float v3 = x - st.tilt_ic2Lo;
    const float v1 = m_tiltA1 * st.tilt_ic1Lo + m_tiltA2 * v3;
    const float v2 = st.tilt_ic2Lo + m_tiltA2 * st.tilt_ic1Lo + m_tiltA3 * v3;
    st.tilt_ic1Lo = 2.0f * v1 - st.tilt_ic1Lo;
    st.tilt_ic2Lo = 2.0f * v2 - st.tilt_ic2Lo;
    const float ls = m_tiltM0Lo * x + m_tiltM1Lo * v1 + m_tiltM2Lo * v2;
    const float nv3 = ls - st.tilt_ic2Hi;
    const float nv1 = m_tiltA1 * st.tilt_ic1Hi + m_tiltA2 * nv3;
    const float nv2 = st.tilt_ic2Hi + m_tiltA2 * st.tilt_ic1Hi + m_tiltA3 * nv3;
    st.tilt_ic1Hi = 2.0f * nv1 - st.tilt_ic1Hi;
    st.tilt_ic2Hi = 2.0f * nv2 - st.tilt_ic2Hi;
    return m_tiltM0Hi * ls + m_tiltM1Hi * nv1 + m_tiltM2Hi * nv2;
}

// ---- Airwindows EQ models -------------------------------------------------
static inline bool ciHas(const char* hay, const char* needle) {
    if (hay == nullptr || needle == nullptr) return false;
    const size_t n = std::strlen(needle);
    for (const char* p = hay; *p; ++p) {
        size_t i = 0;
        while (i < n && p[i] &&
               std::tolower((unsigned char)p[i]) == std::tolower((unsigned char)needle[i]))
            ++i;
        if (i == n) return true;
    }
    return false;
}

void UnifiedFilter::prepareAirwindowsModel() {
    const int airType = airwindowsTypeForModel(m_model);
    if (airType == 0) return;
    if (!m_air) m_air = std::make_unique<DISTRHO::AirSuiteUniversal>();
    m_air->setSampleRate((double)m_sampleRate);
    if (m_airType != airType) {
        m_air->setType((DISTRHO::AirSuiteUniversal::Type)airType);
        m_air->reset();
        m_airType = airType;
    }

    const DISTRHO::AirwindowsSpec spec =
        DISTRHO::AirSuiteUniversal::getSpec((DISTRHO::AirSuiteUniversal::Type)airType);
    for (int i = 0; i < 13; ++i) m_airParams[i] = 0.0f;
    for (int i = 0; i < spec.count && i < 13; ++i) m_airParams[i] = spec.defaults[i];

    const float cutoffNorm = std::clamp(
        std::log(std::clamp(m_cutoffHz, 20.0f, 20000.0f) / 20.0f) / std::log(1000.0f), 0.0f, 1.0f);
    const float resNorm = std::clamp((m_resonance - 0.1f) / 9.9f, 0.0f, 1.0f);
    const float gainNorm = std::clamp((m_gainDb + 24.0f) / 48.0f, 0.0f, 1.0f);

    // The BIQUAD family's published spec order differs from the effect
    // implementations (A=type, B=freq, C=Q, D/E=wet). Map positionally.
    const bool biquadGroup = (m_model >= AirwindowsFirst + 11 &&
                              m_model < AirwindowsFirst + 19);
    if ((biquadGroup || airType == (int)DISTRHO::AirSuiteUniversal::FILTER_BIQUADDOUBLE) &&
        spec.count >= 4) {
        static const int bqType[4] = {0, 2, 1, 3};
        m_airParams[0] = (float)(bqType[std::clamp(m_type, 0, 3)] + 1) * 0.25f;
        m_airParams[1] = cutoffNorm;
        m_airParams[2] = resNorm;
        if (spec.count >= 5) {
            m_airParams[3] = 0.5f;
            m_airParams[4] = 1.0f;
        } else {
            m_airParams[3] = 1.0f;
        }
        return;
    }

    bool freqSet = false, resSet = false, gainSet = false;
    for (int i = 0; i < spec.count && i < 13; ++i) {
        const char* name = spec.names[i];
        if (ciHas(name, "dry/wet") || ciHas(name, "drywet")) {
            m_airParams[i] = 1.0f;
        } else if (ciHas(name, "input")) {
            // Z filters apply A*10 then square twice; 0.1 is the original
            // constructor default and keeps the band at a sane level.
            m_airParams[i] = 0.1f;
        } else if (ciHas(name, "type") && spec.optionLabels[i][0] != nullptr) {
            // BIQUAD Type: 0=LP, 1=HP, 2=BP, 3=Notch in Airwindows order.
            static const int bqType[4] = {0, 2, 1, 3};
            m_airParams[i] = (float)(bqType[std::clamp(m_type, 0, 3)] + 1) * 0.25f;
        } else if (ciHas(name, "poles")) {
            // In the Z filters "Poles" is the progressive wet control; 0 is a
            // hard bypass. Leave it fully wet so the EQ band actually filters.
            m_airParams[i] = 1.0f;
        } else if (ciHas(name, "freq") || ciHas(name, "cutoff") || ciHas(name, "tone")) {
            m_airParams[i] = cutoffNorm;
            freqSet = true;
        } else if (ciHas(name, "nuke")) {
            // X-series "Nuke" is the saturation/nonlinearity amount, not Q.
            m_airParams[i] = std::clamp(m_saturation, 0.0f, 1.0f);
        } else if (ciHas(name, "reson") || ciHas(name, "resedge") ||
                   ciHas(name, "q")) {
            m_airParams[i] = resNorm;
            resSet = true;
        } else if (ciHas(name, "gain") || ciHas(name, "output") || ciHas(name, "intensity")) {
            m_airParams[i] = gainNorm;
            gainSet = true;
        }
    }
    // Positional fallback for algorithms whose params are named by frequency
    // bands ("22K", "15K", ...): first control = cutoff, second = resonance,
    // third = gain.
    if (!freqSet && spec.count > 0) m_airParams[0] = cutoffNorm;
    if (!resSet && spec.count > 1) m_airParams[1] = resNorm;
    if (!gainSet && spec.count > 2) m_airParams[2] = gainNorm;
}

float UnifiedFilter::processSstModel(float x, ChannelState& st) {
    switch (m_model) {
        case SstKorg35: return processKorg35(x, st);
        case SstDiodeLadder: return processDiodeLadder(x, st);
        case SstOBXd: return processOBXd(x, st);
        case SstTriPole: return processTriPole(x, st);
        case SstCytomicTilt: return processCytomicTilt(x, st);
        default: return x;
    }
}

float UnifiedFilter::processSVF(int /*ch*/, float x, ChannelState& st) {
    float out = x;
    for (int stage = 0; stage < m_stages; ++stage) {
        float hp = (out - (2.0f * m_R + m_g) * st.s1[stage] - st.s2[stage]) * m_h;
        float bp = m_g * hp + st.s1[stage];
        float lp = m_g * bp + st.s2[stage];
        st.s1[stage] = m_g * hp + bp;
        st.s2[stage] = m_g * bp + lp;
        if (m_type == 0) out = lp;
        else if (m_type == 1) out = bp;
        else if (m_type == 2) out = hp;
        else out = lp + hp;
    }
    return out;
}

float UnifiedFilter::processLadder(int /*ch*/, float x, ChannelState& st) {
    // Proper 4-pole ladder: four one-pole TPT stages with a GLOBAL feedback
    // from the previous last-stage output. The old implementation fed each
    // 2-pole cell's own state back locally; as Q rose past the midpoint the
    // measured output fell instead of resonating (Moog/CEM/SSM/Jupiter all
    // showed Q 10 below Q 5). Global feedback makes the high-Q end approach
    // self-oscillation, bounded by the tanh input saturation.
    const float gOverOnePlusG = m_ladderG;
    const float slopeMax = ladderKMax(m_slope);
    const float fb = jmap(m_resonance, 0.1f, 10.0f, 0.0f, slopeMax);
    const int poles = 4;
    const float x0 = x;
    float u = x0 - fb * st.zdfState[poles - 1][1];
    if (!std::isfinite(u)) u = 0.0f;
    float prevLp = 0.0f;
    for (int i = 0; i < poles; ++i) {
        const float nl = std::tanh(u);
        const float v = (nl - st.zdfState[i][0]) * gOverOnePlusG;
        const float lp = v + st.zdfState[i][0];
        st.zdfState[i][0] = lp + v;
        prevLp = u;
        u = lp;
    }
    st.zdfState[poles - 1][1] = u; // last output, one-sample loop memory
    const float comp = 1.0f + fb * 0.22f;
    const float y = u * comp;
    if (m_type == 0) return y;                        // LP
    if (m_type == 1) return 2.0f * (prevLp - u);      // BP (last stage difference)
    if (m_type == 2) return (x0 - u) * comp;          // HP
    return (x0 - u) * comp;                           // Notch-ish (input minus LP4)
}

float UnifiedFilter::processTB303(int /*ch*/, float x, ChannelState& st) {
    // Four one-pole TPT stages with GLOBAL feedback from the last stage. The
    // previous revision fed each stage's own 2-pole state back locally and
    // attenuated the output as Q rose (Q 10 measured -50 dB versus Q 0.1), so
    // the model could never resonate. This is the classic ladder topology:
    // high Q increases the loop gain toward self-oscillation, with a tanh
    // input saturation bounding the scream instead of a gain loss.
    const float gOverOnePlusG = m_ladderG;
    float k_val = jmap(m_resonance, 0.1f, 10.0f, 0.0f, ladderKMax(m_slope));
    k_val = std::clamp(k_val, 0.0f, 4.0f);

    float u = x - k_val * st.zdfState[3][1];
    u = std::tanh(u);
    for (int i = 0; i < 4; ++i) {
        const float v = (u - st.zdfState[i][0]) * gOverOnePlusG;
        const float lp = v + st.zdfState[i][0];
        st.zdfState[i][0] = lp + v;
        u = lp;
    }
    // Last-stage output is the one sample of memory the global loop needs.
    st.zdfState[3][1] = u;

    const float f = m_cutoffHz / 8.0f;
    const float hpfCorr = f / std::sqrt(1.0f + f * f);
    // Keep the passband level up as the resonant peak grows; bounded by the
    // tanh so a self-oscillating patch screams instead of clipping to silence.
    const float comp = 1.0f + k_val * 0.22f;
    return u * comp * hpfCorr;
}

float UnifiedFilter::processSEM(int /*ch*/, float x, ChannelState& st) {
    float out = x;
    const float inv1g2 = 1.0f / (1.0f + m_g * m_g);
    for (int stage = 0; stage < m_stages; ++stage) {
        const float bp_sat = std::tanh(st.s1[stage]);
        float hp = (out - 2.0f * m_R * bp_sat - m_g * st.s1[stage] - st.s2[stage]) * inv1g2;
        float bp = m_g * hp + st.s1[stage];
        float lp = m_g * bp + st.s2[stage];
        st.s1[stage] = m_g * hp + bp;
        st.s2[stage] = m_g * bp + lp;
        if (m_type == 0) out = lp;
        else if (m_type == 1) out = bp;
        else if (m_type == 2) out = hp;
        else out = lp + hp;
    }
    return out;
}

float UnifiedFilter::processBitcrush(int ch, float x, ChannelState& st) {
    return processSVF(ch, x, st);
}

float UnifiedFilter::processFormant(int /*ch*/, float x, ChannelState& st) {
    float v = jlimit(0.0f, 4.0f, jmap(std::log10(m_cutoffHz), std::log10(20.0f), std::log10(20000.0f), 0.0f, 4.0f));
    int idx = (int)v; float frac = v - idx;
    if (idx >= 4) { idx = 3; frac = 1.0f; }
    float f1[5] = {730.f,270.f,300.f,530.f,400.f};
    float f2[5] = {1090.f,2290.f,870.f,1840.f,840.f};
    float f3[5] = {2440.f,3010.f,2240.f,2480.f,2800.f};
    float f_arr[3] = {
        f1[idx]+(f1[idx+1]-f1[idx])*frac, f2[idx]+(f2[idx+1]-f2[idx])*frac, f3[idx]+(f3[idx+1]-f3[idx])*frac
    };
    float sum = 0.0f;
    float gains[3] = {1.0f, 0.5f, 0.2f};
    float formQ = 2.0f + m_resonance * 3.0f;
    float formR = 1.0f / (2.0f * formQ);
    for (int f = 0; f < 3; ++f) {
        float wd_f = DSPUtils::kPi * jlimit(20.0f, 20000.0f, f_arr[f]) / m_sampleRate;
        float g_f = std::tan(wd_f);
        float h_f = 1.0f / (1.0f + 2.0f * formR * g_f + g_f * g_f);
        float hp = (x - (2.0f * formR + g_f) * st.form_s1[f] - st.form_s2[f]) * h_f;
        float bp = g_f * hp + st.form_s1[f];
        float lp = g_f * bp + st.form_s2[f];
        st.form_s1[f] = g_f * hp + bp;
        st.form_s2[f] = g_f * bp + lp;
        sum += bp * gains[f];
    }
    return sum;
}

float UnifiedFilter::processComb(int /*ch*/, float x, ChannelState& st) {
    float delaySamples = m_sampleRate / jlimit(20.0f, 20000.0f, m_cutoffHz);
    int dInt = (int)delaySamples;
    float dFrac = delaySamples - (float)dInt;
    float eta = (1.0f - dFrac) / (1.0f + dFrac);
    float fb = jmap(m_resonance, 0.1f, 10.0f, 0.0f, 0.95f);
    if (m_type == 1 || m_type == 3) fb = -fb;
    float out = x;
    for (int stage = 0; stage < m_stages; ++stage) {
        int readIdx1 = (st.combWriteIdx[stage] - dInt) & (kCombBufSize - 1);
        int readIdx2 = (readIdx1 - 1) & (kCombBufSize - 1);
        float delayed = st.combBuffer[stage][readIdx2] + eta * (st.combBuffer[stage][readIdx1] - st.comb_ap_state[stage]);
        st.comb_ap_state[stage] = delayed;
        float combOut;
        if (m_type == 0 || m_type == 1) {
            float toBuffer = std::tanh(out + delayed * fb);
            st.combBuffer[stage][st.combWriteIdx[stage]] = toBuffer;
            combOut = toBuffer;
        } else {
            st.combBuffer[stage][st.combWriteIdx[stage]] = out;
            combOut = out + delayed * fb;
        }
        st.combWriteIdx[stage] = (st.combWriteIdx[stage] + 1) & (kCombBufSize - 1);
        out = combOut;
    }
    return out;
}

float UnifiedFilter::processMS20(int /*ch*/, float x, ChannelState& st) {
    constexpr float DC_ALPHA = 0.9995f;
    float ms_k = jmap(m_resonance, 0.1f, 10.0f, 0.0f, 2.5f);
    float out = x;
    for (int stage = 0; stage < m_stages; ++stage) {
        float fb_raw = st.sk_s2[stage] * ms_k;
        float fb_blocked = fb_raw - st.ms20_dc_x1[stage] + DC_ALPHA * st.ms20_dc_y1[stage];
        st.ms20_dc_x1[stage] = fb_raw;
        st.ms20_dc_y1[stage] = fb_blocked;
        float fb = (fb_blocked > 0.0f) ? std::tanh(fb_blocked * 1.5f) : std::tanh(fb_blocked * 0.8f);
        float v1 = (out - fb - st.sk_s1[stage]) * m_g / (1.0f + m_g);
        float y1 = v1 + st.sk_s1[stage];
        st.sk_s1[stage] = y1 + v1;
        float v2 = (y1 - st.sk_s2[stage]) * m_g / (1.0f + m_g);
        float y2 = v2 + st.sk_s2[stage];
        st.sk_s2[stage] = y2 + v2;
        float lp = y2, hp = out - y1, bp = y1 - y2;
        if (m_type == 0) out = lp;
        else if (m_type == 1) out = bp;
        else if (m_type == 2) out = hp;
        else out = lp + hp;
    }
    // The per-stage feedback subtracts the LP state at low frequencies, so each
    // stage loses ~1/(1+0.7*ms_k); cascading slopes compound the loss and made
    // high slopes inaudible. Compensate as a low shelf (full boost at DC, unity
    // above cutoff — a flat gain would overdrive HP/Notch passbands). Capped at
    // +12 dB: the linear loss model overestimates at high resonance (tanh
    // saturates), and the AGC bounds the steady-state level.
    const float compC = std::min(4.0f, std::pow(1.0f + 0.7f * ms_k, (float)m_stages));
    if (compC > 1.0f) {
        const float shelfG = m_g / (1.0f + m_g);
        const float shelfV = (out - st.ms20ShelfZ) * shelfG;
        const float shelfLp = shelfV + st.ms20ShelfZ;
        st.ms20ShelfZ = shelfLp + shelfV;
        out += (compC - 1.0f) * shelfLp;
    }
    return out;
}

float UnifiedFilter::processPhaser(int /*ch*/, float x, ChannelState& st) {
    float fb = jmap(m_resonance, 0.1f, 10.0f, 0.0f, 0.95f);
    if (m_type == 1 || m_type == 3) fb = -fb;
    float in_ap = std::tanh(x + fb * st.ap_out_prev);
    for (int stage = 0; stage < m_stages; ++stage) {
        float v = (in_ap - st.ap_s[stage]) * m_ladderG;
        float lp = v + st.ap_s[stage];
        st.ap_s[stage] = lp + v;
        in_ap = 2.0f * lp - in_ap;
    }
    st.ap_out_prev = in_ap;
    return (x + in_ap) * 0.5f;
}

float UnifiedFilter::processWavefolder(int /*ch*/, float x, ChannelState& st) {
    constexpr double ADAA_THRESHOLD = 1e-5;
    double fold_gain = (double)jmap(m_resonance, 0.1f, 10.0f, 1.0f, 10.0f);
    double per_stage_gain = std::pow(fold_gain, 1.0 / (double)m_stages);
    float out = x;
    for (int stage = 0; stage < m_stages; ++stage) {
        float hp = (out - (2.0f * m_R + m_g) * st.s1[stage] - st.s2[stage]) * m_h;
        float bp = m_g * hp + st.s1[stage];
        float lp = m_g * bp + st.s2[stage];
        st.s1[stage] = m_g * hp + bp;
        st.s2[stage] = m_g * bp + lp;
        float svf_out;
        if (m_type == 0) svf_out = lp;
        else if (m_type == 1) svf_out = bp;
        else if (m_type == 2) svf_out = hp;
        else svf_out = lp + hp;
        double x_n = (double)svf_out * per_stage_gain;
        double x_z1 = st.wf_x_z1[stage];
        double F_n = -std::cos(x_n);
        double F_z1 = st.wf_F_z1[stage];
        double dx = x_n - x_z1;
        double folded;
        if (std::abs(dx) < ADAA_THRESHOLD)
            folded = std::sin(x_z1) + 0.5 * std::cos(x_z1) * dx;
        else
            folded = (F_n - F_z1) / dx;
        st.wf_x_z1[stage] = x_n;
        st.wf_F_z1[stage] = F_n;
        out = (float)folded;
    }
    return out;
}

float UnifiedFilter::processFDNReverb(int /*ch*/, float x, ChannelState& st) {
    static constexpr float base_ms_tab[4] = {20.0f, 60.0f, 120.0f, 30.0f};
    float base_ms = base_ms_tab[std::clamp(m_slope, 0, 3)];
    float fb = jmap(m_resonance, 0.1f, 10.0f, 0.0f, 0.98f);
    static constexpr float fdnDelayTimes[4] = {1.0f, 1.313f, 1.637f, 1.911f};
    float out = 0.0f;
    for (int i = 0; i < 4; ++i) {
        int writeIdx = st.fdnWriteIdx[i];
        st.fdnBuffer[i][writeIdx] = x;
        st.fdnWriteIdx[i] = (writeIdx + 1) & (kCombBufSize - 1);
        float delaySamples = base_ms * fdnDelayTimes[i] * m_sampleRate / 1000.0f;
        int dInt = std::min((int)delaySamples, kCombBufSize - 1); // guard: never wrap past buffer end (extremely high sample rates)
        int readIdx = (writeIdx - dInt) & (kCombBufSize - 1);
        float delayed = st.fdnBuffer[i][readIdx];
        float damp_a = 0.85f;
        st.fdnLpState[i] = (1.0f - damp_a) * delayed + damp_a * st.fdnLpState[i];
        out += st.fdnLpState[i] * fb;
    }
    return x + out * 0.25f;
}

float UnifiedFilter::processKiloAllPass(int /*ch*/, float x, ChannelState& st) {
    float out = x;
    for (int stage = 0; stage < m_stages; ++stage) {
        float g_stage = m_g * (0.5f + 0.5f * ((float)stage / (float)m_stages));
        float hp = (out - (2.0f * m_R + g_stage) * st.kilo_s1[stage] - st.kilo_s2[stage]) / (1.0f + 2.0f * m_R * g_stage + g_stage * g_stage);
        float bp = g_stage * hp + st.kilo_s1[stage];
        float lp = g_stage * bp + st.kilo_s2[stage];
        st.kilo_s1[stage] = g_stage * hp + bp;
        st.kilo_s2[stage] = g_stage * bp + lp;
        out = lp - 2.0f * m_R * bp + hp;
    }
    return out;
}

float UnifiedFilter::processCEM3320(int /*ch*/, float x, ChannelState& st) {
    // 4-pole global-feedback ladder (same corrected topology as Moog/TB303).
    const float gOverOnePlusG = m_ladderG;
    const float slopeMax = ladderKMax(m_slope);
    const float fb = jmap(m_resonance, 0.1f, 10.0f, 0.0f, slopeMax);
    constexpr int poles = 4;
    float u = x - fb * st.zdfState[poles - 1][1];
    if (!std::isfinite(u)) u = 0.0f;
    float prev = 0.0f;
    for (int i = 0; i < poles; ++i) {
        const float nl = std::tanh(u);
        const float v = (nl - st.zdfState[i][0]) * gOverOnePlusG;
        const float lp = v + st.zdfState[i][0];
        st.zdfState[i][0] = lp + v;
        prev = u;
        u = lp;
    }
    st.zdfState[poles - 1][1] = u;
    float out = u;
    if (m_type == 2) out = x - 2.0f * prev + u;      // HP
    else if (m_type != 0) out = prev - u;             // BP
    return out * (1.0f + fb * 0.22f);
}

float UnifiedFilter::processSSM2040(int /*ch*/, float x, ChannelState& st) {
    const float gOverOnePlusG = m_ladderG;
    const float slopeMax = ladderKMax(m_slope);
    const float fb = jmap(m_resonance, 0.1f, 10.0f, 0.0f, slopeMax);
    constexpr int poles = 4;
    float u = x - fb * st.zdfState[poles - 1][1];
    if (!std::isfinite(u)) u = 0.0f;
    for (int i = 0; i < poles; ++i) {
        const float nl = std::tanh(u * 1.2f) / 1.2f;
        const float v = (nl - st.zdfState[i][0]) * gOverOnePlusG;
        const float lp = v + st.zdfState[i][0];
        st.zdfState[i][0] = lp + v;
        u = lp;
    }
    st.zdfState[poles - 1][1] = u;
    return u * (1.0f + fb * 0.22f);
}

float UnifiedFilter::processCS80(int /*ch*/, float x, ChannelState& st) {
    float cs80Drive = 1.0f + m_resonance * 0.2f;
    float out = x;
    for (int stage = 0; stage < m_stages; ++stage) {
        float sat = std::tanh(out * cs80Drive);
        float hp = (sat - (2.0f * m_R + m_g) * st.s1[stage] - st.s2[stage]) * m_h;
        float bp = m_g * hp + st.s1[stage];
        float lp = m_g * bp + st.s2[stage];
        st.s1[stage] = m_g * hp + bp;
        st.s2[stage] = m_g * bp + lp;
        if (m_type == 0) out = lp;
        else if (m_type == 2) out = hp;
        else out = lp + hp;
    }
    return out;
}

float UnifiedFilter::processJupiter(int /*ch*/, float x, ChannelState& st) {
    const float gOverOnePlusG = m_ladderG;
    const float slopeMax = ladderKMax(m_slope);
    const float fb = jmap(m_resonance, 0.1f, 10.0f, 0.0f, slopeMax);
    constexpr int poles = 4;
    float u = x - fb * st.zdfState[poles - 1][1];
    if (!std::isfinite(u)) u = 0.0f;
    for (int i = 0; i < poles; ++i) {
        const float nl = std::tanh(u);
        const float v = (nl - st.zdfState[i][0]) * gOverOnePlusG;
        const float lp = v + st.zdfState[i][0];
        st.zdfState[i][0] = lp + v;
        u = lp;
    }
    st.zdfState[poles - 1][1] = u;
    return u * (1.0f + fb * 0.22f);
}

float UnifiedFilter::processPhasedArray(int /*ch*/, float x, ChannelState& st) {
    auto waspClip = [](float v) -> float {
        return (v >= 0.0f)
            ? v / (1.0f + v * 3.0f)
            : v / (1.0f - v * 1.5f);
    };
    float out = x;
    for (int stage = 0; stage < m_stages; ++stage) {
        float hp = (out - (2.0f * m_R + m_g) * st.s1[stage] - st.s2[stage]) * m_h;
        float bp = m_g * hp + st.s1[stage];
        float lp = m_g * bp + st.s2[stage];
        st.s1[stage] = waspClip(m_g * hp + bp);
        st.s2[stage] = waspClip(m_g * bp + lp);
        if (m_type == 0)      out = lp;
        else if (m_type == 1) out = bp;
        else if (m_type == 2) out = hp;
        else                  out = lp + hp;
    }
    return out;
}

float UnifiedFilter::processDigitalPrecision(int /*ch*/, float x, ChannelState& st) {
    float out = x;
    for (int k = 0; k < m_stages; ++k) {
        float g_k = dpCoeffs[k].g;
        float R_k = dpCoeffs[k].R;
        float hp = (out - (2.0f * R_k + g_k) * st.dp_s1[k] - st.dp_s2[k]) / (1.0f + 2.0f * R_k * g_k + g_k * g_k);
        float bp = g_k * hp + st.dp_s1[k];
        float lp = g_k * bp + st.dp_s2[k];
        st.dp_s1[k] = g_k * hp + bp;
        st.dp_s2[k] = g_k * bp + lp;
        if (m_type == 0) out = lp;
        else if (m_type == 1) out = bp;
        else if (m_type == 2) out = hp;
        else out = lp + hp;
    }
    return out;
}

float UnifiedFilter::processVactrolLPG(int /*ch*/, float x, ChannelState& st) {
    float targetEnv = std::log10(m_cutoffHz / 20.0f) / 3.0f;
    static constexpr float attackTimeTable[3] = { 0.001f, 0.005f, 0.020f };
    float attackTime = attackTimeTable[std::clamp(m_slope, 0, 2)];
    float attackCoef = 1.0f - std::exp(-1.0f / (attackTime * m_sampleRate));
    float releaseTime = jmap(m_resonance, 0.1f, 10.0f, 0.05f, 2.0f);
    float releaseCoef = 1.0f - std::exp(-1.0f / (releaseTime * m_sampleRate));
    float coef = (targetEnv > st.lpgEnv) ? attackCoef : releaseCoef;
    st.lpgEnv = st.lpgEnv + coef * (targetEnv - st.lpgEnv);
    float lpgFc = jlimit(20.0f, 15000.0f, DSPUtils::normToHz( st.lpgEnv));
    float lpgWd = DSPUtils::kPi * lpgFc / m_sampleRate;
    float lpgG = std::tan(lpgWd);
    float lpgR = 1.0f / (2.0f * 0.707f);
    float lpgH = 1.0f / (1.0f + 2.0f * lpgR * lpgG + lpgG * lpgG);
    float out = x;
    float hp = (out - (2.0f * lpgR + lpgG) * st.s1[0] - st.s2[0]) * lpgH;
    float bp = lpgG * hp + st.s1[0];
    float lp = lpgG * bp + st.s2[0];
    st.s1[0] = lpgG * hp + bp;
    st.s2[0] = lpgG * bp + lp;
    return lp * (st.lpgEnv * st.lpgEnv + 0.001f);
}

float UnifiedFilter::processModalResonator(int /*ch*/, float x, ChannelState& st) {
    float inharmonicity = jmap(m_resonance, 0.1f, 10.0f, 1.0f, 2.5f);
    static constexpr float qBaseTable[3] = { 5.0f, 15.0f, 50.0f };
    float qBase = qBaseTable[std::clamp(m_slope, 0, 2)];
    float sum = 0.0f;
    for (int b = 0; b < kMaxModalBands; ++b) {
        float bFreq = jlimit(20.0f, m_sampleRate * 0.45f, m_cutoffHz * std::pow((float)(b + 1), inharmonicity));
        float g_b = std::tan(DSPUtils::kPi * bFreq / m_sampleRate);
        float q_b = qBase / std::sqrt((float)(b + 1));
        float R_b = 1.0f / (2.0f * q_b);
        float h_b = 1.0f / (1.0f + 2.0f * R_b * g_b + g_b * g_b);
        float hp = (x - (2.0f * R_b + g_b) * st.modal_s1[b] - st.modal_s2[b]) * h_b;
        float bp = g_b * hp + st.modal_s1[b];
        float lp = g_b * bp + st.modal_s2[b];
        st.modal_s1[b] = g_b * hp + bp;
        st.modal_s2[b] = g_b * bp + lp;
        sum += bp * (1.0f / std::sqrt((float)(b + 1)));
    }
    return sum;
}

float UnifiedFilter::processWaveguideMesh(int /*ch*/, float x, ChannelState& st) {
    float delaySamples = m_sampleRate / jlimit(20.0f, 20000.0f, m_cutoffHz);
    int dInt = (int)delaySamples;
    float dFrac = delaySamples - (float)dInt;
    float eta = (1.0f - dFrac) / (1.0f + dFrac);
    float fb = jmap(m_resonance, 0.1f, 10.0f, 0.0f, 0.99f);
    float out = x;
    for (int stage = 0; stage < m_stages; ++stage) {
        int r1 = (st.combWriteIdx[stage] - dInt) & (kCombBufSize - 1);
        int r2 = (r1 - 1) & (kCombBufSize - 1);
        float wgDelayed = st.combBuffer[stage][r2] + eta * (st.combBuffer[stage][r1] - st.comb_ap_state[stage]);
        st.comb_ap_state[stage] = wgDelayed;
        float hp = (wgDelayed - (2.0f * m_R + m_g) * st.s1[stage] - st.s2[stage]) * m_h;
        float bp = m_g * hp + st.s1[stage];
        float scattered = m_g * bp + st.s2[stage];
        st.s1[stage] = m_g * hp + bp;
        st.s2[stage] = m_g * bp + scattered;
        float excitation = out + scattered * fb;
        st.combBuffer[stage][st.combWriteIdx[stage]] = std::tanh(excitation);
        st.combWriteIdx[stage] = (st.combWriteIdx[stage] + 1) & (kCombBufSize - 1);
        if (m_type == 0) out = scattered;
        else out = out * 0.5f + scattered * 0.5f;
    }
    return out;
}

float UnifiedFilter::processBodeShifter(int /*ch*/, float x, ChannelState& st) {
    float logMin = std::log10(20.0f);
    float logMax = std::log10(20000.0f);
    float logCtr = (logMin + logMax) * 0.5f;
    float logFc = std::log10(jlimit(20.0f, 20000.0f, m_cutoffHz));
    float norm = (logFc < logCtr) ? (logFc - logCtr) / (logCtr - logMin) : (logFc - logCtr) / (logMax - logCtr);
    float shiftHz = norm * 1000.0f;
    static constexpr float hilbertA[4] = {0.4794008656f, 0.8762184935f, 0.9765975895f, 0.9974992559f};
    static constexpr float hilbertB[4] = {0.1617584983f, 0.7330289323f, 0.9453497003f, 0.9905991567f};
    float sumA = x;
    for (int i = 0; i < 4; ++i) {
        float y = sumA * hilbertA[i] - st.hilbertStateA[i];
        st.hilbertStateA[i] = st.hilbertStateA[i] * hilbertA[i] + sumA * hilbertA[i];
        sumA = y;
    }
    float sumB = x;
    for (int i = 0; i < 4; ++i) {
        float y = sumB * hilbertB[i] - st.hilbertStateB[i];
        st.hilbertStateB[i] = st.hilbertStateB[i] * hilbertB[i] + sumB * hilbertB[i];
        sumB = y;
    }
    st.bodePhase += 2.0f * DSPUtils::kPi * shiftHz / m_sampleRate;
    if (st.bodePhase > 2.0f * DSPUtils::kPi) st.bodePhase -= 2.0f * DSPUtils::kPi;
    float cosPhase = std::cos(st.bodePhase);
    float sinPhase = std::sin(st.bodePhase);
    return sumA * cosPhase - sumB * sinPhase;
}

float UnifiedFilter::processZPlaneMorph(int /*ch*/, float x, ChannelState& st) {
    // Real SVF cascade: each of the 7 stages sweeps its cutoff up (morph), and
    // the selected type taps LP/BP/HP/Notch per stage. (Previously LP/HP were
    // flat attenuators and BP/Notch were silent/DC garbage.)
    float out = x;
    for (int k = 0; k < 7; ++k) {
        const float g_k = m_g * (0.5f + 0.5f * ((float)k / 6.0f));
        const float R_k = m_R;
        const float den_k = 1.0f + 2.0f * R_k * g_k + g_k * g_k;
        float hp = (out - (2.0f * R_k + g_k) * st.zplane_s1[k] - st.zplane_s2[k]) / den_k;
        float bp = g_k * hp + st.zplane_s1[k];
        float lp = g_k * bp + st.zplane_s2[k];
        st.zplane_s1[k] = g_k * hp + bp;
        st.zplane_s2[k] = g_k * bp + lp;
        if (m_type == 0) out = lp;
        else if (m_type == 1) out = bp;
        else if (m_type == 2) out = hp;
        else out = lp + hp;
    }
    return out;
}

float UnifiedFilter::processPhasedArray2(int /*ch*/, float x, ChannelState& st) {
    float mixedPhase = 0.0f;
    float in_ap = x;
    float depth = jmap(m_resonance, 0.1f, 10.0f, 0.0f, 0.8f);
    for (int stage = 0; stage < m_stages; ++stage) {
        float fb = depth;
        if (stage % 2 != 0) fb = -fb;
        float v = (in_ap - st.pa_s[stage]) * m_ladderG;
        float lp = v + st.pa_s[stage];
        st.pa_s[stage] = lp + v;
        float stage_out = 2.0f * lp - in_ap;
        mixedPhase += stage_out * ((float)(stage + 1) / (float)m_stages);
        in_ap = std::tanh(stage_out * 1.1f);
    }
    if (m_type == 0)
        return (x + mixedPhase) * 0.5f;
    else
        return x - mixedPhase;
}

float UnifiedFilter::processNyquistAntiAlias(int /*ch*/, float x, ChannelState& st) {
    float out = x;
    for (int stage = 0; stage < m_stages; ++stage) {
        float hp = (out - (2.0f * m_R + m_g) * st.aa_s1[stage] - st.aa_s2[stage]) * m_h;
        float bp = m_g * hp + st.aa_s1[stage];
        float lp = m_g * bp + st.aa_s2[stage];
        st.aa_s1[stage] = m_g * hp + bp;
        st.aa_s2[stage] = m_g * bp + lp;
        out = lp;
    }
    return out;
}

float UnifiedFilter::processAnalogEmulation(int ch, float x, ChannelState& st) {
    switch (m_model) {
        case 5:  return processFormant(ch, x, st);
        case 21: return processVactrolLPG(ch, x, st);
        case 22: return processModalResonator(ch, x, st);
        case 27: return processNyquistAntiAlias(ch, x, st);
        default: return x;
    }
}

float UnifiedFilter::processSpectral(int ch, float x, ChannelState& st) {
    switch (m_model) {
        case 8:  return processPhaser(ch, x, st);
        case 11: return processKiloAllPass(ch, x, st);
        case 24: return processBodeShifter(ch, x, st);
        case 25: return processZPlaneMorph(ch, x, st);
        case 26: return processPhasedArray2(ch, x, st);
        default: return x;
    }
}

const char* UnifiedFilter::modelName(int m) {
    if (isAirwindowsModel(m)) {
        const auto& ident = DISTRHO::AirSuiteUniversal::getRuntimeIdentity(
            (DISTRHO::AirSuiteUniversal::Type)airwindowsTypeForModel(m));
        return (ident.displayName != nullptr && ident.displayName[0]) ? ident.displayName
                                                                     : "Airwindows";
    }
    static const char* names[Count] = {
        "Clean SVF", "Moog Ladder", "TB-303 Diode", "SEM", "Bitcrush/SRR", "Formant (Vowel)",
        "Comb Filter", "MS-20 Screaming", "All-Pass Phaser", "Wavefolder", "FDN Reverb",
        "Kilo All-Pass", "CEM3320 (Prophet)", "SSM 2040 (OB)", "CS-80 (Yamaha)", "Jupiter (Roland)",
        "Phased Array", "Butterworth", "Chebyshev", "Bessel", "Elliptic",
        "Vactrol LPG", "Modal Resonator", "Waveguide Mesh", "Bode Freq Shifter",
        "Z-Plane 2D Morph", "Phased Array 2", "Nyquist Anti-alias", "None",
        "Korg 35 Ladder", "Diode Ladder (Odin)", "OB-Xd Filter", "TriPole Filter", "Cytomic Tilt"
    };
    return (m >= 0 && m < Count) ? names[m] : "Unknown";
}

const char* UnifiedFilter::typeName(int t) {
    static const char* names[] = {"Low-Pass", "Band-Pass", "High-Pass", "Notch", "Bell", "Low Shelf", "High Shelf", "All-Pass"};
    return (t >= 0 && t <= 7) ? names[t] : "Unknown";
}

const char* UnifiedFilter::filterTypeNameForModel(int m, int t) {
    if (t < 0 || t > 7) return "Unknown";
    // Models with non-standard type mappings
    switch (m) {
        case 6: { // Comb: FB polarity + saturation modes
            static const char* combTypes[] = {"Sat+FB", "Sat-FB", "Lin+FB", "Lin-FB"};
            return (t < 4) ? combTypes[t] : "Unknown";
        }
        case 10: // FDN Reverb: single character
            return "Reverb";
        case 11: // Kilo All-Pass: single character
            return "AllPass";
        case 22: // Modal resonator: single character
            return "Resonance";
        case 24: // Bode: single character
            return "Shift";
        case 8: // Phaser: feedback polarity
            return (t == 0) ? "+FB" : (t == 1) ? "-FB" : typeName(t);
        case 14: // CS-80: LP/HP only (no BP/Notch)
            return (t == 0) ? "Low-Pass" : (t == 2) ? "High-Pass" : typeName(t);
        case 23: // Waveguide: Wet/Mix
            return (t == 0) ? "Wet" : (t == 1) ? "Mix" : typeName(t);
        case 26: // Phased Array: Blend/Wet
            return (t == 0) ? "Blend" : "Wet";
        case 29: // Korg 35
            return (t == 0) ? "Low-Pass" : (t == 2) ? "High-Pass" : typeName(t);
        case 32: // TriPole
            return (t == 0) ? "Low-Pass" : (t == 1) ? "Band-Pass" : (t == 2) ? "High-Pass" : typeName(t);
        case 33: // Cytomic Tilt: one shape, gain sets the tilt
            return "Tilt";
        default:
            return typeName(t);
    }
}

const char* UnifiedFilter::slopeName(int s) {
    static const char* names[] = {"12 dB/oct", "24 dB/oct", "48 dB/oct", "96 dB/oct"};
    return (s >= 0 && s <= 3) ? names[s] : "Unknown";
}

float UnifiedFilter::getMagnitudeForFrequency(float frequency) const {
    // NoneModel: flat response (0 dB)
    if (m_model == NoneModel) return 1.0f;
    float fc = jlimit(20.0f, 20000.0f, m_cutoffHz);
    float res = jlimit(0.1f, 10.0f, m_resonance);
    float mag = 1.0f;
    const int m = m_model;

    // Bilinear-warped frequency ratio: tan(pi*f/sr)/tan(pi*fc/sr). The DSP cores
    // are bilinear (g = tan(pi*fc/sr)), so this is the true ratio the digital
    // filters see; the unwarped f/fc understates rolloff near Nyquist.
    const float fcTan = std::tan(DSPUtils::kPi * fc / m_sampleRate);
    const float warpF = (fcTan > 1.0e-6f)
        ? std::max(0.0f, std::min(std::tan(DSPUtils::kPi * frequency / m_sampleRate) / fcTan, 1.0e4f))
        : 1000.0f;
    const float warpF2 = warpF * warpF;

    // Airwindows models: graph approximation from the mapped cutoff/Q/type.
    if (isAirwindowsModel(m_model)) {
        float d = 1.0f / jlimit(0.1f, 10.0f, jlimit(0.1f, 10.0f, m_resonance));
        const float den = std::sqrt(std::pow(1.0f - warpF2, 2.0f) + std::pow(warpF * d, 2.0f));
        float mm = 1.0f / den;
        if (m_type == 1) mm *= warpF;
        else if (m_type == 2) mm *= warpF2;
        else if (m_type == 3) mm *= std::fabs(1.0f - warpF2);
        return mm;
    }

    // EQ types (Bell, LowShelf, HighShelf, AllPass) use biquad magnitude
    if (m_type >= 4) {
        const float w0 = 2.0f * DSPUtils::kPi * fc / m_sampleRate;
        const float cosW0 = std::cos(w0);
        const float sinW0 = std::sin(w0);
        const float A = std::pow(10.0f, m_gainDb / 40.0f);
        const float alpha = sinW0 / (2.0f * std::max(res, 1.0e-6f));
        const float freqW = 2.0f * DSPUtils::kPi * frequency / m_sampleRate;
        const std::complex<float> z = std::exp(std::complex<float>(0, -freqW));
        const std::complex<float> z2 = z * z;
        std::complex<float> num(1, 0), den(1, 0);
        switch (m_type) {
            case 4:
                num = std::complex<float>(1.0f + alpha * A, 0) + std::complex<float>(-2.0f * cosW0, 0) * z + std::complex<float>(1.0f - alpha * A, 0) * z2;
                den = std::complex<float>(1.0f + alpha / A, 0) + std::complex<float>(-2.0f * cosW0, 0) * z + std::complex<float>(1.0f - alpha / A, 0) * z2;
                break;
            case 5: {
                float sq = 2.0f * std::sqrt(A) * alpha;
                num = std::complex<float>(A * ((A + 1) - (A - 1) * cosW0 + sq), 0)
                    + std::complex<float>(2.0f * A * ((A - 1) - (A + 1) * cosW0), 0) * z
                    + std::complex<float>(A * ((A + 1) - (A - 1) * cosW0 - sq), 0) * z2;
                den = std::complex<float>((A + 1) + (A - 1) * cosW0 + sq, 0)
                    + std::complex<float>(-2.0f * ((A - 1) + (A + 1) * cosW0), 0) * z
                    + std::complex<float>((A + 1) + (A - 1) * cosW0 - sq, 0) * z2;
                break; }
            case 6: {
                float sq = 2.0f * std::sqrt(A) * alpha;
                num = std::complex<float>(A * ((A + 1) + (A - 1) * cosW0 + sq), 0)
                    + std::complex<float>(-2.0f * A * ((A - 1) + (A + 1) * cosW0), 0) * z
                    + std::complex<float>(A * ((A + 1) + (A - 1) * cosW0 - sq), 0) * z2;
                den = std::complex<float>((A + 1) - (A - 1) * cosW0 + sq, 0)
                    + std::complex<float>(2.0f * ((A - 1) - (A + 1) * cosW0), 0) * z
                    + std::complex<float>((A + 1) - (A - 1) * cosW0 - sq, 0) * z2;
                break; }
            case 7:
                num = std::complex<float>(1.0f - alpha, 0) + std::complex<float>(-2.0f * cosW0, 0) * z + std::complex<float>(1.0f + alpha, 0) * z2;
                den = std::complex<float>(1.0f + alpha, 0) + std::complex<float>(-2.0f * cosW0, 0) * z + std::complex<float>(1.0f - alpha, 0) * z2;
                break;
        }
        float magNum = std::abs(num);
        float magDen = std::abs(den);
        return (magDen > 1e-20f) ? magNum / magDen : 1.0f;
    }

    // Cytomic Tilt: low shelf x high shelf around the pivot.
    if (m == 33) {
        const float blend = warpF2 / (1.0f + warpF2);
        return std::pow(10.0f, (m_gainDb / 40.0f) * (2.0f * blend - 1.0f));
    }

    // Ported sst models (29..32): SVF-style slope/tap estimate for the graph.
    if (m >= 29 && m <= 32) {
        float d = 1.0f / jlimit(0.1f, 10.0f, res);
        const float den = std::sqrt(std::pow(1.0f - warpF2, 2.0f) + std::pow(warpF * d, 2.0f));
        float mm = 1.0f / den;
        int order = (m == 29) ? 2 : (m == 30) ? 4 : (m == 31) ? 2 : 3;
        if (m == 30) order = 1 + std::clamp(m_slope, 0, 3);
        if (m == 31 && m_slope > 0) order = 4;
        if (m_type == 1) mm *= warpF;
        else if (m_type == 2) mm *= warpF2;
        else if (m_type == 3) mm *= std::fabs(1.0f - warpF2);
        return std::pow(mm, (float)order);
    }

    // SVF-based models: 0=CleanSVF, 3=SEM, 4=Bitcrush, 14=CS80, 16=Wasp
    if (m == 0 || m == 3 || m == 4 || m == 14 || m == 16) {
        float adjRes = res * ((m != 14) ? m_scalerSVF : 1.0f);
        float d = 1.0f / jlimit(0.1f, 10.0f, adjRes);
        float den = std::sqrt(std::pow(1.0f - warpF2, 2.0f) + std::pow(warpF * d, 2.0f));
        mag = 1.0f / den;
        if (m_type == 1) mag *= warpF; else if (m_type == 2) mag *= warpF2;
        else if (m_type == 3) mag *= std::abs(1.0f - warpF2);
        return std::pow(mag, (float)m_stages);
    }

    // Ladder-family models (1=Moog, 2=TB-303, 12=CEM3320, 13=SSM2040,
    // 15=Jupiter): match the corrected global-feedback DSP. The old fitted
    // graph deliberately showed NO resonant peak (it was fitted to the broken
    // local-feedback cores), which made the UI look like Q ran backwards even
    // after the audio was fixed. H(s) = 1 / ((1 + jw)^4 + k), with the same k
    // mapping and output compensation as the per-sample ladder.
    if (m == 1 || m == 2 || m == 12 || m == 13 || m == 15) {
        const float kMax = ladderKMax(m_slope);
        const float k = jmap(res, 0.1f, 10.0f, 0.0f, kMax);
        const float w = warpF;
        const std::complex<float> s1(1.0f, w);
        const std::complex<float> s2 = s1 * s1;
        const std::complex<float> s4 = s2 * s2;
        const std::complex<float> den = s4 + std::complex<float>(k, 0.0f);
        float mag = 1.0f / std::max(1.0e-6f, std::abs(den));
        if (m_type == 1) mag *= w;
        else if (m_type == 2) mag *= w * w;
        else if (m_type == 3) mag *= std::fabs(1.0f - w * w);
        mag *= 1.0f + k * 0.22f;
        if (m == 2) {
            const float f = fc / 8.0f;
            mag *= f / std::sqrt(1.0f + f * f); // TB-303 hpf correction
        }
        return mag;
    }

    // MS-20 (model 7): fitted to measured DSP (DC-blocked feedback gives an LF
    // droop and weak-null HP/Notch taps). Staging: each slope doubles the stage
    // count; the DSP compensates the per-stage loss with a low shelf (full boost
    // at DC, unity above cutoff), leaving an ~0.867^(N-1) cascade interaction
    // residual that is mirrored here.
    if (m == 7) {
        const float ms_k = jmap(res, 0.1f, 10.0f, 0.0f, 2.5f);
        const float L = 0.96f / std::pow(1.0f + warpF2, 0.95f);
        const float compC = std::min(4.0f, std::pow(1.0f + 0.7f * ms_k, (float)m_stages));
        const float comp = 1.0f + (compC - 1.0f) / std::sqrt(1.0f + warpF2); // low shelf
        const float interact = std::pow(0.867f, (float)(m_stages - 1));
        if (m_type == 1) return std::pow(1.15f * warpF * L, (float)m_stages) * comp * interact;
        const float ex = (m_stages == 1) ? 1.0f : 0.65f * (float)m_stages;
        if (m_type == 2) return std::pow(std::fabs(1.0f - 0.71f * L), ex) * comp * interact;
        if (m_type == 3) return std::pow(std::fabs(1.0f - 1.55f * L + 1.75f * L * L), ex) * comp * interact;
        return std::pow(L, (float)m_stages) * comp * interact;
    }

    // Phaser (model 8)
    if (m == 8) {
        float phi = -2.0f * std::atan(frequency / fc);
        float fb = jmap(res, 0.1f, 10.0f, 0.0f, 0.95f);
        if (m_type == 1 || m_type == 3) fb = -fb;
        float c_ap = std::cos((float)m_stages * phi);
        float s_ap = std::sin((float)m_stages * phi);
        float den2 = (1.0f - fb * c_ap) * (1.0f - fb * c_ap) + (fb * s_ap) * (fb * s_ap);
        float rya = (c_ap - fb) / den2;
        float iya = s_ap / den2;
        float ro = 0.5f * (1.0f + rya);
        float io = 0.5f * iya;
        return std::sqrt(ro * ro + io * io);
    }

    // Comb Filter (model 6)
    if (m == 6) {
        float delaySamples = m_sampleRate / jlimit(20.0f, 20000.0f, fc);
        float fb = jmap(res, 0.1f, 10.0f, 0.0f, 0.95f);
        if (m_type == 1 || m_type == 3) fb = -fb;
        float wD = 2.0f * DSPUtils::kPi * frequency * (delaySamples / m_sampleRate);
        float mm = (m_type == 0 || m_type == 1)
            ? (1.0f / std::sqrt(1.0f + fb * fb - 2.0f * fb * std::cos(wD)))
            : std::sqrt(1.0f + fb * fb + 2.0f * fb * std::cos(wD));
        return std::pow(mm, (float)m_stages);
    }

    // KiloAllPass (model 11): allpass is magnitude-flat
    if (m == 11) {
        return 1.0f;
    }

    // FDN Reverb (model 10): dry + damped feedback; comb peaks grow with feedback
    if (m == 10) {
        static constexpr float base_ms_tab[4] = { 20.0f, 60.0f, 120.0f, 30.0f };
        const float base_ms = base_ms_tab[std::clamp(m_slope, 0, 3)];
        const float fb_g = jmap(res, 0.1f, 10.0f, 0.0f, 0.98f);
        const float baseD = base_ms / 1000.0f;
        float fdn_total = 0.0f;
        for (int i = 0; i < 4; ++i) {
            float wD = 2.0f * DSPUtils::kPi * frequency * (baseD * (1.0f + i * 0.313f));
            fdn_total += 1.0f / std::sqrt(std::max(0.001f, 1.0f + fb_g*fb_g - 2.0f*fb_g*std::cos(wD)));
        }
        return 0.6f + (fdn_total * 0.25f) * 0.4f;
    }

    // Vactrol LPG (model 21) — steady-state approximation using cutoff as envelope position
    if (m == 21) {
        float envPos = std::log10(fc / 20.0f) / 3.0f;
        float d = 1.0f / 0.707f;
        float w_lpg = warpF;
        mag = (1.0f / std::sqrt(std::pow(1.0f - w_lpg * w_lpg, 2.0f) + std::pow(w_lpg * d, 2.0f))) * (envPos * envPos + 0.001f);
        return mag;
    }

    // Formant (model 5): sum of 3 BP resonators at formant frequencies (mirrors DSP)
    if (m == 5) {
        float v = jlimit(0.0f, 4.0f, jmap(std::log10(fc), std::log10(20.0f), std::log10(20000.0f), 0.0f, 4.0f));
        int idx = (int)v; float frac = v - idx;
        if (idx >= 4) { idx = 3; frac = 1.0f; }
        static const float f1t[5] = {730.f, 270.f, 300.f, 530.f, 400.f};
        static const float f2t[5] = {1090.f, 2290.f, 870.f, 1840.f, 840.f};
        static const float f3t[5] = {2440.f, 3010.f, 2240.f, 2480.f, 2800.f};
        const float fa[3] = {
            f1t[idx] + (f1t[idx+1] - f1t[idx]) * frac,
            f2t[idx] + (f2t[idx+1] - f2t[idx]) * frac,
            f3t[idx] + (f3t[idx+1] - f3t[idx]) * frac
        };
        const float formQ = 2.0f + res * 3.0f;
        const float gains[3] = {1.0f, 0.5f, 0.2f};
        // Complex sum: the resonator BPs cancel in phase between formants.
        std::complex<float> total(0.0f, 0.0f);
        for (int f = 0; f < 3; ++f) {
            const float frTan = std::tan(DSPUtils::kPi * jlimit(20.0f, 20000.0f, fa[f]) / m_sampleRate);
            const float wk = (frTan > 1.0e-6f) ? std::tan(DSPUtils::kPi * frequency / m_sampleRate) / frTan : 1.0f;
            total += std::complex<float>(0.0f, wk) / std::complex<float>(1.0f - wk * wk, wk / formQ) * gains[f];
        }
        return std::abs(total);
    }

    // Modal Resonator (model 22): complex sum of BP modes (phase-cancels between modes)
    if (m == 22) {
        float inharmonicity = jmap(res, 0.1f, 10.0f, 1.0f, 2.5f);
        static constexpr float qBaseTable[3] = { 5.0f, 15.0f, 50.0f };
        float qBase = qBaseTable[std::clamp(m_slope, 0, 2)];
        std::complex<float> total(0.0f, 0.0f);
        for (int b = 0; b < kMaxModalBands; ++b) {
            float bFreq = std::clamp(fc * std::pow((float)(b + 1), inharmonicity), 20.0f, m_sampleRate * 0.45f);
            const float bTan = std::tan(DSPUtils::kPi * bFreq / m_sampleRate);
            const float wk = (bTan > 1.0e-6f) ? std::tan(DSPUtils::kPi * frequency / m_sampleRate) / bTan : 1.0f;
            float q = qBase / std::sqrt((float)(b + 1));
            total += std::complex<float>(0.0f, wk) / std::complex<float>(1.0f - wk * wk, wk / q) * (1.0f / std::sqrt((float)(b + 1)));
        }
        return std::abs(total);
    }

    // Waveguide Mesh (model 23)
    if (m == 23) {
        // DSP output is the scattered SVF LP (Wet) or dry/delayed mix (Mix).
        // The mix carries comb notches from the delay line (fitted to DSP).
        float adjRes = res * m_scalerSVF;
        float d = 1.0f / jlimit(0.1f, 10.0f, adjRes);
        float den = std::sqrt(std::pow(1.0f - warpF2, 2.0f) + std::pow(warpF * d, 2.0f));
        float svfLP = 1.0f / den;
        if (m_type == 0) return std::pow(svfLP, (float)m_stages);
        // Mix: |x + H_svf * z^-D| / 2 with the delay line phase
        const float delaySamples = m_sampleRate / jlimit(20.0f, 20000.0f, fc);
        const float wD = 2.0f * DSPUtils::kPi * frequency * (delaySamples / m_sampleRate);
        const std::complex<float> delayed = svfLP * std::exp(std::complex<float>(0.0f, -wD));
        return std::pow(0.5f * std::abs(1.0f + delayed), (float)m_stages);
    }

    // Bode Frequency Shifter (model 24)
    if (m == 24) {
        const float logMin = std::log10(20.0f);
        const float logMax = std::log10(20000.0f);
        const float logCtr = (logMin + logMax) * 0.5f;
        const float logFc  = std::log10(jlimit(20.0f, 20000.0f, fc));
        const float norm   = (logFc < logCtr) ? (logFc - logCtr) / (logCtr - logMin) : (logFc - logCtr) / (logMax - logCtr);
        const float shiftHz = norm * 1000.0f;
        return (std::abs(frequency - (1000.0f + shiftHz)) < 80.0f) ? 6.0f : 1.0f;
    }

    // Z-Plane 2D Morph (model 25): SVF cascade with rising per-stage cutoff (mirrors processZPlaneMorph)
    if (m == 25) {
        float mag_accum = 1.0f;
        for (int k = 0; k < 7; ++k) {
            const float g_k = fcTan * (0.5f + 0.5f * ((float)k / 6.0f));
            const float w_k = (g_k > 1.0e-6f) ? std::tan(DSPUtils::kPi * frequency / m_sampleRate) / g_k : warpF;
            const float wk2 = w_k * w_k;
            const float d_k = 2.0f * m_R;
            const float den_k = std::sqrt(std::pow(1.0f - wk2, 2.0f) + std::pow(w_k * d_k, 2.0f));
            float mm = 1.0f / den_k;
            if (m_type == 1) mm *= w_k;
            else if (m_type == 2) mm *= wk2;
            else if (m_type == 3) mm *= std::fabs(1.0f - wk2);
            mag_accum *= mm;
        }
        return mag_accum;
    }

    // Phased Array 2 (model 26)
    if (m == 26) {
        const float phi_g       = -2.0f * std::atan(warpF);
        const float totalPhase  = (float)m_stages * phi_g;
        const float depth       = jmap(res, 0.1f, 10.0f, 0.0f, 0.8f);
        if (m_type == 0)
            return (1.0f + depth) * std::abs(std::cos(totalPhase * 0.5f));
        else
            return 2.0f * std::abs(std::sin(totalPhase * 0.5f)) * (1.0f + depth);
    }

    // Nyquist Anti-alias (model 27)
    if (m == 27) {
        float adjRes = res * m_scalerSVF;
        float d = 1.0f / jlimit(0.1f, 10.0f, adjRes);
        float den = std::sqrt(std::pow(1.0f - warpF2, 2.0f) + std::pow(warpF * d, 2.0f));
        return std::pow(1.0f / den, (float)m_stages);
    }

    // Digital Precision (models 17-20)
    if (m >= 17 && m <= 20) {
        float mag_total = 1.0f;
        for (int k = 0; k < m_stages; ++k) {
            float cur_R = dpCoeffs[k].R;
            float sw = warpF;
            float sw2 = sw * sw;
            float den = std::sqrt(std::pow(1.0f - sw2, 2.0f) + std::pow(sw * 2.0f * cur_R, 2.0f));
            float mm = 1.0f / den;
            if (m_type == 1) mm *= sw; else if (m_type == 2) mm *= sw2;
            else if (m_type == 3) mm *= std::abs(1.0f - sw2);
            mag_total *= mm;
        }
        return mag_total;
    }

    // Wavefolder (model 9): SVF tap into ADAA fold; add the fold-drive gain
    if (m == 9) {
        float d = 1.0f / jlimit(0.1f, 10.0f, res);
        float den = std::sqrt(std::pow(1.0f - warpF2, 2.0f) + std::pow(warpF * d, 2.0f));
        mag = 1.0f / den;
        if (m_type == 1) mag *= warpF; else if (m_type == 2) mag *= warpF2;
        else if (m_type == 3) mag *= std::abs(1.0f - warpF2);
        return std::pow(mag, (float)m_stages) * ((m_type == 0) ? (1.0f + 0.43f * res) : 1.0f);
    }

    // Generic fallback: use SVF approximation
    float d = 1.0f / jlimit(0.1f, 10.0f, res);
    float den = std::sqrt(std::pow(1.0f - warpF2, 2.0f) + std::pow(warpF * d, 2.0f));
    mag = 1.0f / den;
    if (m_type == 1) mag *= warpF; else if (m_type == 2) mag *= warpF2;
    else if (m_type == 3) mag *= std::abs(1.0f - warpF2);
    return std::pow(mag, (float)m_stages);
}
