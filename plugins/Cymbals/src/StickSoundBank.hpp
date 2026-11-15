#ifndef STICK_SOUND_BANK_HPP
#define STICK_SOUND_BANK_HPP

#include <cmath>
#include <algorithm>
#include <array>
#include <cstdint>
#include "AdsrEnvelope.hpp"
#include "MsegEnvelope.hpp"
#include "DSPUtils.hpp"

using namespace DSPUtils;

static inline float xorshift32Unit(uint32_t& state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return (float)(state & 0x7FFFFFFF) / (float)0x7FFFFFFF;
}

struct MetallicPartialBank {
    struct Partial {
        float phase;
        float freqRatio;
        float amplitude;
    };
    std::array<Partial, 12> partials;
    float sampleRate = 44100.0f;
    float spread = 0.5f; // 0.5 = legacy ratios; 0..1 scales inharmonicity
    uint32_t m_rngState = 0;

    MetallicPartialBank() {
        m_rngState = 2463534242u ^ (uint32_t)(uintptr_t)this;
        float ratios[12] = {1.0f, 1.414f, 1.732f, 2.15f, 2.67f, 3.12f, 3.89f, 4.35f, 5.12f, 6.23f, 7.89f, 9.12f};
        for (int i = 0; i < 12; ++i) {
            partials[i].phase = xorshift32Unit(m_rngState);
            partials[i].freqRatio = ratios[i];
            partials[i].amplitude = 1.0f / (1.0f + (float)i * 0.5f);
        }
    }

    float process(float baseFreq, float amp) {
        float out = 0;
        const float spreadScale = 0.5f + std::clamp(spread, 0.0f, 1.0f);
        for (auto& p : partials) {
            const float ratio = 1.0f + (p.freqRatio - 1.0f) * spreadScale;
            p.phase += ratio * baseFreq / sampleRate;
            if (p.phase >= 1.0f) p.phase -= 1.0f;
            out += std::sin(p.phase * 2.0f * (float)M_PI) * p.amplitude;
        }
        return out * amp * 0.15f;
    }
};

struct NoiseGenerator {
    uint32_t m_rngState = 2463534242u;
    float color = 0.0f; // 0 = white (legacy), 1 = brown
    float lp = 0.0f;

    NoiseGenerator() {
        m_rngState ^= (uint32_t)(uintptr_t)this;
    }
    void setColor(float c) { color = std::clamp(c, 0.0f, 1.0f); }

    float process() {
        const float white = xorshift32Unit(m_rngState) * 2.0f - 1.0f;
        if (color <= 0.001f) return white;
        const float coeff = 0.02f + 0.18f * color;
        lp += (white - lp) * coeff;
        const float brown = lp * (0.35f / std::sqrt(coeff));
        const float c = color * 0.75f;
        return white * (1.0f - c) + brown * c;
    }
};

struct SimpleOscillator {
    float phase = 0;
    float sampleRate = 44100.0f;
    float pulseWidth = 0.25f;
    float squareWidth = 0.5f;
    float sawCurve = 0.5f;
    float sawCurveExp = 1.0f;
    float triangleSkew = 0.5f;
    float sineDrive = 0.0f;
    float phaseOffset = 0.0f;
    // type matches StickSoundBank::StickType: 0 SINE, 1 TRIANGLE, 2 SAW, 3 SQUARE, 4 PULSE
    float process(float freq, int type) {
        phase += freq / sampleRate;
        if (phase >= 1.0f) phase -= 1.0f;
        float p = phase + phaseOffset;
        if (p >= 1.0f) p -= 1.0f;
        if (p < 0.0f) p += 1.0f;
        switch (type) {
            case 1: { // TRIANGLE: variable peak position; 0.5 = legacy
                const float sk = std::clamp(triangleSkew, 0.05f, 0.95f);
                return (p < sk) ? (-1.0f + 2.0f * p / sk)
                                : (1.0f - 2.0f * (p - sk) / (1.0f - sk));
            }
            case 2: // SAW: curve exponent; 1.0 = legacy linear ramp
                return 2.0f * std::pow(p, sawCurveExp) - 1.0f;
            case 3: { // SQUARE: variable width; 0.5 = legacy 50%
                const float d = std::clamp(squareWidth, 0.05f, 0.95f);
                float v = (p < d) ? 1.0f : -1.0f;
                v -= 2.0f * d - 1.0f;
                return v / (2.0f * std::max(d, 1.0f - d));
            }
            case 4: { // PULSE, variable duty, DC removed, peak-normalized
                const float d = std::clamp(pulseWidth, 0.05f, 0.95f);
                float v = (p < d) ? 1.0f : -1.0f;
                v -= 2.0f * d - 1.0f;
                return v / (2.0f * std::max(d, 1.0f - d));
            }
            default: { // SINE + optional drive/fold
                float v = std::sin(p * 2.0f * (float)M_PI);
                if (sineDrive > 0.0f)
                    v += sineDrive * (std::tanh(v * 6.0f) - v);
                return v;
            }
        }
    }
};

class StickSoundBank {
public:
    enum StickType { SINE, TRIANGLE, SAW, SQUARE, PULSE, NOISE, METALLIC, NOISE_METAL, HI_HAT, SILENT };
    static constexpr int kMaxUnison = 4;

    StickSoundBank(float sampleRate = 44100.0f) : sampleRate(sampleRate) {
        ampEnv.setAttack(0.001f); ampEnv.setDecay(0.01f); ampEnv.setSustain(0.0f); ampEnv.setRelease(0.01f);
        pitchEnv.setAttack(0.001f); pitchEnv.setDecay(0.1f); pitchEnv.setSustain(0.0f); pitchEnv.setRelease(0.1f);
        baseFreq = 220.0f; velocity = 0.0f; pitchAmount = 1.0f; pitchTimer = 0.05f; timerPhase = 0.0f;
        useMsegPitch = false;
        m_sizzleRngState = 2463534242u ^ (uint32_t)(uintptr_t)this;
        // Propagate sample rate to sub-oscillators immediately
        ampEnv.setSampleRate(sampleRate);
        pitchEnv.setSampleRate(sampleRate);
        for (int u = 0; u < kMaxUnison; ++u) {
            osc[u].sampleRate = sampleRate;
            // Tiny phase spread only: a quarter-cycle offset made a 4-voice
            // sine cancel at detune 0. 0.01 cycle stays coherent but avoids an
            // exact stack.
            osc[u].phase = 0.01f * (float)u;
        }
        metallicBank.sampleRate = sampleRate;
        updateUnisonRatios();
    }

    void setSampleRate(float sr) {
        sampleRate = sr;
        ampEnv.setSampleRate(sr);
        pitchEnv.setSampleRate(sr);
        for (auto& o : osc) o.sampleRate = sr;
        metallicBank.sampleRate = sr;
    }
    void setAmpAttack(float v) { ampEnv.setAttack(mapTimeExp(v, 0.0003f, 0.060f)); }
    void setAmpDecay(float v) { ampEnv.setDecay(mapTimeExp(v, 0.0030f, 0.900f)); }
    void setAmpSustain(float v) { ampEnv.setSustain(v); }
    void setAmpDecayCurve(float norm) { ampEnv.setDecayCurve(std::clamp(norm, 0.0f, 1.0f) * 2.0f - 1.0f); }
    void setAmpReleaseCurve(float norm) { ampEnv.setReleaseCurve(std::clamp(norm, 0.0f, 1.0f) * 2.0f - 1.0f); }
    void setAmpRelease(float v) {
        ampRelNorm = std::clamp(v, 0.0f, 1.0f);
        ampEnv.setRelease(mapTimeExp(ampRelNorm, 0.0040f, 1.200f) * ampRelScale);
    }
    // Release-velocity support: scales ONLY the note-off-initiated amp release
    // time (multiplier >= 1). setAmpRelease() re-applies it over future param writes.
    void setAmpReleaseScale(float mul) {
        ampRelScale = std::clamp(mul, 1.0f, 8.0f);
        setAmpRelease(ampRelNorm);
    }
    bool isActive() const { return ampEnv.isActive(); }
    // MPE per-note pitch bend: semitones -> multiplicative factor, folded into
    // the per-sample pitch computation (pow paid once per MIDI event, not per
    // sample). 0 semitones => exactly 1.0f => bit-identical legacy math.
    void setPitchBendSemitones(float semis) { pitchBendMul = std::pow(2.0f, semis / 12.0f); }
    // Inverse of setPitchBendSemitones — returns the ACTUAL applied semitone
    // offset, so tests can assert bend+slide combined (not just the stored
    // per-source fields that each write independently).
    float getPitchBendSemitones() const { return 12.0f * std::log2f(pitchBendMul); }
    void setPitchAttack(float v) { pitchEnv.setAttack(mapTimeExp(v, 0.0002f, 0.050f)); }
    void setPitchDecay(float v) { pitchEnv.setDecay(mapTimeExp(v, 0.0050f, 1.500f)); }
    void setPitchSustain(float v) { pitchEnv.setSustain(v); }
    void setPitchRelease(float v) { pitchEnv.setRelease(mapTimeExp(v, 0.0050f, 1.500f)); }
    void setPitchDecayCurve(float norm) { pitchEnv.setDecayCurve(std::clamp(norm, 0.0f, 1.0f) * 2.0f - 1.0f); }
    void setPitchReleaseCurve(float norm) { pitchEnv.setReleaseCurve(std::clamp(norm, 0.0f, 1.0f) * 2.0f - 1.0f); }
    void setPitchAmount(float v) { pitchAmount = v; }
    void setPitchTimer(float v) { pitchTimer = v; }
    void setCurrentStick(StickType t) { currentStickType = t; }
    // Per-type character controls (IMPULSES header).
    void setPulseWidth(float v) { for (auto& o : osc) o.pulseWidth = std::clamp(v, 0.05f, 0.95f); }
    void setNoiseColor(float v) { noiseGen.setColor(v); }
    void setMetallicSpread(float v) { metallicBank.spread = std::clamp(v, 0.0f, 1.0f); }
    void setNoiseMetalBlend(float v) { noiseMetalBlend = std::clamp(v, 0.0f, 1.0f); }
    void setHatTone(float v) { hatTone = std::clamp(v, 0.0f, 1.0f); }
    void setSquareWidth(float v) { for (auto& o : osc) o.squareWidth = std::clamp(v, 0.0f, 1.0f); }
    void setSawCurve(float v) {
        const float c = std::clamp(v, 0.0f, 1.0f);
        const float e = std::pow(4.0f, c - 0.5f);
        for (auto& o : osc) { o.sawCurve = c; o.sawCurveExp = e; }
    }
    void setTriangleSkew(float v) { for (auto& o : osc) o.triangleSkew = std::clamp(v, 0.0f, 1.0f); }
    void setSineDrive(float v) { for (auto& o : osc) o.sineDrive = std::clamp(v, 0.0f, 1.0f); }
    void setPhaseOffset(float v) {
        const float off = std::clamp(v, 0.0f, 1.0f) - 0.5f;
        for (auto& o : osc) o.phaseOffset = off;
    }
    void setGrit(float v) { grit = std::clamp(v, 0.0f, 1.0f); }
    void setUnison(int count) {
        unisonCount = std::clamp(count, 1, kMaxUnison);
        updateUnisonRatios();
    }
    void setDetune(float v) {
        unisonDetune = std::clamp(v, 0.0f, 1.0f);
        updateUnisonRatios();
    }

    void setUseMsegPitch(bool use) { useMsegPitch = use; }
    float getMsegPitchPhase() const { return msegPitch.getPhase(); }
    bool isMsegPitchActive() const { return msegPitch.isActive(); }
    void setPitchMsegDuration(float sec) { msegPitch.setDuration(sec); }
    void setPitchMsegPoints(const std::vector<MsegPoint>& pts) { msegPitch.setPoints(pts); }

    void triggerNote(float freq, float vel) {
        baseFreq = freq; velocity = vel; timerPhase = 0.0f;
        ampEnv.trigger(); pitchEnv.trigger();
        if (useMsegPitch) msegPitch.trigger();
    }
    // Retune a SOUNDING hit without restarting its envelopes or phase. Master
    // Tune uses this so a held note follows the control instead of latching
    // the frequency captured at trigger time.
    void setBaseFrequency(float freq) { baseFreq = std::max(1e-6f, freq); }
    void releaseNote() { ampEnv.releaseNote(); pitchEnv.releaseNote(); }

    void process(float& outL, float& outR) {
        float amp = ampEnv.process() * velocity;
        if (amp <= 0.0f) {
            // Keep the pitch-MSEG clock running while the amp is silent, so the
            // UI playhead finishes its sweep (otherwise it freezes mid-plot).
            if (useMsegPitch) msegPitch.process();
            return;
        }

        float pEnv = 0.0f;
        if (useMsegPitch) {
            pEnv = msegPitch.process();
        } else {
            pEnv = pitchEnv.process();
        }
        // SILENT (SOUND = NONE): keep the amp/pitch clocks advancing for the
        // UI, but emit exact silence so the external audio input is the only
        // excitation source. No oscillator, noise, or sizzle residue.
        if (currentStickType == SILENT) {
            outL = outR = 0.0f;
            return;
        }
        float freq = baseFreq * (1.0f + pEnv * pitchAmount) * pitchBendMul;

        float sig = 0.0f;
        switch (currentStickType) {
            case NOISE:
                sig = noiseGen.process() * amp;
                break;
            case METALLIC:
                sig = metallicBank.process(freq, amp);
                sig += noiseGen.process() * 0.1f * amp;
                break;
            case NOISE_METAL: {
                // The cymbal tutorial's hat source is an oscillator PLUS
                // shaped noise, not either alone. Blend defaults to the legacy
                // 0.55/0.55 balance at 0.5.
                const float m = 0.1f + 0.9f * noiseMetalBlend;
                const float n = 1.0f - 0.9f * noiseMetalBlend;
                sig = (metallicBank.process(freq, amp) * m)
                    + (noiseGen.process() * n * amp);
                break;
            }
            case HI_HAT: {
                // Hi-hats need a bright, differentiated noise edge: the raw
                // white source alone measured ~8 kHz centroid against a
                // ~16 kHz real closed hat. The first difference is a cheap
                // one-zero high-pass and pushes the noise edge upward without
                // changing the shared comb/filter path.
                const float n = noiseGen.process();
                const float edge = n - m_hatPrev;
                m_hatPrev = n;
                // Keep the bright differentiated edge, but retain raw noise:
                // real hat spectra are broad down into the low hundreds of Hz,
                // while the pure differentiator was ~20 dB down there. The
                // extra raw component is what lets the comb/EQ stage match the
                // full spectral envelope instead of only the top octave.
                // Hat tone defaults to the legacy 0.22/0.35/0.22 mix at 0.5.
                const float edgeW = 0.1f + 0.24f * hatTone;
                const float rawW = 0.5f - 0.3f * hatTone;
                const float metalW = 0.1f + 0.24f * hatTone;
                sig = (edge * edgeW + n * rawW + metallicBank.process(freq, amp) * metalW) * amp;
                break;
            }
            default: {
                const int n = std::clamp(unisonCount, 1, kMaxUnison);
                float sum = 0.0f;
                for (int u = 0; u < n; ++u)
                    sum += osc[u].process(freq * unisonRatio[u], (int)currentStickType);
                sig = sum * (1.0f / std::sqrt((float)n)) * amp;
                if (grit > 0.0f) sig += noiseGen.process() * grit * 0.2f * amp;
                break;
            }
        }

        float sizzle = xorshift32Unit(m_sizzleRngState) * 2.0f - 1.0f;
        sig += sizzle * 0.005f * amp;
        outL = outR = sig;
    }

private:
    static float mapTimeExp(float normalized, float minSeconds, float maxSeconds) {
        normalized = std::clamp(normalized, 0.0f, 1.0f);
        const float ratio = maxSeconds / minSeconds;
        return minSeconds * std::pow(ratio, normalized);
    }

    float sampleRate;
    AdsrEnvelope ampEnv, pitchEnv;
    MsegEnvelope msegPitch;
    bool useMsegPitch;
    SimpleOscillator osc[kMaxUnison];
    int unisonCount = 1;
    float unisonDetune = 0.0f;
    float unisonRatio[kMaxUnison] = {1.0f, 1.0f, 1.0f, 1.0f};
    void updateUnisonRatios() {
        const int n = std::clamp(unisonCount, 1, kMaxUnison);
        const float cents = 40.0f * std::clamp(unisonDetune, 0.0f, 1.0f);
        for (int u = 0; u < kMaxUnison; ++u) {
            const float offset = (n <= 1) ? 0.0f
                : (((float)u / (float)(n - 1)) - 0.5f) * 2.0f; // -1..+1
            unisonRatio[u] = std::pow(2.0f, (offset * cents) / 1200.0f);
        }
    }
    MetallicPartialBank metallicBank;
    NoiseGenerator noiseGen;
    uint32_t m_sizzleRngState = 0;
    float m_hatPrev = 0.0f;
    StickType currentStickType = SINE;
    float baseFreq, velocity, pitchAmount, pitchTimer, timerPhase;
    float noiseMetalBlend = 0.5f;
    float hatTone = 0.5f;
    float grit = 0.0f;
    float ampRelNorm = 0.01f;  // last normalized amp-release param (for scale re-apply)
    float pitchBendMul = 1.0f;
    float ampRelScale = 1.0f;  // release-velocity multiplier (1 = legacy)
};

#endif
