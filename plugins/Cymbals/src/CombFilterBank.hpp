#ifndef COMB_FILTER_BANK_HPP
#define COMB_FILTER_BANK_HPP

#include <vector>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <cstring>
#include "DSPUtils.hpp"
#include "StickSoundBank.hpp" // xorshift32Unit (header-only, no circularity)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace DSPUtils;

struct HilbertTransformer {
    float a[12];
    float zL[6] = {0}, zR[6] = {0};
    float sampleRate = 48000.0f;

    HilbertTransformer() {
        setSampleRate(48000.0f);
    }

    void setSampleRate(float sr) {
        sampleRate = sr;
        float srRatio = 48000.0f / sr;
        float baseCoeffs[6] = {0.4794008f, 0.8762184f, 0.9765975f, 0.9974992f, 0.9997499f, 0.9999750f};
        float imagCoeffs[6] = {0.1617584f, 0.7330289f, 0.9453499f, 0.9904992f, 0.9990499f, 0.9999050f};
        
        for (int i = 0; i < 6; ++i) {
            float adjust = std::pow(srRatio, 0.5f);
            a[i] = 1.0f - (1.0f - baseCoeffs[i]) * adjust;
            if (a[i] < 0.0f) a[i] = 0.0f;
            if (a[i] > 0.9999f) a[i] = 0.9999f;
        }
        for (int i = 0; i < 6; ++i) {
            float adjust = std::pow(srRatio, 0.5f);
            a[i + 6] = 1.0f - (1.0f - imagCoeffs[i]) * adjust;
            if (a[i + 6] < 0.0f) a[i + 6] = 0.0f;
            if (a[i + 6] > 0.9999f) a[i + 6] = 0.9999f;
        }
    }

    void process(float in, float& outReal, float& outImag) {
        float x = in;
        for (int i = 0; i < 6; ++i) {
            float y = a[i] * x + zL[i];
            zL[i] = x - a[i] * y;
            x = y;
        }
        outReal = x;
        x = in;
        for (int i = 6; i < 12; ++i) {
            float y = a[i] * x + zR[i - 6];
            zR[i - 6] = x - a[i] * y;
            x = y;
        }
        outImag = x;
    }

    void reset() {
        for (int i = 0; i < 6; ++i) zL[i] = zR[i] = 0.0f;
    }
};

struct FrequencyShifter {
    HilbertTransformer hilbert;
    float phase = 0.0f;

    void setSampleRate(float sr) {
        hilbert.setSampleRate(sr);
    }

    float process(float in, float shiftHz, float sr) {
        if (std::abs(shiftHz) < 0.1f) return in;
        float real, imag;
        hilbert.process(in, real, imag);
        phase += (shiftHz / sr);
        while (phase >= 1.0f) phase -= 1.0f;
        while (phase < 0.0f) phase += 1.0f;
        float cosPhi = std::cos(phase * 2.0f * (float)M_PI);
        float sinPhi = std::sin(phase * 2.0f * (float)M_PI);
        return (real * cosPhi) - (imag * sinPhi);
    }

    void reset() {
        hilbert.reset();
        phase = 0.0f;
    }
};

// One damped 2-pole modal resonator (Saike Partials `saike_modal.jsfx-inc`):
//     H(z) = winb*z^-1 / (1 + a1*z^-1 + a2*z^-2)
// `active` is false when the partial's frequency exceeds the Nyquist guard
// (f_k >= 0.48*sr) — the stage then skips it in the process loop.
struct PartialResonator {
    float winb = 0.0f, wout = 0.0f, a1 = 0.0f, a2 = 0.0f; // 2-pole coeffs
    float y = 0.0f, yh = 0.0f;                            // state: y = current, yh = previous
    bool active = false;
};

class CombFilterBank {
public:
    static constexpr int kMaxGroups = 6;   // UI/host group ceiling
    int activeGroupCount = kMaxGroups;     // main bank: 6; final bank: 1
    static constexpr int kMaxBufferSize = 8192;
    enum FilterType {
        FT_COMB = 0,    // delay-line feedback comb (existing behavior, unchanged)
        FT_ALLPASS = 1, // Biquad::ALL_PASS at tuned freq
        FT_NOTCH = 2,   // Biquad::NOTCH at tuned freq
        FT_RESO_LP = 3, // Biquad::LPF
        FT_RESO_BP = 4, // Biquad::BAND_PASS
        FT_RESO_HP = 5, // Biquad::HPF
        FT_PHASER = 6,  // 4 cascaded Biquad::ALL_PASS, LFO-modulated, dry+wet
        FT_FLANGE = 7,  // modulated fractional delay + feedback, dry+wet
        FT_GLITCH_PM = 8, // comb with audio-rate modulated fractional read (phase modulation)
        FT_BITCRUSH = 9,  // comb with sample-and-hold + 5-bit quantization in the feedback loop
        FT_POP = 10,      // comb with hard clip + threshold-triggered buffer scramble
        FT_STUTTER = 11,  // comb with asynchronous windowed buffer scrub
        FT_VOWEL = 12,    // 3-band formant cross-feedback loop (parallel BPs, serial injection)
        FT_VOCAL = 13,    // FT_VOWEL + glottal pulse injection (self-oscillating voice)
        FT_TALKBOX = 14,  // formant-gated bitwise XOR corruption
        // Saike Partials modal resonators: damped 2-pole partial bank (each
        // model is one coefficient table over the same resonator engine).
        // Listed ALPHABETICALLY — dropdown order must equal enum order, so
        // the UI lists them in the same sequence. Values 15..22 are new in
        // this release (no preset/automation compat surface).
        FT_PARTIAL_BEAM_CLAMP = 15,
        FT_PARTIAL_BEAM_OPEN = 16,
        FT_PARTIAL_BEATING = 17,
        FT_PARTIAL_MARIMBA = 18,
        FT_PARTIAL_MEMBRANE = 19,
        FT_PARTIAL_METAL = 20,
        FT_PARTIAL_PAN = 21,
        FT_PARTIAL_TUBE = 22,
        FT_COUNT = 23
    };

    // The 8 partials types are alphabetical in the FilterType enum; the
    // configurePartial model tables follow Saike's model order (0=Metal,
    // 1=Tube, 2=Beating, 3=Beam open, 4=Beam clamped, 5=Membrane,
    // 6=Marimba, 7=Pan). This maps the alphabetical enum position (index
    // filterType - FT_PARTIAL_BEAM_CLAMP) to the Saike model index.
    static constexpr int kPartialModelByType[8] = {4, 3, 2, 6, 5, 0, 7, 1};

    static constexpr bool isPartialType(int t) {
        return t >= FT_PARTIAL_BEAM_CLAMP && t <= FT_PARTIAL_TUBE;
    }
    enum SaturatorType {
        SAT_TANH       = 0,
        SAT_SOFTSIGN   = 1,
        SAT_ARCTAN     = 2,
        SAT_CUBIC      = 3,
        SAT_HARDCLIP   = 4,   // hard clip — aggressive odd harmonics
        SAT_FOLDBACK   = 5,   // wavefold — metallic, ring-mod-like
        SAT_ASYMMETRIC = 6,   // tube-style even+odd harmonics
        SAT_SINE       = 7,   // sine waveshape — smooth harmonic enrichment
    };

    // Partials models (Saike Partials). Matches the `partials` slider max 64.
    static constexpr int kMaxPartials = 64;

    // Zeros of J1 (transcribed verbatim from saike_partials.jsfx `bessel_roots`, first 64 values).
    static constexpr float kPartialBesselRoots[kMaxPartials] = {
        1.000000f,  2.917295f,  4.831885f,  6.746213f,  8.660476f,  10.574713f, 12.488940f, 14.403161f,
        16.317378f, 18.231593f, 20.145807f, 22.060020f, 23.974232f, 25.888443f, 27.802654f, 29.716865f,
        31.631075f, 33.545286f, 35.459496f, 37.373706f, 39.287916f, 41.202125f, 43.116335f, 45.030545f,
        46.944754f, 48.858964f, 50.773173f, 52.687383f, 54.601592f, 56.515802f, 58.430011f, 60.344220f,
        62.258430f, 64.172639f, 66.086848f, 68.001058f, 69.915267f, 71.829476f, 73.743685f, 75.657895f,
        77.572104f, 79.486313f, 81.400522f, 83.314731f, 85.228941f, 87.143150f, 89.057359f, 90.971568f,
        92.885777f, 94.799986f, 96.714196f, 98.628405f, 100.542614f, 102.456823f, 104.371032f, 106.285241f,
        108.199450f, 110.113660f, 112.027869f, 113.942078f, 115.856287f, 117.770496f, 119.684705f, 121.598914f
    };

    // ponytail: the strike curve is a linear-floor blend (see the process()
    // branch): fh = gain * 10^(stiffness-4.6) * sign(x) * [(1-w)|x| + w|x|^exp]
    // with w = stiffnessExp/3. The pure Saike 10^stiffness * x^stiffnessExp
    // (rectified) crushed sub-unit inputs into silence (x^2.3 at 0.3 is -25 dB,
    // at 0.1 over -50 dB) — unacceptable for a filter type fed by the dispersed
    // cymbal bus. At defaults the blend is exactly 1.0 at |x|=1, so the level
    // calibration below is preserved. kPartialExciteGain is the calibrated
    // level scale at the default stiffness (see kPartialDefaultStiffness);
    // kPartialModelGain normalizes each model to the FT_COMB reference at
    // defaults (measured +-0.11 dB).
    static constexpr float kPartialExciteGain = 4700000.0f;
    static constexpr float kPartialDefaultStiffness = 4.6f; // Saike stiffness slider default
    static constexpr float kPartialModelGain[8] = {
        1.05f, // METAL
        1.54f, // TUBE (odd partials only, half the energy)
        0.35f, // BEATING (all odd harmonics at 1.08 excitation)
        1.36f, // BEAM_OPEN
        1.07f, // BEAM_CLAMP
        2.87f, // MEMBRANE (10/k damping thins it out)
        0.78f, // MARIMBA
        0.83f, // PAN
    };
    static constexpr float kPartialOutScale  = 1.0f;

    // Verbatim Saike math (saike_modal.jsfx-inc pars/pars_offset/pars_beam/
    // pars_drum/pars_marimba/pars_pan, constants from saike_partials.jsfx
    // sliders): one resonator engine, model = Saike model table index 0..7
    // (see kPartialModelByType for the enum -> model mapping). `slot` runs
    // 0..kMaxPartials-1.
    // Returns the partial's frequency f_k (0 when inactive) so callers can
    // draw the spectrum without re-deriving the tables.
    static float configurePartial(int model, int slot, float f0,
                                  float inharmCoef, float baseDamp, float freqDampSlider,
                                  float relPos, float sr, PartialResonator& out) {
        const float fDamp = std::pow(10.0f, freqDampSlider); // Saike freq_damp slider (-6..-1)
        const float rp = relPos;                             // Saike rel_position (0.0001..0.999)
        const float irate = 1.0f / sr;
        const float normFactor = f0 / (220.0f * sr); // legacy_gain off: Adivfs*f0/220
        const float k = (float)(slot + 1);

        float fK = f0, r = 0.0f, win = 0.0f;
        if (model == 0) {                       // METAL
            fK = f0 * k * std::sqrt(1.0f + inharmCoef * (k - 1.0f) * (k - 1.0f));
            r = baseDamp + fDamp * 2.0f * (float)M_PI * fK;
            win = std::sin(k * (float)M_PI * rp);
        } else if (model == 1) {                // TUBE (odd partials only)
            const float ko = 2.0f * (float)slot + 1.0f;
            fK = f0 * ko * std::sqrt(1.0f + inharmCoef * (ko - 1.0f) * (ko - 1.0f));
            r = baseDamp + fDamp * 2.0f * (float)M_PI * fK;
            win = std::sin(ko * (float)M_PI * rp);
        } else if (model == 2) {                // BEATING (pairs detuned ~3.3 Hz)
            // Pairs share the harmonic index: slots {0,1}->k=1, {2,3}->k=2, ...
            // differing only by the offset — that shared index is what beats.
            const float kb = (float)(slot / 2 + 1);
            const float off = (slot % 2 == 1) ? 3.0f * (1.0f + 0.05f * kb * kb) * (0.1f + rp) : 0.0f;
            fK = f0 * kb * std::sqrt(1.0f + inharmCoef * (kb - 1.0f) * (kb - 1.0f)) + off;
            r = baseDamp + fDamp * 2.0f * (float)M_PI * fK * (1.0f + fK / 1000.0f);
            win = ((int)kb % 2 == 0) ? 0.08f : 1.08f;
        } else if (model == 3 || model == 4) {  // BEAM_OPEN / BEAM_CLAMP
            float ratio;
            if (model == 3) { // unclamped: k<=3 table, else k+0.5
                static const float t[3] = {1.5056187f, 2.4997527f, 3.5000107f};
                ratio = (slot < 3) ? t[slot] : k + 0.5f;
            } else {          // clamped: k<=4 table, else k-0.5
                static const float t[4] = {0.5968642f, 1.4941756f, 2.5002469f, 3.4999893f};
                ratio = (slot < 4) ? t[slot] : k - 0.5f;
            }
            fK = f0 * ratio * (1.0f + inharmCoef * k);
            const float omk = 2.0f * (float)M_PI * fK;
            r = baseDamp + (fDamp / 1000.0f) * omk * omk;
            win = std::sin(k * (float)M_PI * rp);
        } else if (model == 5) {                // MEMBRANE (bessel roots)
            fK = f0 * kPartialBesselRoots[slot] * (1.0f + inharmCoef * k * k);
            const float omk = 2.0f * (float)M_PI * fK;
            r = baseDamp + 10.0f / k + (fDamp / 1000.0f) * omk * omk;
            win = std::sin(k * (float)M_PI * rp);
        } else if (model == 6) {                // MARIMBA
            float ratio, camp;
            if (slot < 8) {
                static const float rt[8] = {1.0f, 1.911076f, 4.0f, 10.25f, 14.2511700468f, 17.745709828f, 19.0f, 24.352574f};
                static const float ca[8] = {1.0f, 0.04f + 0.2f * rp, 1.0f, 1.0f, 0.04f + 0.3f * rp, 0.04f + 0.3f * rp, 0.09f, 0.04f + 0.2f * rp};
                ratio = rt[slot]; camp = ca[slot];
            } else { ratio = 2.0f * k; camp = 0.09f; }
            fK = ratio * f0 * std::sqrt(1.0f + inharmCoef * k * k);
            const float omk = 2.0f * (float)M_PI * fK;
            r = baseDamp + (fDamp / 1000.0f) * omk * omk;
            win = camp;
        } else {                                // PAN (k=6 ratio is upstream's
            float ratio, camp;                  // 3.34/f0 beat-pair bug fix)
            if (slot < 10) {
                static const float rt[10] = {1.0f, 1.19672275218f, 0.74899885f, 1.57885f, 2.000333953889411f, 2.000333953889411f, 2.23426319696f, 3.0029642446f, 3.5708f, 3.997778188f};
                static const float ca[10] = {3.5f - 1.0f * rp, 0.05f + 0.26f * rp, 0.05f + 0.18f * rp, 0.01f + 0.03f * rp, 0.025f, 0.025f, 0.005f * rp, 0.05f, 0.003f * rp, 0.03f * rp};
                ratio = rt[slot]; camp = ca[slot];
                if (slot == 5) ratio += 3.34f / f0; // JSFX bug: `3.34 / used_partials += f_0`
            } else { ratio = 0.5f * k + 1.0f; camp = 0.0005f; }
            fK = ratio * f0 * std::sqrt(1.0f + (inharmCoef - 0.0001f) * k * k); // Saike's (inharm - 0.0001) guard
            const float omk = 2.0f * (float)M_PI * fK;
            r = baseDamp + 5.0f / k + (fDamp / 1000.0f) * omk * omk;
            win = camp * 0.33f;
        }

        if (fK >= 0.48f * sr) {
            out.active = false;
            out.winb = out.wout = out.a1 = out.a2 = 0.0f;
            out.y = out.yh = 0.0f;
            return 0.0f;
        }

        const float omegaK = fK * 2.0f * (float)M_PI * irate;
        const float gamma = std::exp(-r * irate);
        const float wb = k * (float)M_PI * 0.1f; // soundboard_factor = pi*0.1
        // Source scales win by wb and wout by 1/wb so the output needs no wb_k
        // multiply (winb = win*wb*b carries the shape, wout the 1/wb recovery).
        const float b = normFactor * gamma * std::sin(omegaK) / fK;
        out.winb = (win * wb) * b;
        out.wout = win / wb;
        out.a1 = -2.0f * gamma * std::cos(omegaK);
        out.a2 = gamma * gamma;
        out.active = true;
        return fK;
    }

    struct CombStage {
        float buffer[kMaxBufferSize];
        int bufferSize = 1000;
        int writeIndex = 0;
        float feedback = 0.0f;
        float lpfFreq = 20000.0f;
        float hpfFreq = 20.0f;
        OnePoleFilter lpfFilter;
        OnePoleFilter hpfFilter;
        int satType = SAT_TANH;
        // Per-stage filter topology (FilterType). FT_COMB keeps the original
        // delay-line feedback comb; the other modes replace or reshape it.
        int filterType = FT_COMB;
        float tunedFreq = 440.0f; // set by applyScaledFrequency; drives all mode tunings
        Biquad biquad;            // FT_ALLPASS / FT_NOTCH / FT_RESO_*
        Biquad phaserStages[4];   // FT_PHASER cascade
        Biquad formantStages[3];  // FT_VOWEL / FT_VOCAL / FT_TALKBOX bands (F1/F2/F3)
        float crossFb = 0.0f;     // formant resonance state (bounded loop back to the input node)
        float modeAmount = 0.6f;  // per-mode wet/amount (ALLPASS dry-wet, FLANGE wet tap)
        float allpassFb = 0.0f;   // FT_ALLPASS feedback loop state
        float glottalPhase = 0.0f;
        float glottalPeriod = 0.0f; // FT_VOCAL glottal oscillator (human pitch range)
        float lfoPhase = 0.0f;    // FT_PHASER / FT_FLANGE LFO, advanced per sample, wrapped at 2π
        float lfoRate = 0.25f;    // per-stage detuned in the bank ctor (0.18 + index*0.013 Hz)
        int lfoTick = -1;         // phaser coefficient-refresh trigger
        float flangeBuffer[2048]; // FT_FLANGE dedicated short delay (0-42 ms at 48 kHz)
        int flangeWriteIndex = 0;
        float pmPhase = 0.0f;     // FT_GLITCH_PM modulator phase
        float crushPhase = 0.0f;  // FT_BITCRUSH hold counter
        float crushedWet = 0.0f;  // FT_BITCRUSH held value
        float popTimer = 0.0f;    // FT_POP reset cooldown (samples)
        uint32_t rngState = 0u;   // FT_STUTTER RNG (seeded in the bank ctor)
        int stutPos = 0, stutStart = 0, stutLen = 0, stutTimer = 0; // FT_STUTTER window scrub
        float cachedBqFreq = -1.0f;      // biquad/formant coefficient cache
        float cachedBqFeedback = -1.0f;
        float cachedBqSr = -1.0f;
        float cachedBqModeA = -1.0f;     // formant spread changes the ratios
        float cachedBqModeB = -1.0f;     // glottal pitch / morph depth
        float explicitQ = -1.0f;         // <0 = AUTO from feedback; else user Q
        float cachedBqQ = -2.0f;         // cache key for explicitQ

        float effectiveQ(float base, float span) const {
            return explicitQ > 0.0f ? explicitQ
                                    : base + std::clamp(feedback, 0.0f, 0.998f) * span;
        }
        PartialResonator partials[kMaxPartials]; // FT_PARTIAL_* modal resonator bank
        float cachedPartFreq = -1.0f, cachedPartAmt = -1.0f;
        float cachedPartRate = -1.0f, cachedPartSr = -1.0f;
        float cachedPartRelPos = -1.0f, cachedPartFreqDamp = -1.0f;
        float cachedPartStiffness = -1.0f, cachedPartStiffExp = -1.0f, cachedPartCount = -1.0f;
        float modeRate = 0.5f;            // normalized 0..1 (inharmonicity for partials)
        // Per-filter-type MODE A/B (normalized 0..1, meaning depends on
        // filterType; 0.5 reproduces the old fixed behaviour).
        float modeA = 0.5f;
        float modeB = 0.5f;
        // Saike Partials model controls (slider defaults; ranges in PluginParameters.cpp):
        float partRelPos = 0.1f;          // rel_position: strike position 0.0001..0.999
        float partFreqDamp = -3.5f;       // freq_damp: 10^x frequency-dependent damping, -6..-1
        float partStiffness = 4.6f;       // stiffness: 10^x strike gain, 2..6
        float partStiffExp = 2.3f;        // stiffness_exp: strike nonlinearity exponent, 1..3
        int partCount = 32;               // partials: active resonator count, 16..64
        float satAmount = 0.0f;
        float satUser = 0.0f;       // cached getSafeSatAmount(satAmount, satType)
        bool satUserDirty = true;
        float satStressOnset = 0.86f;
        float satStressKnee = 0.995f;
        float mix = 1.0f;
        bool solo = false;
        bool enabled = true;
        float miniDispState[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        float cachedLpfFreq = -1.0f;
        float cachedHpfFreq = -1.0f;
        float cachedSampleRate = -1.0f;
        std::atomic<int> pendingSize{0};
        int transitionOldSize = 0;
        int transitionOldWriteIndex = 0;
        int transitionSamplesLeft = 0;
        int transitionSamplesTotal = 0;
        
        CombStage() {
            std::memset(buffer, 0, sizeof(buffer));
            std::memset(flangeBuffer, 0, sizeof(flangeBuffer));
        }

        CombStage(CombStage&& other) noexcept 
            : bufferSize(other.bufferSize),
              writeIndex(other.writeIndex),
              feedback(other.feedback),
                            lpfFreq(other.lpfFreq),
                            hpfFreq(other.hpfFreq),
                            lpfFilter(other.lpfFilter),
                            hpfFilter(other.hpfFilter),
                            satType(other.satType),
                            filterType(other.filterType),
                            tunedFreq(other.tunedFreq),
                            biquad(other.biquad),
                            crossFb(other.crossFb),
                            modeAmount(other.modeAmount),
                            allpassFb(other.allpassFb),
                            glottalPhase(other.glottalPhase),
                            glottalPeriod(other.glottalPeriod),
                            lfoPhase(other.lfoPhase),
                            lfoRate(other.lfoRate),
                            lfoTick(other.lfoTick),
                            flangeWriteIndex(other.flangeWriteIndex),
                            pmPhase(other.pmPhase),
                            crushPhase(other.crushPhase),
                            crushedWet(other.crushedWet),
                            popTimer(other.popTimer),
                            rngState(other.rngState),
                            stutPos(other.stutPos),
                            stutStart(other.stutStart),
                            stutLen(other.stutLen),
                            stutTimer(other.stutTimer),
                            cachedBqFreq(other.cachedBqFreq),
                            cachedBqFeedback(other.cachedBqFeedback),
                            cachedBqSr(other.cachedBqSr),
                            cachedBqModeA(other.cachedBqModeA),
                            cachedBqModeB(other.cachedBqModeB),
                            explicitQ(other.explicitQ),
                            cachedBqQ(other.cachedBqQ),
                            cachedPartFreq(other.cachedPartFreq),
                            cachedPartAmt(other.cachedPartAmt),
                            cachedPartRate(other.cachedPartRate),
                            cachedPartSr(other.cachedPartSr),
                            cachedPartRelPos(other.cachedPartRelPos),
                            cachedPartFreqDamp(other.cachedPartFreqDamp),
                            cachedPartStiffness(other.cachedPartStiffness),
                            cachedPartStiffExp(other.cachedPartStiffExp),
                            cachedPartCount(other.cachedPartCount),
                            modeRate(other.modeRate),
                            modeA(other.modeA),
                            modeB(other.modeB),
                            partRelPos(other.partRelPos),
                            partFreqDamp(other.partFreqDamp),
                            partStiffness(other.partStiffness),
                            partStiffExp(other.partStiffExp),
                            partCount(other.partCount),
                            satAmount(other.satAmount),
                            satUser(other.satUser),
                            satUserDirty(other.satUserDirty),
                            satStressOnset(other.satStressOnset),
                                            satStressKnee(other.satStressKnee),
                              mix(other.mix),
                              solo(other.solo),
                              enabled(other.enabled),
                              cachedLpfFreq(other.cachedLpfFreq),
                              cachedHpfFreq(other.cachedHpfFreq),
                              cachedSampleRate(other.cachedSampleRate) {
            std::memcpy(buffer, other.buffer, sizeof(buffer));
            std::memcpy(miniDispState, other.miniDispState, sizeof(miniDispState));
            std::memcpy(phaserStages, other.phaserStages, sizeof(phaserStages));
            std::memcpy(formantStages, other.formantStages, sizeof(formantStages));
            std::memcpy(flangeBuffer, other.flangeBuffer, sizeof(flangeBuffer));
            std::memcpy(partials, other.partials, sizeof(partials));
            pendingSize.store(other.pendingSize.load());
            transitionOldSize = other.transitionOldSize;
            transitionOldWriteIndex = other.transitionOldWriteIndex;
            transitionSamplesLeft = other.transitionSamplesLeft;
            transitionSamplesTotal = other.transitionSamplesTotal;
        }

        CombStage(const CombStage& other) 
            : bufferSize(other.bufferSize),
              writeIndex(other.writeIndex),
              feedback(other.feedback),
                            lpfFreq(other.lpfFreq),
                            hpfFreq(other.hpfFreq),
                            lpfFilter(other.lpfFilter),
                            hpfFilter(other.hpfFilter),
                            satType(other.satType),
                            filterType(other.filterType),
                            tunedFreq(other.tunedFreq),
                            biquad(other.biquad),
                            crossFb(other.crossFb),
                            modeAmount(other.modeAmount),
                            allpassFb(other.allpassFb),
                            glottalPhase(other.glottalPhase),
                            glottalPeriod(other.glottalPeriod),
                            lfoPhase(other.lfoPhase),
                            lfoRate(other.lfoRate),
                            lfoTick(other.lfoTick),
                            flangeWriteIndex(other.flangeWriteIndex),
                            pmPhase(other.pmPhase),
                            crushPhase(other.crushPhase),
                            crushedWet(other.crushedWet),
                            popTimer(other.popTimer),
                            rngState(other.rngState),
                            stutPos(other.stutPos),
                            stutStart(other.stutStart),
                            stutLen(other.stutLen),
                            stutTimer(other.stutTimer),
                            cachedBqFreq(other.cachedBqFreq),
                            cachedBqFeedback(other.cachedBqFeedback),
                            cachedBqSr(other.cachedBqSr),
                            cachedBqModeA(other.cachedBqModeA),
                            cachedBqModeB(other.cachedBqModeB),
                            explicitQ(other.explicitQ),
                            cachedBqQ(other.cachedBqQ),
                            cachedPartFreq(other.cachedPartFreq),
                            cachedPartAmt(other.cachedPartAmt),
                            cachedPartRate(other.cachedPartRate),
                            cachedPartSr(other.cachedPartSr),
                            cachedPartRelPos(other.cachedPartRelPos),
                            cachedPartFreqDamp(other.cachedPartFreqDamp),
                            cachedPartStiffness(other.cachedPartStiffness),
                            cachedPartStiffExp(other.cachedPartStiffExp),
                            cachedPartCount(other.cachedPartCount),
                            modeRate(other.modeRate),
                            modeA(other.modeA),
                            modeB(other.modeB),
                            partRelPos(other.partRelPos),
                            partFreqDamp(other.partFreqDamp),
                            partStiffness(other.partStiffness),
                            partStiffExp(other.partStiffExp),
                            partCount(other.partCount),
                            satAmount(other.satAmount),
                            satUser(other.satUser),
                            satUserDirty(other.satUserDirty),
                            satStressOnset(other.satStressOnset),
                                            satStressKnee(other.satStressKnee),
                              mix(other.mix),
                              solo(other.solo),
                              enabled(other.enabled),
                              cachedLpfFreq(other.cachedLpfFreq),
                              cachedHpfFreq(other.cachedHpfFreq),
                              cachedSampleRate(other.cachedSampleRate) {
            std::memcpy(buffer, other.buffer, sizeof(buffer));
            std::memcpy(miniDispState, other.miniDispState, sizeof(miniDispState));
            std::memcpy(phaserStages, other.phaserStages, sizeof(phaserStages));
            std::memcpy(formantStages, other.formantStages, sizeof(formantStages));
            std::memcpy(flangeBuffer, other.flangeBuffer, sizeof(flangeBuffer));
            std::memcpy(partials, other.partials, sizeof(partials));
            pendingSize.store(other.pendingSize.load());
            transitionOldSize = other.transitionOldSize;
            transitionOldWriteIndex = other.transitionOldWriteIndex;
            transitionSamplesLeft = other.transitionSamplesLeft;
            transitionSamplesTotal = other.transitionSamplesTotal;
        }

        CombStage& operator=(const CombStage& other) {
            if (this != &other) {
                bufferSize = other.bufferSize;
                writeIndex = other.writeIndex;
                feedback = other.feedback;
                lpfFreq = other.lpfFreq;
                hpfFreq = other.hpfFreq;
                lpfFilter = other.lpfFilter;
                hpfFilter = other.hpfFilter;
                satType = other.satType;
                filterType = other.filterType;
                tunedFreq = other.tunedFreq;
                biquad = other.biquad;
                crossFb = other.crossFb;
                modeAmount = other.modeAmount;
                allpassFb = other.allpassFb;
                glottalPhase = other.glottalPhase;
                glottalPeriod = other.glottalPeriod;
                lfoPhase = other.lfoPhase;
                lfoRate = other.lfoRate;
                lfoTick = other.lfoTick;
                flangeWriteIndex = other.flangeWriteIndex;
                pmPhase = other.pmPhase;
                crushPhase = other.crushPhase;
                crushedWet = other.crushedWet;
                popTimer = other.popTimer;
                rngState = other.rngState;
                stutPos = other.stutPos;
                stutStart = other.stutStart;
                stutLen = other.stutLen;
                stutTimer = other.stutTimer;
                cachedBqFreq = other.cachedBqFreq;
                cachedBqFeedback = other.cachedBqFeedback;
                cachedBqSr = other.cachedBqSr;
                cachedBqModeA = other.cachedBqModeA;
                cachedBqModeB = other.cachedBqModeB;
                explicitQ = other.explicitQ;
                cachedBqQ = other.cachedBqQ;
                cachedPartFreq = other.cachedPartFreq;
                cachedPartAmt = other.cachedPartAmt;
                cachedPartRate = other.cachedPartRate;
                cachedPartSr = other.cachedPartSr;
                cachedPartRelPos = other.cachedPartRelPos;
                cachedPartFreqDamp = other.cachedPartFreqDamp;
                cachedPartStiffness = other.cachedPartStiffness;
                cachedPartStiffExp = other.cachedPartStiffExp;
                cachedPartCount = other.cachedPartCount;
                modeRate = other.modeRate;
                modeA = other.modeA;
                modeB = other.modeB;
                partRelPos = other.partRelPos;
                partFreqDamp = other.partFreqDamp;
                partStiffness = other.partStiffness;
                partStiffExp = other.partStiffExp;
                partCount = other.partCount;
                satAmount = other.satAmount;
                satUser = other.satUser;
                satUserDirty = other.satUserDirty;
                satStressOnset = other.satStressOnset;
                satStressKnee = other.satStressKnee;
                mix = other.mix;
                solo = other.solo;
                enabled = other.enabled;
                cachedLpfFreq = other.cachedLpfFreq;
                cachedHpfFreq = other.cachedHpfFreq;
                cachedSampleRate = other.cachedSampleRate;
                std::memcpy(buffer, other.buffer, sizeof(buffer));
                std::memcpy(miniDispState, other.miniDispState, sizeof(miniDispState));
                std::memcpy(phaserStages, other.phaserStages, sizeof(phaserStages));
                std::memcpy(formantStages, other.formantStages, sizeof(formantStages));
                    std::memcpy(flangeBuffer, other.flangeBuffer, sizeof(flangeBuffer));
                std::memcpy(partials, other.partials, sizeof(partials));
                pendingSize.store(other.pendingSize.load());
                transitionOldSize = other.transitionOldSize;
                transitionOldWriteIndex = other.transitionOldWriteIndex;
                transitionSamplesLeft = other.transitionSamplesLeft;
                transitionSamplesTotal = other.transitionSamplesTotal;
            }
            return *this;
        }

        void updateFilterCoefficients(float sr) {
            if (sr <= 0.0f) {
                sr = 44100.0f;
            }

            if (cachedSampleRate == sr &&
                cachedLpfFreq == lpfFreq &&
                cachedHpfFreq == hpfFreq) {
                return;
            }

            lpfFilter.setCutoffHz(lpfFreq, sr);
            hpfFilter.setCutoffHz(hpfFreq, sr);
            cachedSampleRate = sr;
            cachedLpfFreq = lpfFreq;
            cachedHpfFreq = hpfFreq;
        }

        // FT_PARTIAL_* modal resonator bank: recompute all active 2-pole
        // resonators when any model parameter changed. Knob mapping mirrors
        // the Saike sliders: damp slider -2..2 over modeAmount 0..1,
        // inharmonic -4..0 over modeRate 0..1, freq_damp / rel_position /
        // stiffness / stiffness_exp / partials are their own params.
        void updatePartialCoefficients(int model, float sr) {
            if (tunedFreq == cachedPartFreq && modeAmount == cachedPartAmt &&
                modeRate == cachedPartRate && sr == cachedPartSr &&
                partRelPos == cachedPartRelPos && partFreqDamp == cachedPartFreqDamp &&
                partStiffness == cachedPartStiffness && partStiffExp == cachedPartStiffExp &&
                partCount == cachedPartCount) return;
            const float baseDamp = std::pow(10.0f, -2.0f + 4.0f * modeAmount);
            const float inharmCoef = std::pow(10.0f, -4.0f + 4.0f * modeRate);
            const int count = std::clamp(partCount, 1, kMaxPartials);
            for (int p = 0; p < kMaxPartials; ++p) {
                if (p < count)
                    configurePartial(model, p, tunedFreq, inharmCoef, baseDamp, partFreqDamp, partRelPos, sr, partials[p]);
                else
                    partials[p] = PartialResonator{};
            }
            cachedPartFreq = tunedFreq; cachedPartAmt = modeAmount;
            cachedPartRate = modeRate; cachedPartSr = sr;
            cachedPartRelPos = partRelPos; cachedPartFreqDamp = partFreqDamp;
            cachedPartStiffness = partStiffness; cachedPartStiffExp = partStiffExp;
            cachedPartCount = (float)partCount;
        }

        static inline float processAllpassStage(float x, float a, float& z) {
            const float y = -a * x + z;
            z = x + a * y;
            return y;
        }

        // Cubic (Catmull-Rom) fractional read from a circular buffer. size =
        // buffer length, writeIdx = current write position, delaySamples =
        // distance back to read (fractional allowed; must stay < size).
        // ponytail: if magnitude-response purity during modulation ever matters,
        // swap for a Thiran allpass — the read sites are isolated in this helper.
        static inline float cubicRead(const float* buf, int size, int writeIdx, float delaySamples) {
            if (size <= 0) return 0.0f;
            const float idxF = (float)writeIdx - delaySamples;
            const int i0 = (int)std::floor(idxF);
            const float frac = idxF - (float)i0;
            const int a0 = ((i0 % size) + size) % size;
            const int i1 = (a0 + 1) % size, i2 = (a0 + 2) % size, i3 = (a0 + 3) % size;
            const float d0 = buf[a0], d1 = buf[i1], d2 = buf[i2], d3 = buf[i3];
            const float a = -0.5f*d0 + 1.5f*d1 - 1.5f*d2 + 0.5f*d3;
            const float b = d0 - 2.5f*d1 + 2.0f*d2 - 0.5f*d3;
            const float c = -0.5f*d0 + 0.5f*d2;
            return ((a*frac + b)*frac + c)*frac + d1;
        }

        float applyMiniDispersion(float x, float amount) {
            if (amount <= 1e-5f) return x;

            const float depth = std::pow(std::clamp(amount, 0.0f, 1.0f), 0.85f);
            const float baseCoeff[4] = {0.27f, 0.39f, 0.53f, 0.67f};
            const int stageCount = (amount < 0.34f) ? 2 : ((amount < 0.67f) ? 3 : 4);

            float y = x;
            for (int i = 0; i < stageCount; ++i) {
                const float a = std::min(0.92f, baseCoeff[i] * depth);
                y = processAllpassStage(y, a, miniDispState[i]);
                if (std::abs(miniDispState[i]) < 1e-18f) miniDispState[i] = 0.0f;
            }
            return y;
        }

        static float applySaturator(float x, int type) {
            switch (type) {
                case SAT_SOFTSIGN:
                    return x / (1.0f + std::abs(x));
                case SAT_ARCTAN:
                    return std::atan(2.0f * x) * (2.0f / (float)M_PI);
                case SAT_CUBIC: {
                    const float c = std::clamp(x, -1.5f, 1.5f);
                    return c - (c * c * c) / 3.0f;
                }
                case SAT_HARDCLIP:
                    return std::clamp(x, -1.0f, 1.0f);
                case SAT_FOLDBACK: {
                    float y = x;
                    if (y > 1.0f)       y = 2.0f - y;
                    else if (y < -1.0f) y = -2.0f - y;
                    if (y > 1.0f)       y = 2.0f - y;
                    else if (y < -1.0f) y = -2.0f - y;
                    return y;
                }
                case SAT_ASYMMETRIC:
                    if (x >= 0.0f) return 1.0f - std::exp(-x * 1.5f);
                    else           return std::tanh(x * 0.8f);
                case SAT_SINE:
                    return std::sin(x * (float)M_PI * 0.5f);
                case SAT_TANH:
                default:
                    return std::tanh(x);
            }
        }

        static float smoothstep(float edge0, float edge1, float x) {
            float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
            return t * t * (3.0f - 2.0f * t);
        }

        // Keep each saturator in a safe range so loop energy does not build into
        // a synthetic feedback whistle when feedback is already high.
        static float getSafeSatAmount(float amount, int type) {
            const float a = std::clamp(amount, 0.0f, 1.0f);
            if (a <= 0.0f) return 0.0f; // pow(0, 1.25) * maxAmt == 0
            float maxAmt = 0.45f;
            switch (type) {
                case SAT_TANH:       maxAmt = 0.55f; break;
                case SAT_SOFTSIGN:   maxAmt = 0.50f; break;
                case SAT_ARCTAN:     maxAmt = 0.45f; break;
                case SAT_CUBIC:      maxAmt = 0.40f; break;
                case SAT_HARDCLIP:   maxAmt = 0.80f; break;
                case SAT_FOLDBACK:   maxAmt = 0.70f; break;
                case SAT_ASYMMETRIC: maxAmt = 0.55f; break;
                case SAT_SINE:       maxAmt = 0.50f; break;
                default: break;
            }
            // Slightly compress control near the top for finer low-end behavior.
            return std::pow(a, 1.25f) * maxAmt;
        }

        float satUserValue() {
            if (satUserDirty) {
                satUser = getSafeSatAmount(satAmount, satType);
                satUserDirty = false;
            }
            return satUser;
        }

        float process(float input, float sr, float miniDispAmount, float memoryDamping, float drainFactor = 1.0f) {
            int ps = pendingSize.exchange(0);
            if (ps > 0) {
                const int oldSize = bufferSize;
                const int oldWrite = writeIndex;

                bufferSize = std::min(ps, kMaxBufferSize);
                if (bufferSize <= 0) bufferSize = 1;
                writeIndex %= bufferSize;

                const float damp = std::clamp(memoryDamping, 0.0f, 1.0f);
                if (damp > 1e-5f && oldSize > 0 && oldSize != bufferSize) {
                    const float fadeSec = 0.002f + 0.058f * damp;
                    transitionSamplesTotal = std::max(1, (int)(fadeSec * sr));
                    transitionSamplesLeft = transitionSamplesTotal;
                    transitionOldSize = oldSize;
                    transitionOldWriteIndex = oldWrite % oldSize;
                } else {
                    transitionSamplesTotal = 0;
                    transitionSamplesLeft = 0;
                    transitionOldSize = 0;
                    transitionOldWriteIndex = 0;
                }
            }
            
            // ---- Non-comb filter topologies. Each branch reshapes `input` and
            // falls into the shared tail below (returns the full processed signal).
            if (filterType == FT_PHASER) {
                // 4 cascaded biquad allpasses, center swept ±1 octave by the
                // per-stage LFO. Coefficients refresh every 16 samples
                // (per-sample sin/cos for 4 stages × 40 stages would be too
                // heavy); the LFO moves at sub-audio rates.
                const int block = 16;
                const int tick = (int)(lfoPhase * (float)block / (2.0f * (float)M_PI));
                if (tick != lfoTick) {
                    lfoTick = tick;
                    const float sweep = 0.5f + 0.5f * std::sin(lfoPhase); // 0..1
                    // A = sweep depth: 0.5 -> ±1 octave (the old fixed value).
                    const float depthOct = 2.0f * modeA;
                    const float center = std::clamp(tunedFreq, 20.0f, 0.45f * sr)
                                       * std::pow(2.0f, (sweep - 0.5f) * 2.0f * depthOct);
                    const float q = effectiveQ(0.7f, 4.3f);
                    for (int i = 0; i < 4; ++i) {
                        const float off = std::pow(2.0f, 0.25f * (float)(i - 1)); // 0.5x..2x spread
                        phaserStages[i].setParams(Biquad::ALL_PASS, std::min(0.45f * sr, center * off), 0.0f, q, sr);
                    }
                }
                lfoPhase += 2.0f * (float)M_PI * lfoRate / sr;
                if (lfoPhase >= 2.0f * (float)M_PI) lfoPhase -= 2.0f * (float)M_PI;
                float y = input, dummy = y;
                for (int i = 0; i < 4; ++i) { phaserStages[i].process(y, dummy); dummy = y; }
                // B = dry/wet (0.5 -> the old fixed 50/50).
                input = (1.0f - modeB) * input + modeB * y;
            } else if (filterType == FT_FLANGE) {
                // Modulated fractional delay read from flangeBuffer, cubic-interpolated.
                lfoPhase += 2.0f * (float)M_PI * lfoRate / sr;
                if (lfoPhase >= 2.0f * (float)M_PI) lfoPhase -= 2.0f * (float)M_PI;
                const float mod = 0.5f + 0.5f * std::sin(lfoPhase); // 0..1
                // A = delay centre (2..12 ms), B = sweep depth (0..10 ms).
                // Defaults 0.5/0.5 give the old fixed 2-12 ms sweep. Clamped so
                // the read never wraps past the write pointer (flangeBuffer
                // holds 2048 samples; 12 ms needs it all at 192 kHz).
                // Centre 2..12 ms, sweep depth 0..20 ms. B is halved by the
                // (mod - 0.5) term, so 20*B at B=0.5 gives the shipped 10 ms
                // sweep: 7 +/- 5 ms = the original 2..12 ms range.
                const float centreMs = 2.0f + 10.0f * modeA;
                const float sweepMs = 20.0f * modeB;
                const float delayMs = std::clamp(centreMs + (mod - 0.5f) * sweepMs, 0.25f, 20.0f);
                const float delaySamples = std::min(2047.0f, delayMs * 0.001f * sr);
                const float regen = std::clamp(feedback, 0.0f, 0.95f);   // regeneration into the delay line
                const float delayed = cubicRead(flangeBuffer, 2048, flangeWriteIndex, delaySamples);
                // Sanitized write: a non-finite write would regenerate forever (the
                // shared-tail isfinite guard clears input but not the delay line).
                const float wv = input + regen * delayed;
                flangeBuffer[flangeWriteIndex] = std::isfinite(wv) ? wv : 0.0f;
                flangeWriteIndex = (flangeWriteIndex + 1) % 2048;
                input = input + (0.3f + 0.7f * modeAmount) * delayed; // dry always present + wet tap from modeAmount
            } else if (filterType == FT_VOWEL || filterType == FT_VOCAL || filterType == FT_TALKBOX) {
                // Parallel formant filter: 3 BAND_PASS bands at tunedFreq x
                // {1.0, 2.7, 4.5} (TALKBOX morphs the ratios between two vowel
                // shapes at the mode LFO rate, depth from feedback), each
                // gain-compensated to unity at center (RBJ bandpass peaks at
                // Q) and summed. Feedback sharpens Q (2..8) and adds bounded
                // resonance (loop gain <= 0.5) — stable by construction, no
                // self-oscillation (the old serial cross-feedback loop rang at
                // F3 and clipped into noise).
                const float fb = std::clamp(feedback, 0.0f, 1.0f);
                const float q = effectiveQ(2.0f, 6.0f);
                float ratios[3];
                if (filterType == FT_TALKBOX) {
                    // "ah" {1.0, 2.7, 4.5} morphs toward "ee"-ish {0.65, 3.3, 4.0}
                    // as the LFO rises; fb scales the morph depth (0 = static vowel).
                    lfoPhase += 2.0f * (float)M_PI * lfoRate / sr;
                    if (lfoPhase >= 2.0f * (float)M_PI) lfoPhase -= 2.0f * (float)M_PI;
                    const float s = std::clamp(fb * (0.5f + 0.5f * std::sin(lfoPhase)) * (0.5f + modeB), 0.0f, 1.0f);
                    // A = formant spread, applied to the morph endpoints as well
                    // (0.5 -> the original "ah".."ee" range).
                    const float spread = 0.5f + modeA;
                    ratios[0] = 1.0f + (1.00f + s * (0.65f - 1.00f) - 1.0f) * spread;
                    ratios[1] = 1.0f + (2.70f + s * (3.30f - 2.70f) - 1.0f) * spread;
                    ratios[2] = 1.0f + (4.50f + s * (4.00f - 4.50f) - 1.0f) * spread;
                } else {
                    // A = formant spread: 0.5 -> the classic {1, 2.7, 4.5}.
                    const float spread = 0.5f + modeA;
                    ratios[0] = 1.0f;
                    ratios[1] = 1.0f + (2.7f - 1.0f) * spread;
                    ratios[2] = 1.0f + (4.5f - 1.0f) * spread;
                }
                // TALKBOX ratios move every sample -> recompute its coefficients
                // continuously; VOWEL/VOCAL use the freq/fb/sr cache.
                if (filterType == FT_TALKBOX || tunedFreq != cachedBqFreq || feedback != cachedBqFeedback
                    || sr != cachedBqSr || modeA != cachedBqModeA || modeB != cachedBqModeB
                    || explicitQ != cachedBqQ) {
                    for (int i = 0; i < 3; ++i) {
                        const float f = std::min(0.45f * sr, std::clamp(tunedFreq, 20.0f, 20000.0f) * ratios[i]);
                        formantStages[i].setParams(Biquad::BAND_PASS, f, 0.0f, q, sr);
                    }
                    // B shifts the glottal pitch around the old human-range value.
                    glottalPeriod = std::max(1.0f, sr / std::clamp(tunedFreq * (0.5f + modeB), 40.0f, 500.0f));
                    cachedBqFreq = tunedFreq; cachedBqFeedback = feedback; cachedBqSr = sr;
                    cachedBqModeA = modeA; cachedBqModeB = modeB;
                    cachedBqQ = explicitQ;
                }
                float node = input + crossFb;
                if (filterType == FT_VOCAL) {
                    // Glottal pulse injection, gated on input activity (threshold 0.01):
                    // narrow asymmetric spike (0.8, then -0.25), period = glottalPeriod.
                    if (std::abs(input) > 0.01f) {
                        glottalPhase += 1.0f;
                        if (glottalPhase >= glottalPeriod) glottalPhase = 0.0f;
                        const float pulse = (glottalPhase < 1.0f) ? 0.8f : (glottalPhase < 3.0f) ? -0.25f : 0.0f;
                        node += 0.3f * pulse;
                    } else {
                        glottalPhase = 0.0f;
                    }
                }
                // Per-band fresh copies: Biquad::process clobbers its input ref,
                // so the bands must each see the same node (parallel, not serial).
                float b0 = node, b1 = node, b2 = node, xdummy;
                xdummy = node; formantStages[0].process(b0, xdummy); b0 = xdummy;
                xdummy = node; formantStages[1].process(b1, xdummy); b1 = xdummy;
                xdummy = node; formantStages[2].process(b2, xdummy); b2 = xdummy;
                const float formantSum = (b0 + b1 + b2) / q; // unity at each center
                crossFb = 0.5f * fb * formantSum;            // bounded resonance (loop gain <= 0.5)
                input = formantSum;
            } else if (filterType == FT_ALLPASS) {
                // Transient smearing: 4 cascaded allpasses at closely-spaced
                // centers around the tuning (0.88x..1.12x) — phase crowding
                // disperses sharp peaks in time while the magnitude stays flat.
                // Feedback around the chain (0..0.6) adds the "phasey" wash;
                // wet/dry from modeAmount keeps the dry attack.
                if (tunedFreq != cachedBqFreq || feedback != cachedBqFeedback || sr != cachedBqSr ||
                    explicitQ != cachedBqQ) {
                    const float f = std::clamp(tunedFreq, 20.0f, std::min(20000.0f, 0.45f * sr));
                    const float q = effectiveQ(0.7f, 4.3f);
                    for (int k = 0; k < 4; ++k)
                        phaserStages[k].setParams(Biquad::ALL_PASS, f * (0.88f + 0.08f * (float)k), 0.0f, q, sr);
                    cachedBqFreq = tunedFreq; cachedBqFeedback = feedback; cachedBqSr = sr;
                    cachedBqQ = explicitQ;
                }
                float x = input + allpassFb;
                for (int k = 0; k < 4; ++k) { float dummy = x; phaserStages[k].process(x, dummy); x = dummy; }
                allpassFb = 0.6f * std::clamp(feedback, 0.0f, 1.0f) * x;
                const float wet = std::clamp(modeAmount, 0.0f, 1.0f);
                input = (1.0f - wet) * input + wet * x;
            } else if (isPartialType(filterType)) {
                // Saike Partials modal resonator bank: kMaxPartials parallel
                // damped 2-poles driven by the stage tuning. The summed
                // resonator output replaces `input` and falls into the shared
                // tail (mini-dispersion -> LPF/HPF -> saturator), which
                // returns early for non-comb-family types — same topology as
                // FT_VOWEL/FT_VOCAL (resonator bank, no comb delay-line loop).
                const int model = kPartialModelByType[filterType - FT_PARTIAL_BEAM_CLAMP];
                updatePartialCoefficients(model, sr);
                // Saike strike curve, blended with a linear floor. The pure
                // 10^stiffness * x^stiffnessExp form crushes sub-unit inputs
                // (x^2.3 at x=0.1 is 52 dB below linear) — the stage is a
                // filter type fed by the dispersed cymbal bus, whose peaks are
                // well below 1.0, so the default exponent made the partials
                // effectively inaudible. w = stiffExp/3 blends linear (w=0)
                // to the full strike curve (w=1); stiffness scales the
                // excitation (10^stiff). Sign-preserving (bipolar): Saike
                // rectifies, but a filter type should respond to both
                // excursions. At defaults (stiff 4.6, exp 2.3, |x|=1) the
                // blend is exactly 1.0, preserving the calibrated level.
                const float w = partStiffExp / 3.0f;
                const float mag = std::abs(input);
                const float strike = (1.0f - w) * mag + w * std::pow(mag, partStiffExp);
                const float fh = std::isfinite(input)
                    ? kPartialExciteGain * std::pow(10.0f, partStiffness - kPartialDefaultStiffness)
                        * ((input > 0.0f) ? strike : -strike)
                    : 0.0f;
                float sum = 0.0f;
                for (int p = 0; p < kMaxPartials; ++p) {
                    PartialResonator& pr = partials[p];
                    if (!pr.active) continue;
                    const float hist = pr.yh;
                    pr.yh = pr.y;
                    pr.y = pr.winb * fh - pr.a1 * pr.y - pr.a2 * hist;
                    if (std::abs(pr.y) < 1e-20f) pr.y = 0.0f; // denormal flush, matches the shared-tail pattern
                    sum += pr.wout * pr.y;
                }
                // Per-model level normalization (defaults sit at the comb level).
                input = sum * kPartialOutScale * kPartialModelGain[model];
            } else if (filterType == FT_NOTCH || filterType == FT_RESO_LP ||
                       filterType == FT_RESO_BP || filterType == FT_RESO_HP) {
                // Biquad resonator family — Q mapped from the feedback knob.
                if (tunedFreq != cachedBqFreq || feedback != cachedBqFeedback || sr != cachedBqSr ||
                    explicitQ != cachedBqQ) {
                    const float f = std::clamp(tunedFreq, 20.0f, std::min(20000.0f, 0.45f * sr));
                    const float q = effectiveQ(0.7f, 9.3f); // AUTO: [0.7, ~10]
                    Biquad::Type t = Biquad::BAND_PASS;
                    if      (filterType == FT_NOTCH)   t = Biquad::NOTCH;
                    else if (filterType == FT_RESO_LP) t = Biquad::LPF;
                    else if (filterType == FT_RESO_HP) t = Biquad::HPF;
                    biquad.setParams(t, f, 0.0f, q, sr);
                    cachedBqFreq = tunedFreq; cachedBqFeedback = feedback; cachedBqSr = sr;
                    cachedBqQ = explicitQ;
                }
                float dummy = input;
                biquad.process(input, dummy); // L channel carries the mono signal
            }

            // Shared tail — runs for every mode EXCEPT the five comb-family modes
            // (FT_COMB, FT_GLITCH_PM, FT_STUTTER, FT_BITCRUSH, FT_POP), which fall
            // through into the existing comb body instead.
            if (filterType != FT_COMB && filterType != FT_GLITCH_PM &&
                filterType != FT_STUTTER && filterType != FT_BITCRUSH && filterType != FT_POP) {
                input = applyMiniDispersion(input, miniDispAmount);
                updateFilterCoefficients(sr);
                input = hpfFilter.processHP(lpfFilter.processLP(input));
                const float satUser = satUserValue();
                const float fbGain = feedback * 0.998f;
                const float stress = smoothstep(satStressOnset, satStressKnee, fbGain);
                const float sat = satUser * stress;
                if (sat > 1e-5f) {
                    const float drive = 1.0f + sat * 3.0f;
                    const float satSig = applySaturator(input * drive, satType) / drive;
                    input = input * (1.0f - sat) + satSig * sat;
                }
                if (std::abs(input) < 1e-12f) input = 0.0f;
                if (!std::isfinite(input)) {
                    biquad.clear();
                    for (auto& ps : phaserStages) ps.clear();
                    for (auto& fs : formantStages) fs.clear();
                    // The one-poles latch NaN too (processLP: state += alpha*(in-state));
                    // clear them or the stage stays silent forever.
                    lpfFilter.clear();
                    hpfFilter.clear();
                    input = 0.0f;
                }
                input = std::max(-16.0f, std::min(16.0f, input));
                return input;
            }

            int readIndex = (writeIndex + 1) % bufferSize;
            float wet = buffer[readIndex];
            wet *= drainFactor;

            if (transitionSamplesLeft > 0 && transitionOldSize > 0) {
                const int oldReadIndex = (transitionOldWriteIndex + 1) % transitionOldSize;
                const float wetOld = buffer[oldReadIndex];
                const float t = 1.0f - ((float)transitionSamplesLeft / (float)transitionSamplesTotal);
                wet = wetOld * (1.0f - t) + wet * t;

                transitionOldWriteIndex = (transitionOldWriteIndex + 1) % transitionOldSize;
                --transitionSamplesLeft;
                if (transitionSamplesLeft <= 0) {
                    transitionSamplesTotal = 0;
                    transitionOldSize = 0;
                    transitionOldWriteIndex = 0;
                }
            }

            if (filterType == FT_GLITCH_PM) {
                // Audio-rate phase modulation of the delay read; the modulator tracks
                // tuning so the scale engine shifts everything coherently. Depth is
                // 5-25% of a rotation from feedback (alias-free: a delay ≥ bufferSize
                // would wrap back toward the write pointer).
                // B = modulator ratio (0.5 -> the old fixed 0.5 x tuned freq);
                // A = depth scale (0.5 -> x1.0, i.e. the old 5..25% sweep).
                pmPhase += 2.0f * (float)M_PI * (std::max(0.01f, modeB) * tunedFreq) / sr;
                if (pmPhase >= 2.0f * (float)M_PI) pmPhase -= 2.0f * (float)M_PI;
                const float pmDepth = std::clamp((0.05f + 0.20f * feedback) * (0.5f + modeA), 0.0f, 0.45f);
                wet = cubicRead(buffer, bufferSize, writeIndex,
                                (float)(bufferSize - 1) * (1.0f - pmDepth * (0.5f + 0.5f * std::sin(pmPhase))));
            } else if (filterType == FT_STUTTER) {
                // Asynchronous windowed scrub: pick a random [10,50]ms window
                // (clamped to buffer length) and read it cyclically;
                // re-randomize every 20-80 ms (or immediately if a buffer resize
                // shrank the delay line past the cached window).
                if (stutTimer <= 0 || stutStart + stutLen > bufferSize) {
                    const float winSec = 0.010f + 0.040f * feedback; // 10-50 ms
                    // A scales the window AFTER clamping it to the delay line:
                    // scaling winSec before the clamp is a no-op whenever the
                    // window is already longer than the buffer (which is most
                    // mid/high tunings), so it has to act on the clamped length.
                    const int defLen = std::min(bufferSize, std::max(1, (int)(winSec * sr)));
                    const int maxLen = std::max(1, std::min(bufferSize, (int)(defLen * (0.5f + modeA))));
                    const int minLen = std::min(maxLen, std::max(1, (int)(0.4f * maxLen)));
                    stutLen = minLen + (int)(xorshift32Unit(rngState) * (float)(maxLen - minLen + 1)) % (maxLen - minLen + 1);
                    stutLen = std::max(1, std::min(bufferSize, stutLen));
                    stutStart = (bufferSize > stutLen) ? (int)(xorshift32Unit(rngState) * (float)(bufferSize - stutLen)) : 0;
                    stutPos = stutStart;
                    stutTimer = (int)(0.020f * sr * (0.5f + modeB))
                              + (int)(xorshift32Unit(rngState) * (float)(0.060f * sr * (0.5f + modeB)));
                }
                --stutTimer;
                wet = buffer[stutPos];
                stutPos = (stutPos + 1 >= stutStart + stutLen) ? stutStart : stutPos + 1;
            }

            // Per-resonator mini-dispersion sits in the feedback path before damping.
            wet = applyMiniDispersion(wet, miniDispAmount);

            if (filterType == FT_BITCRUSH) {
                // Feedback shortens the hold (2->1 tuning periods) and coarsens the
                // quantization (5->3 bits); at feedback 0 this is the original behavior.
                // B scales the sample&hold rate around the old value; A offsets
                // the bit depth (0.5 -> 0 extra bits, so defaults are unchanged).
                const float holdFreq = (0.5f + 0.5f * feedback) * tunedFreq * (0.5f + modeB); // [0.25, 1.5] x old
                const float hold = std::max(1.0f, (holdFreq > 1e-3f) ? sr / holdFreq : (float)bufferSize);
                const int bits = std::clamp((int)std::lround(5.0f - 2.0f * feedback + (modeA - 0.5f) * 8.0f), 1, 16);
                // levels = 2^(bits-1): 16 at fb 0 (bit-identical to the shipped
                // round(x*16)/16 quantizer), 4 at fb 0.9.
                const float levels = (float)(1 << (bits - 1));
                if (crushPhase >= hold) {
                    crushPhase = 0.0f;
                    crushedWet = std::round(std::clamp(wet, -1.0f, 1.0f) * levels) / levels;
                }
                ++crushPhase;
                wet = crushedWet;
            }

            updateFilterCoefficients(sr);
            float filteredWet = hpfFilter.processHP(lpfFilter.processLP(wet));

            const float satUser = satUserValue();
            const float fbGain = feedback * 0.998f;

            // "Material stress": saturator only engages strongly as feedback rises.
            // This keeps low/medium feedback musical and adds limiting behavior near runaway.
            const float stressByFeedback = smoothstep(satStressOnset, satStressKnee, fbGain);
            const float sat = satUser * stressByFeedback;

            float feedbackSignal = filteredWet * fbGain;
            if (filterType == FT_POP) {
                // Hard clip + conditional scramble. Feedback scales both the loop clip and
                // the scramble trigger (0.6+0.4*fb): at low feedback the loop is quieter,
                // so a lower trigger keeps pops firing; at high feedback this approaches
                // the original +-1.5 / 2.0. Trigger reads the UN-clipped loop signal.
                // A scales the clip/trigger (0.5 -> the old fixed level);
                // B scales the cooldown (0.5 -> 20 ms).
                const float popScale = 0.5f + modeA;
                const float popClip = 1.5f * (0.6f + 0.4f * feedback) * popScale;    // [0.9, 1.5] x scale
                const float popTrigger = 2.0f * (0.6f + 0.4f * feedback) * popScale; // [1.2, 2.0] x scale
                if (std::abs(feedbackSignal) > popTrigger && popTimer <= 0.0f) {
                    popTimer = 0.02f * sr * (0.5f + modeB); // 10..30 ms at defaults 20 ms
                    for (int k = 0; k < bufferSize; k += 3) buffer[k] = -buffer[k]; // invert every 3rd sample
                    for (int k = 0; k < bufferSize; ++k) buffer[k] *= 0.5f;
                }
                if (popTimer > 0.0f) --popTimer;
                feedbackSignal = std::max(-popClip, std::min(popClip, feedbackSignal));
            }
            if (sat > 1e-5f) {
                const float drive = 1.0f + sat * 3.0f;
                const float satSig = applySaturator(feedbackSignal * drive, satType) / drive;
                feedbackSignal = feedbackSignal * (1.0f - sat) + satSig * sat;
            }
            // The returned wet is the group bus contribution. Bound it like the
            // shared tail and the buffer write: one-pole damping can overshoot a
            // clamped buffer sample, which at max feedback drove FT_STUTTER (and
            // any comb-family mode) past the nominal +-16 range.
            if (!std::isfinite(feedbackSignal)) feedbackSignal = 0.0f;
            feedbackSignal = std::max(-16.0f, std::min(16.0f, feedbackSignal));
            float newSample = input + feedbackSignal;
            
            if (std::abs(newSample) < 1e-12f) newSample = 0.0f;
            if (std::isnan(newSample) || std::isinf(newSample)) {
                newSample = 0.0f;
                feedbackSignal = 0.0f;
                std::memset(buffer, 0, sizeof(buffer));
            }
            
            newSample = std::max(-16.0f, std::min(16.0f, newSample));
            buffer[writeIndex] = newSample;
            writeIndex = (writeIndex + 1) % bufferSize;

            // Return only the resonant (feedback) contribution.
            // The buffer stores input+feedback for correct comb delay topology,
            // but callers accumulate only the wet resonance into the group bus.
            return feedbackSignal;
        }
    };

    struct GroupBus {
        OnePoleFilter lpf;
        OnePoleFilter hpf;
        float lpfFreq = 20000.0f;
        float hpfFreq = 400.0f;
        float gain = 1.0f;
        bool isSeries = false;
        FrequencyShifter shifter;
        float shiftFreq = 0.0f;
        // ---- Per-group peak limiter (OFF BY DEFAULT) ----
        // limiterOn is the zero-cost gate: process()/processStereo() early-out
        // on it before touching ANY of the state below, so while a group's
        // limiter is off the group bus stays bit-identical to a build without
        // this code (no envelope advance, no gain smoothing, no state drift).
        bool limiterOn = false;
        float limiterThresholdLin = 0.5f;   // linear ceiling applied to the group sum
        float limiterReleaseSec = 0.2f;     // control kept so setSampleRate rebuilds the coeff
        float limiterReleaseCoeff = 0.999f; // one-pole coefficient toward unity gain
        float limiterGain = 1.0f;           // smoothed gain actually applied
    };

    CombFilterBank(float sampleRate = 44100.0f) : sampleRate(sampleRate) {
        for (int i = 0; i < 40; ++i) {
            stageScaleFactors[i] = 1.0f;
            baseFrequencies[i] = 440.0f;
            // Per-stage detune prevents 40 stages phase-locking their LFOs.
            stages[i].lfoRate = 0.18f + (float)i * 0.013f; // 0.18..0.69 Hz
            stages[i].rngState = 0x9E3779B9u ^ (uint32_t)(i * 2654435761u);
        }
        for (int i = 0; i < 10; ++i) clusterAmounts[i] = 0.0f;
        for (auto& grp : groups) {
            grp.shifter.setSampleRate(sampleRate);
            grp.gain = 1.0f;
        }
    }
    
    void setSampleRate(float sr) {
        sampleRate = sr;
        for (auto& grp : groups) {
            grp.shifter.setSampleRate(sr);
            // The release one-pole is rate dependent; rebuild it from the
            // stored control (never from the sample count).
            const float s = std::max(1.0f, sr);
            grp.limiterReleaseCoeff =
                std::exp(-1.0f / (grp.limiterReleaseSec * s));
        }
    }

    void startSoftClear(float fadeSec = 0.05f) {
        m_drainSamplesTotal = std::max(1, (int)(fadeSec * sampleRate));
        m_drainSamplesLeft = m_drainSamplesTotal;
    }
    
    void setFrequency(int index, float freq) {
        if (index < 0 || index >= 40) return;
        baseFrequencies[index] = freq;
        applyScaledFrequency(index);
    }

    void setScaleFactor(float scale) {
        scaleFactor = std::max(0.001f, scale);
        for (int i = 0; i < 40; ++i) applyScaledFrequency(i);
    }

    // Global musical transposition applied after the per-stage scale.
    void setStageScaleFactor(int index, float scale) {
        if (index < 0 || index >= 40) return;
        stageScaleFactors[index] = std::max(0.001f, scale);
        applyScaledFrequency(index);
    }

    void setFeedback(int index, float fb) {
        if (index < 0 || index >= 40) return;
        stages[index].feedback = std::clamp(fb, 0.0f, 0.998f);
    }

    void setFilterQ(int index, float q) {
        if (index < 0 || index >= 40) return;
        stages[index].explicitQ = q;
        stages[index].cachedBqQ = -2.0f;
    }

    void setLPF(int index, float freq) {
        if (index < 0 || index >= 40) return;
        stages[index].lpfFreq = freq;
    }

    void setHPF(int index, float freq) {
        if (index < 0 || index >= 40) return;
        stages[index].hpfFreq = freq;
    }

    void setSaturatorType(int index, int type) {
        if (index < 0 || index >= 40) return;
        const int t = std::clamp(type, 0, 7);
        if (t != stages[index].satType) {
            stages[index].satType = t;
            stages[index].satUserDirty = true;
        }
    }

    void setFilterType(int index, int type) {
        if (index < 0 || index >= 40) return;
        const int t = std::clamp(type, 0, (int)FT_COUNT - 1);
        if (t == stages[index].filterType) return;
        stages[index].filterType = t;
        // State clear on topology switch (mirror setStageEnabled cleanup).
        std::memset(stages[index].buffer, 0, sizeof(stages[index].buffer));
        stages[index].writeIndex = 0;
        stages[index].biquad.clear();
        for (auto& ps : stages[index].phaserStages) ps.clear();
        for (auto& fs : stages[index].formantStages) fs.clear();
        stages[index].lpfFilter.clear();
        stages[index].hpfFilter.clear();
        stages[index].cachedSampleRate = -1.0f;
        stages[index].cachedLpfFreq = -1.0f;
        stages[index].cachedHpfFreq = -1.0f;
        stages[index].cachedBqFreq = -1.0f;
        stages[index].cachedBqFeedback = -1.0f;
        stages[index].cachedBqSr = -1.0f;
        for (int s = 0; s < 4; ++s) stages[index].miniDispState[s] = 0.0f;
        stages[index].transitionOldSize = 0;
        stages[index].transitionOldWriteIndex = 0;
        stages[index].transitionSamplesLeft = 0;
        stages[index].transitionSamplesTotal = 0;
        stages[index].lfoPhase = 0.0f; stages[index].lfoTick = -1;
        std::memset(stages[index].flangeBuffer, 0, sizeof(stages[index].flangeBuffer));
        stages[index].flangeWriteIndex = 0;
        stages[index].pmPhase = 0.0f;
        stages[index].crushPhase = 0.0f; stages[index].crushedWet = 0.0f;
        stages[index].popTimer = 0.0f;
        stages[index].stutPos = stages[index].stutStart = stages[index].stutLen = stages[index].stutTimer = 0;
        stages[index].crossFb = 0.0f;
        stages[index].allpassFb = 0.0f;
        stages[index].glottalPhase = 0.0f; stages[index].glottalPeriod = 0.0f;
        for (auto& pr : stages[index].partials) pr = PartialResonator{};
        stages[index].cachedPartFreq = -1.0f; stages[index].cachedPartAmt = -1.0f;
        stages[index].cachedPartRate = -1.0f; stages[index].cachedPartSr = -1.0f;
        stages[index].cachedPartRelPos = -1.0f; stages[index].cachedPartFreqDamp = -1.0f;
        stages[index].cachedPartStiffness = -1.0f; stages[index].cachedPartStiffExp = -1.0f; stages[index].cachedPartCount = -1.0f;
    }

    void setModeRate(int index, float normalized) {
        if (index < 0 || index >= 40) return;
        // 0.05..8 Hz log; per-stage detune (0..8%) keeps 40 LFOs from locking.
        const float hz = 0.05f * std::pow(160.0f, std::clamp(normalized, 0.0f, 1.0f));
        stages[index].lfoRate = hz * (1.0f + 0.02f * (float)(index % 5));
        stages[index].modeRate = std::clamp(normalized, 0.0f, 1.0f); // normalized, for partials inharmonicity
    }

    void setModeAmount(int index, float amount) {
        if (index < 0 || index >= 40) return;
        stages[index].modeAmount = std::clamp(amount, 0.0f, 1.0f);
    }

    // Per-filter-type extra controls (see FilterType). A/B are normalized;
    // each branch below maps them to the range that type actually needs.
    void setModeA(int index, float normalized) {
        if (index < 0 || index >= 40) return;
        stages[index].modeA = std::clamp(normalized, 0.0f, 1.0f);
    }
    void setModeB(int index, float normalized) {
        if (index < 0 || index >= 40) return;
        stages[index].modeB = std::clamp(normalized, 0.0f, 1.0f);
    }

    // Saike Partials model controls (only meaningful for FT_PARTIAL_* types).
    // Setters take normalized 0..1 like setModeRate and convert internally;
    // the param ranges (0.0001..0.999 etc.) are host-display metadata only.
    void setPartRelPos(int index, float v) {
        if (index < 0 || index >= 40) return;
        stages[index].partRelPos = 0.0001f + std::clamp(v, 0.0f, 1.0f) * 0.9989f; // Saike rel_position 0.0001..0.999
    }

    void setPartFreqDamp(int index, float v) {
        if (index < 0 || index >= 40) return;
        stages[index].partFreqDamp = -6.0f + std::clamp(v, 0.0f, 1.0f) * 5.0f; // Saike freq_damp slider -6..-1
    }

    void setPartStiffness(int index, float v) {
        if (index < 0 || index >= 40) return;
        stages[index].partStiffness = 2.0f + std::clamp(v, 0.0f, 1.0f) * 4.0f; // Saike stiffness 2..6
    }

    void setPartStiffExp(int index, float v) {
        if (index < 0 || index >= 40) return;
        stages[index].partStiffExp = 1.0f + std::clamp(v, 0.0f, 1.0f) * 2.0f; // Saike stiffness_exp 1..3
    }

    void setPartCount(int index, float v) {
        if (index < 0 || index >= 40) return;
        stages[index].partCount = 16 + std::clamp((int)std::lroundf(std::clamp(v, 0.0f, 1.0f) * 48.0f), 0, 48); // Saike partials 16..64
    }

    void setSaturatorAmount(int index, float amount) {
        if (index < 0 || index >= 40) return;
        const float v = std::clamp(amount, 0.0f, 1.0f);
        if (v != stages[index].satAmount) {
            stages[index].satAmount = v;
            stages[index].satUserDirty = true;
        }
    }

    float getSaturatorAmount(int index) const {   // (MED-4) live-value readback
        return (index >= 0 && index < 40) ? stages[index].satAmount : 0.0f;
    }

    void setSatStressOnset(int index, float onset) {
        if (index < 0 || index >= 40) return;
        stages[index].satStressOnset = std::clamp(onset, 0.0f, 0.999f);
        if (stages[index].satStressKnee <= stages[index].satStressOnset + 0.001f)
            stages[index].satStressKnee = std::min(1.0f, stages[index].satStressOnset + 0.001f);
    }

    void setSatStressKnee(int index, float knee) {
        if (index < 0 || index >= 40) return;
        stages[index].satStressKnee = std::clamp(knee, 0.001f, 1.0f);
        if (stages[index].satStressKnee <= stages[index].satStressOnset + 0.001f)
            stages[index].satStressKnee = std::min(1.0f, stages[index].satStressOnset + 0.001f);
    }

    void setStageMix(int index, float mix) {
        if (index < 0 || index >= 40) return;
        stages[index].mix = std::clamp(mix, 0.0f, 1.0f);
    }

    void setStageSolo(int index, bool solo) {
        if (index < 0 || index >= 40) return;
        stages[index].solo = solo;
    }

    void setStageEnabled(int index, bool en) {
        if (index < 0 || index >= kMaxGroups * 4) return;
        if (!en && stages[index].enabled) {
            std::memset(stages[index].buffer, 0, sizeof(stages[index].buffer));
            stages[index].writeIndex = 0;
            stages[index].lpfFilter.clear();
            stages[index].hpfFilter.clear();
            stages[index].cachedSampleRate = -1.0f;
            stages[index].cachedLpfFreq = -1.0f;
            stages[index].cachedHpfFreq = -1.0f;
            stages[index].biquad.clear();
            for (auto& ps : stages[index].phaserStages) ps.clear();
            for (auto& fs : stages[index].formantStages) fs.clear();
            stages[index].cachedBqFreq = -1.0f;
            stages[index].cachedBqFeedback = -1.0f;
            stages[index].cachedBqSr = -1.0f;
            for (int s = 0; s < 4; ++s) stages[index].miniDispState[s] = 0.0f;
            stages[index].pendingSize.store(0);
            stages[index].transitionOldSize = 0;
            stages[index].transitionOldWriteIndex = 0;
            stages[index].transitionSamplesLeft = 0;
            stages[index].transitionSamplesTotal = 0;
            stages[index].lfoPhase = 0.0f; stages[index].lfoTick = -1;
            std::memset(stages[index].flangeBuffer, 0, sizeof(stages[index].flangeBuffer));
            stages[index].flangeWriteIndex = 0;
            stages[index].pmPhase = 0.0f;
            stages[index].crushPhase = 0.0f; stages[index].crushedWet = 0.0f;
            stages[index].popTimer = 0.0f;
            stages[index].stutPos = stages[index].stutStart = stages[index].stutLen = stages[index].stutTimer = 0;
            stages[index].crossFb = 0.0f;
            stages[index].allpassFb = 0.0f;
            stages[index].glottalPhase = 0.0f; stages[index].glottalPeriod = 0.0f;
            stages[index].solo = false;
        }
        stages[index].enabled = en;
    }

    void setGroupGain(int groupIdx, float gain) {
        if (groupIdx >= 0 && groupIdx < kMaxGroups) groups[groupIdx].gain = gain;
    }

    // ---- Per-group peak limiter -------------------------------------------
    // All three take the normalized 0..1 control the host/UI reports and
    // convert to linear/seconds here, on the message thread, so the audio
    // thread never evaluates a pow()/exp(). Threshold 0..1 -> -24..0 dB,
    // Release 0..1 -> 5 ms..1 s (exponential one-pole toward unity gain).
    // Mirrors setGroupGain's addressing: groupIdx 0..9 == groups 1..10.
    void setGroupLimiterOn(int groupIdx, float v) {
        if (groupIdx < 0 || groupIdx >= 10) return;
        groups[groupIdx].limiterOn = (v > 0.5f);
    }

    void setGroupLimiterThreshold(int groupIdx, float v) {
        if (groupIdx < 0 || groupIdx >= 10) return;
        const float db = -24.0f + std::clamp(v, 0.0f, 1.0f) * 24.0f;
        groups[groupIdx].limiterThresholdLin = std::pow(10.0f, db / 20.0f);
    }

    void setGroupLimiterRelease(int groupIdx, float v) {
        if (groupIdx < 0 || groupIdx >= 10) return;
        groups[groupIdx].limiterReleaseSec = 0.005f + std::clamp(v, 0.0f, 1.0f) * 0.995f;
        const float sr = std::max(1.0f, sampleRate);
        groups[groupIdx].limiterReleaseCoeff =
            std::exp(-1.0f / (groups[groupIdx].limiterReleaseSec * sr));
    }

    // RT-safe: reads only the pre-converted threshold/coefficient, writes one
    // float. No allocation, no lock, no function pointer. Only reached when
    // limiterOn is true, so an off limiter cannot move limiterGain.
    static inline float applyGroupLimiter(GroupBus& g, float x) {
        const float peak = std::fabs(x);
        float target = 1.0f;
        if (peak > g.limiterThresholdLin) target = g.limiterThresholdLin / peak;
        if (target < g.limiterGain) {
            g.limiterGain = target; // instant attack: no overshoot past the ceiling
        } else {
            const float c = g.limiterReleaseCoeff;
            g.limiterGain = c * g.limiterGain + (1.0f - c); // exponential recovery to 1.0
        }
        return x * g.limiterGain;
    }

    void setSeries(int bankIdx, bool series) {
        if (bankIdx >= 0 && bankIdx < kMaxGroups) groups[bankIdx].isSeries = series;
    }

    void setShift(int groupIdx, float hz) {
        if (groupIdx >= 0 && groupIdx < kMaxGroups) groups[groupIdx].shiftFreq = hz;
    }

    void setCluster(int groupIdx, float amount) {
        if (groupIdx < 0 || groupIdx >= 10) return;
        clusterAmounts[groupIdx] = std::clamp(amount, 0.0f, 1.0f);
        for (int s = 0; s < 4; ++s)
            applyScaledFrequency(groupIdx * 4 + s);
    }

    void setResonatorDiffusion(float amount) {
        resonatorDiffusion = std::clamp(amount, 0.0f, 1.0f);
    }

    void setMemoryDamping(float amount) {
        memoryDamping = std::clamp(amount, 0.0f, 1.0f);
    }

    void setActiveGroupCount(int count) {
        activeGroupCount = std::clamp(count, 1, kMaxGroups);
    }

    bool hasEnabledStages() const {
        // A group toggled off sets its GAIN to zero; the stages stay enabled
        // so that their state survives. Audibility therefore requires both
        // the stage and its group gain to be live.
        for (int g = 0; g < activeGroupCount; ++g) {
            if (groups[g].gain <= 1e-6f) continue;
            for (int s = 0; s < 4; ++s) {
                const int i = g * 4 + s;
                if (stages[i].enabled && stages[i].mix > 0.0005f) return true;
            }
        }
        return false;
    }

    void process(float inL, float inR, float& outL, float& outR) {
        outL = 0.0f; outR = 0.0f;
        bool anySolo = false;
        for (int i = 0; i < 40; ++i) {
            if (stages[i].solo && stages[i].enabled) { anySolo = true; break; }
        }
        const float midIn = 0.5f * (inL + inR);
        float drainFactor = 1.0f;
        if (m_drainSamplesLeft > 0) {
            drainFactor = (float)m_drainSamplesLeft / (float)m_drainSamplesTotal;
            --m_drainSamplesLeft;
        }
        for (int g = 0; g < activeGroupCount; ++g) {
            float groupSum = 0.0f;
            bool active = false;
            const float groupPos = ((float)g - 4.5f) / 4.5f;
            const float pan = std::clamp(groupPos * 0.35f, -1.0f, 1.0f);
            const float panL = std::sqrt(0.5f * (1.0f - pan));
            const float panR = std::sqrt(0.5f * (1.0f + pan));
            float input = midIn;
            float currentInput = input;
            for (int s = 0; s < 4; ++s) {
                int idx = g * 4 + s;
                // Skip disabled stages entirely — no processing, no series feed-through
                if (!stages[idx].enabled) continue;
                {
                    float out = stages[idx].process(currentInput, sampleRate, resonatorDiffusion, memoryDamping, drainFactor);
                    const float stageMix = std::clamp(stages[idx].mix, 0.0f, 1.0f);
                    const bool stageAudible = (stageMix > 0.0005f) && (!anySolo || stages[idx].solo);
                    const float stageOut = stageAudible ? (out * stageMix) : 0.0f;
                    if (groups[g].isSeries) {
                        currentInput += out; // accumulate resonance into signal chain
                        if (stageAudible) {
                            groupSum += stageOut;
                            active = true;
                        }
                    } else {
                        if (stageAudible) {
                            groupSum += stageOut;
                            active = true;
                        }
                    }
                }
            }
            if (active) {
                float filtered = groupSum;
                filtered = groups[g].shifter.process(filtered, groups[g].shiftFreq, sampleRate);
                const float shiftAmt = std::clamp(std::abs(groups[g].shiftFreq) / 10000.0f, 0.0f, 1.0f);
                // Make high shift settings more audible: suppress residual unshifted bleed
                // from Hilbert approximation and slightly emphasize the shifted sideband.
                filtered = (filtered - groupSum * (0.45f * shiftAmt)) * (1.0f + 0.90f * shiftAmt);
                filtered *= groups[g].gain;
                // Per-group limiter sits after the group gain, on the group's
                // summed output. Zero-cost when off: the branch is the only
                // thing that runs and no limiter state is touched.
                if (groups[g].limiterOn) filtered = applyGroupLimiter(groups[g], filtered);
                outL += filtered * panL;
                outR += filtered * panR;
            }
        }

        if (!std::isfinite(outL)) outL = 0.0f; else if (std::abs(outL) < 1e-7f) outL = 0.0f;
        if (!std::isfinite(outR)) outR = 0.0f; else if (std::abs(outR) < 1e-7f) outR = 0.0f;
    }

    void processStereo(float& inL, float& inR) {
        float dryL = inL, dryR = inR;
        float outL = 0.0f, outR = 0.0f;
        bool anySolo = false;
        for (int i = 0; i < 40; ++i) {
            if (stages[i].solo && stages[i].enabled) { anySolo = true; break; }
        }
        const float midIn = 0.5f * (inL + inR);
        float drainFactor = 1.0f;
        if (m_drainSamplesLeft > 0) {
            drainFactor = (float)m_drainSamplesLeft / (float)m_drainSamplesTotal;
            --m_drainSamplesLeft;
        }
        for (int g = 0; g < activeGroupCount; ++g) {
            float groupSum = 0.0f;
            bool active = false;
            const float groupPos = ((float)g - 4.5f) / 4.5f;
            const float pan = std::clamp(groupPos * 0.35f, -1.0f, 1.0f);
            const float panL = std::sqrt(0.5f * (1.0f - pan));
            const float panR = std::sqrt(0.5f * (1.0f + pan));
            float input = midIn;
            float currentInput = input;
            for (int s = 0; s < 4; ++s) {
                int idx = g * 4 + s;
                // Skip disabled stages entirely — no processing, no series feed-through
                if (!stages[idx].enabled) continue;
                {
                    float out = stages[idx].process(currentInput, sampleRate, resonatorDiffusion, memoryDamping, drainFactor);
                    const float stageMix = std::clamp(stages[idx].mix, 0.0f, 1.0f);
                    const bool stageAudible = (stageMix > 0.0005f) && (!anySolo || stages[idx].solo);
                    const float stageOut = stageAudible ? (out * stageMix) : 0.0f;
                    if (groups[g].isSeries) {
                        currentInput += out; // accumulate resonance into signal chain
                        if (stageAudible) {
                            groupSum += stageOut;
                            active = true;
                        }
                    } else {
                        if (stageAudible) {
                            groupSum += stageOut;
                            active = true;
                        }
                    }
                }
            }
            if (active) {
                float filtered = groupSum;
                filtered = groups[g].shifter.process(filtered, groups[g].shiftFreq, sampleRate);
                const float shiftAmt = std::clamp(std::abs(groups[g].shiftFreq) / 10000.0f, 0.0f, 1.0f);
                // Mirror the same shift emphasis used in process() for consistent behavior.
                filtered = (filtered - groupSum * (0.45f * shiftAmt)) * (1.0f + 0.90f * shiftAmt);
                filtered *= groups[g].gain;
                if (groups[g].limiterOn) filtered = applyGroupLimiter(groups[g], filtered);
                outL += filtered * panL;
                outR += filtered * panR;
            }
        }

        if (!std::isfinite(outL)) outL = 0.0f; else if (std::abs(outL) < 1e-7f) outL = 0.0f;
        if (!std::isfinite(outR)) outR = 0.0f; else if (std::abs(outR) < 1e-7f) outR = 0.0f;
        inL = dryL + outL; inR = dryR + outR;
    }

    void reset() {
        for (int i = 0; i < 40; ++i) {
            std::memset(stages[i].buffer, 0, sizeof(stages[i].buffer));
            stages[i].writeIndex = 0;
            stages[i].lpfFilter.clear();
            stages[i].hpfFilter.clear();
            stages[i].cachedSampleRate = -1.0f;
            stages[i].cachedLpfFreq = -1.0f;
            stages[i].cachedHpfFreq = -1.0f;
            stages[i].biquad.clear();
            for (auto& ps : stages[i].phaserStages) ps.clear();
            for (auto& fs : stages[i].formantStages) fs.clear();
            stages[i].cachedBqFreq = -1.0f;
            stages[i].cachedBqFeedback = -1.0f;
            stages[i].cachedBqSr = -1.0f;
            for (int s = 0; s < 4; ++s) stages[i].miniDispState[s] = 0.0f;
            stages[i].transitionOldSize = 0;
            stages[i].transitionOldWriteIndex = 0;
            stages[i].transitionSamplesLeft = 0;
            stages[i].transitionSamplesTotal = 0;
            stages[i].lfoPhase = 0.0f; stages[i].lfoTick = -1;
            std::memset(stages[i].flangeBuffer, 0, sizeof(stages[i].flangeBuffer));
            stages[i].flangeWriteIndex = 0;
            stages[i].pmPhase = 0.0f;
            stages[i].crushPhase = 0.0f; stages[i].crushedWet = 0.0f;
            stages[i].popTimer = 0.0f;
            stages[i].stutPos = stages[i].stutStart = stages[i].stutLen = stages[i].stutTimer = 0;
            stages[i].crossFb = 0.0f;
            stages[i].allpassFb = 0.0f;
            stages[i].glottalPhase = 0.0f; stages[i].glottalPeriod = 0.0f;
            for (auto& pr : stages[i].partials) pr = PartialResonator{};
            stages[i].cachedPartFreq = -1.0f; stages[i].cachedPartAmt = -1.0f;
            stages[i].cachedPartRate = -1.0f; stages[i].cachedPartSr = -1.0f;
            stages[i].cachedPartRelPos = -1.0f; stages[i].cachedPartFreqDamp = -1.0f;
            stages[i].cachedPartStiffness = -1.0f; stages[i].cachedPartStiffExp = -1.0f; stages[i].cachedPartCount = -1.0f;
        }
        for (int i = 0; i < kMaxGroups; ++i) {
            groups[i].lpf.clear();
            groups[i].hpf.clear();
            groups[i].shifter.reset();
            groups[i].limiterGain = 1.0f;
        }
    }

    int getNumStages() const { return 40; }

#ifdef HOST_BINARY
    float testGetGroupShift(int groupIdx) const {
        return (groupIdx >= 0 && groupIdx < kMaxGroups) ? groups[groupIdx].shiftFreq : 0.0f;
    }
#endif

private:
    static constexpr float kFreqMin = 10.0f;
    static constexpr float kFreqMax = 20000.0f;

    void applyScaledFrequency(int index) {
        if (index < 0 || index >= 40) return;
        int g = index / 4;
        float freq = baseFrequencies[index];

        // Cluster: pull toward geometric center of group in log space
        float cluster = clusterAmounts[g];
        if (cluster > 0.001f) {
            float logCenter = 0.0f;
            for (int s = 0; s < 4; ++s)
                logCenter += std::log2(std::max(kFreqMin, baseFrequencies[g * 4 + s]));
            logCenter *= 0.25f;
            float logFreq = std::log2(std::max(kFreqMin, freq));
            freq = std::pow(2.0f, logFreq + cluster * (logCenter - logFreq));
        }

        float scaled = freq * scaleFactor * stageScaleFactors[index];
        scaled = std::max(kFreqMin, std::min(kFreqMax, scaled));
        stages[index].tunedFreq = scaled; // drives the filter-type mode tunings
        int samples = (int)(sampleRate / std::max(kFreqMin, scaled));
        samples = std::max(1, samples);
        
        if (samples != stages[index].bufferSize) {
            stages[index].pendingSize.store(samples);
        }
    }

    float sampleRate;
    float scaleFactor = 1.0f;
    float stageScaleFactors[40];
    float baseFrequencies[40];
    float clusterAmounts[10];
    float resonatorDiffusion = 0.0f;
    float memoryDamping = 0.0f;
    int m_drainSamplesLeft = 0;
    int m_drainSamplesTotal = 1;
    CombStage stages[40];
    GroupBus groups[10];
};

#endif
