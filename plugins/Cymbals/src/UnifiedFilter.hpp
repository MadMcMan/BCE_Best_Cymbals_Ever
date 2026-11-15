#pragma once

#include <cstdint>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <memory>

namespace DISTRHO { class AirSuiteUniversal; }

class UnifiedFilter {
public:
    enum Model : uint8_t {
        CleanSVF = 0, MoogLadder, TB303, SEM, Bitcrush, Formant,
        Comb, MS20, AllPassPhaser, Wavefolder, FDNReverb,
        KiloAllPass, CEM3320, SSM2040, CS80, Jupiter,
        PhasedArray, Butterworth, Chebyshev, Bessel, Elliptic,
        VactrolLPG, ModalResonator, WaveguideMesh, BodeShifter,
        ZPlaneMorph, PhasedArray2,         NyquistAntiAlias,
        NoneModel,
        // Unique filters ported from sst-filters (Surge XT). Appended AFTER
        // NoneModel so existing preset model values 0..28 keep their meaning.
        SstKorg35, SstDiodeLadder, SstOBXd, SstTriPole, SstCytomicTilt,
        // Airwindows FILTER/BIQUAD/XYZ algorithms exposed as EQ models.
        AirwindowsFirst = 34,
        AirwindowsCount = 30,
        Count = AirwindowsFirst + AirwindowsCount
    };

    enum FilterType : uint8_t { LowPass = 0, BandPass, HighPass, Notch, Bell, LowShelf, HighShelf, AllPass };
    enum Slope : uint8_t { Oct12 = 0, Oct24, Oct48, Oct96 };

    UnifiedFilter();
    ~UnifiedFilter();
    UnifiedFilter(const UnifiedFilter&) = delete;
    UnifiedFilter& operator=(const UnifiedFilter&) = delete;

    void setModel(int m);
    void setType(int t);
    void setSlope(int s);
    void setCutoffHz(float hz);
    void setResonance(float r);
    void setGainDb(float db);
    void setSaturation(float s) { m_saturation = std::clamp(s, 0.0f, 1.0f); }
    void setSampleRate(float sr);
    // Per-sample modulation path: clamps + coefficient refresh in one call.
    void setModulatedValues(float cutoffHz, float resonance, float gainDb) {
        setCutoffHz(cutoffHz);
        setResonance(resonance);
        setGainDb(gainDb);
        prepare();
    }

    void prepare();
    void reset();
    float processSampleL(float input);
    float processSampleR(float input);
    // Airwindows models are stereo processors: use this pair API for EQ bands.
    void processStereoSample(float& inOutL, float& inOutR);

    int    model()     const { return m_model; }
    int    type()      const { return m_type; }
    int    slope()     const { return m_slope; }
    float  cutoffHz()  const { return m_cutoffHz; }
    float  resonance() const { return m_resonance; }
    float  gainDb()    const { return m_gainDb; }
    float  sampleRate() const { return m_sampleRate; }

    static const char* modelName(int m);
    static const char* typeName(int t);
    static bool isAirwindowsModel(int m) {
        return m >= AirwindowsFirst && m < AirwindowsFirst + AirwindowsCount;
    }
    static int airwindowsTypeForModel(int m);
    // Returns the Type name appropriate for the given model.
    // Many models map types to model-specific modes (e.g. Comb uses types as
    // feedback polarity/saturation modes, not traditional LP/BP/HP/Notch).
    static const char* filterTypeNameForModel(int m, int t);
    static const char* slopeName(int s);

    float getMagnitudeForFrequency(float frequency) const;

    // Returns a bitmask of supported FilterType values for a given model.
    // Bit 0 = LowPass, bit 1 = BandPass, ..., bit 7 = AllPass.
    // Every model's native processing handles FilterTypes 0-3 (LP/BP/HP/Notch).
    // Types 4-7 (Bell/LowShelf/HighShelf/AllPass) use a model-independent biquad
    // that bypasses the model's character — they are deliberately excluded here.
    // Matches QuadMorphFilter's ModelCapabilities.h.
    // EQ types (Bell/LowShelf/HighShelf/AllPass) use a model-independent biquad
    // that works for ALL models. Only CleanSVF exposes them by default since its
    // linear SVF core is the best match for biquad-based EQ shaping.
    static constexpr uint32_t EQ = (1u << 4) | (1u << 5) | (1u << 6) | (1u << 7);
    static uint32_t supportedTypesMask(int model);

    static constexpr int kMaxStages = 8;
    static constexpr int kCombBufSize = 32768; // covers 229 ms max FDN tap up to ~143 kHz (previously aliased >= 88.2 kHz)
    static constexpr int kMaxModalBands = 8;

    // Returns whether the Slope control affects this model's DSP processing.
    // False for models with fixed internal structure: Formant (fixed 3-formant
    // bank), Bode (fixed Hilbert chains), Z-Plane (fixed 7-stage morph cascade).
    static bool slopeIsRelevant(int model) {
        return !(model == 5 || model == 24 || model == 25 ||
                 model == 29 || model == 31 || model == 32 || model == 33 ||
                 isAirwindowsModel(model));
    }

    struct ChannelState {
        float s1[kMaxStages] = {};
        float s2[kMaxStages] = {};
        float sk_s1[kMaxStages] = {};
        float sk_s2[kMaxStages] = {};
        float ms20_dc_x1[kMaxStages] = {};
        float ms20_dc_y1[kMaxStages] = {};
        float ms20ShelfZ = 0.0f; // MS-20 slope-compensation shelf state
        float zdfState[kMaxStages][4] = {};
        float dp_s1[kMaxStages] = {};
        float dp_s2[kMaxStages] = {};
        float ap_s[16] = {};
        float kilo_s1[16] = {};
        float kilo_s2[16] = {};
        float pa_s[16] = {};
        // sst-filters ports (one instance; these models are not cascaded).
        float k35_lz = 0.0f, k35_hz = 0.0f, k35_2z = 0.0f;
        float diode_z[4] = {}, diode_fb[3] = {};
        float obxd_s[4] = {};
        float tri_z[3] = {}, tri_x[3] = {}, tri_fb = 0.0f, tri_fb1 = 0.0f;
        float tilt_ic1Lo = 0.0f, tilt_ic2Lo = 0.0f, tilt_ic1Hi = 0.0f, tilt_ic2Hi = 0.0f;
        float aa_s1[8] = {};
        float aa_s2[8] = {};
        float srrPhase = 0.0f;
        float srrHeld = 0.0f;
        float srrBitSteps = 65536.0f;
        float form_s1[3] = {};
        float form_s2[3] = {};
        int combWriteIdx[kMaxStages] = {};
        float comb_ap_state[kMaxStages] = {};
        float combBuffer[kMaxStages][kCombBufSize] = {};
        float ap_out_prev = 0.0f;
        float lpgEnv = 0.0f;
        float modal_s1[kMaxModalBands] = {};
        float modal_s2[kMaxModalBands] = {};
        float hilbertStateA[4] = {};
        float hilbertStateB[4] = {};
        float bodePhase = 0.0f;
        float bodeOutPrev = 0.0f;
        float zplane_s1[7] = {};
        float zplane_s2[7] = {};
        float fdnBuffer[4][kCombBufSize] = {};
        int fdnWriteIdx[4] = {};
        float fdn_ap_state[4] = {};
        float fdnLpState[4] = {};
        float diodeHpfS = 0.0f;
        float diodePrevY4 = 0.0f;
        float diodeAdaaPrevInput = 0.0f;
        float diodeAdaaPrevLp[4] = {};
        float moogAdaaPrevInput[kMaxStages] = {};
        float wf_x_z1[kMaxStages] = {};
        double wf_F_z1[kMaxStages] = {};
        float rmsIn = 0.0f;
        float rmsOut = 0.0f;
        float agcGain = 1.0f;
        float ap_out_prev_val = 0.0f;
    };

    ChannelState chL, chR;

    float processSampleImpl(float x, ChannelState& st);
    // sst-filters ports (sst-filters is GPL-3.0-or-later, Surge Synth Team).
    void prepareSstModels();
    void prepareAirwindowsModel();
    float processSstModel(float x, ChannelState& st);
    float processKorg35(float x, ChannelState& st);
    float processDiodeLadder(float x, ChannelState& st);
    float processOBXd(float x, ChannelState& st);
    float processTriPole(float x, ChannelState& st);
    float processCytomicTilt(float x, ChannelState& st);

private:

    int m_model = 0;
    int m_type = 0;
    int m_slope = 0;
    float m_cutoffHz = 1000.0f;
    float m_resonance = 0.707f;
    float m_gainDb = 0.0f;
    float m_saturation = 0.0f;
    float m_sampleRate = 48000.0f;

    float m_g = 0.0f;
    float m_R = 0.0f;
    float m_h = 0.0f;
    float m_ladderG = 0.0f;
    float m_ladderRes = 0.0f;
    int m_stages = 1;
    int m_order = 2;
    float m_scalerSVF = 1.0f;
    float m_scalerMoog = 1.0f;

    // Biquad state for EQ types (Bell, LowShelf, HighShelf, AllPass)
    float m_bqA1 = 0.0f, m_bqA2 = 0.0f;
    float m_bqB0 = 0.0f, m_bqB1 = 0.0f, m_bqB2 = 0.0f;
    // sst-filters port coefficients.
    float m_k35G = 0.0f, m_k35Lb = 0.0f, m_k35Hb = 0.0f, m_k35K = 0.0f, m_k35Alpha = 1.0f;
    float m_diodeAlpha = 0.0f, m_diodeGamma = 0.0f, m_diodeG = 0.0f;
    float m_diodeG1 = 0.0f, m_diodeG2 = 0.0f, m_diodeG3 = 0.0f, m_diodeG4 = 0.0f, m_diodeKm = 0.0f;
    float m_obxdG12 = 0.0f, m_obxdR12 = 0.0f, m_obxdG24 = 0.0f, m_obxdR24 = 0.0f;
    float m_obxdRcor = 0.0f, m_obxdRcorInv = 1.0f;
    float m_triB[3] = {}, m_triA[3] = {1.0f, 1.0f, 1.0f}, m_triK = 0.0f;
    float m_tiltA1 = 1.0f, m_tiltA2 = 0.0f, m_tiltA3 = 0.0f;
    float m_tiltM0Lo = 1.0f, m_tiltM1Lo = 0.0f, m_tiltM2Lo = 0.0f;
    float m_tiltM0Hi = 1.0f, m_tiltM1Hi = 0.0f, m_tiltM2Hi = 0.0f;
    std::unique_ptr<DISTRHO::AirSuiteUniversal> m_air;
    int m_airType = -1;
    float m_airParams[13] = {};
    float m_bqStateL[2] = {};
    float m_bqStateR[2] = {};

    void updateCoefficients();
    void setSlopeInternal(int s);
    void updateBiquadCoeffs();

    float processSVF(int ch, float x, ChannelState& st);
    float processLadder(int ch, float x, ChannelState& st);
    float processTB303(int ch, float x, ChannelState& st);
    float processSEM(int ch, float x, ChannelState& st);
    float processBitcrush(int ch, float x, ChannelState& st);
    float processFormant(int ch, float x, ChannelState& st);
    float processComb(int ch, float x, ChannelState& st);
    float processMS20(int ch, float x, ChannelState& st);
    float processPhaser(int ch, float x, ChannelState& st);
    float processWavefolder(int ch, float x, ChannelState& st);
    float processFDNReverb(int ch, float x, ChannelState& st);
    float processKiloAllPass(int ch, float x, ChannelState& st);
    float processCEM3320(int ch, float x, ChannelState& st);
    float processSSM2040(int ch, float x, ChannelState& st);
    float processCS80(int ch, float x, ChannelState& st);
    float processJupiter(int ch, float x, ChannelState& st);
    float processPhasedArray(int ch, float x, ChannelState& st);
    float processDigitalPrecision(int ch, float x, ChannelState& st);
    float processVactrolLPG(int ch, float x, ChannelState& st);
    float processModalResonator(int ch, float x, ChannelState& st);
    float processWaveguideMesh(int ch, float x, ChannelState& st);
    float processBodeShifter(int ch, float x, ChannelState& st);
    float processZPlaneMorph(int ch, float x, ChannelState& st);
    float processPhasedArray2(int ch, float x, ChannelState& st);
    float processNyquistAntiAlias(int ch, float x, ChannelState& st);
    float processAnalogEmulation(int ch, float x, ChannelState& st);
    float processSpectral(int ch, float x, ChannelState& st);

    float applyAGC(int ch, float out, ChannelState& st);

    struct BiquadCoeffs { float g = 0.0f; float R = 0.0f; float h = 0.0f; float alpha = 1.0f; };
    BiquadCoeffs dpCoeffs[kMaxStages] = {};
    BiquadCoeffs zplaneCoeffs[7] = {};
};
