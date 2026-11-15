#ifndef PHASE_DISTORTION_HPP
#define PHASE_DISTORTION_HPP

#include <algorithm>
#include <cmath>

// Phase Distortion — quadrature phase modulator, Kilohearts-compatible controls.
//
// The wet path is a true phase modulation of the input's analytic signal:
//
//     out = I * cos(bias + phi) + Q * sin(bias + phi)
//     phi = DRIVE^2 * K * modulator
//
// I and Q are a quadrature pair produced by two allpass cascades (B leads A by
// about 90 degrees across the audio band). The modulator is the in-phase signal,
// tone-filtered and envelope-normalised, exactly as the reference unit drives
// its phase from the signal itself.
//
// Measured against kHs Phase Distortion (VST2, 48 kHz, sine input):
//  * drive is quadratic: a sine of amplitude A gets a phase index
//    k = 37.76 * drive^2 * A. The rendered harmonics match the captured
//    reference to within ~0.002 at drive 0.25/0.5/0.75/1.0;
//  * Bias is a carrier phase offset of (bias - 0.5) * 360 degrees;
//  * Spread is a per-channel carrier offset of +/- spread * 45 degrees (the
//    two channels differ by 90 degrees at spread = 1);
//  * Tone is a two-pole Butterworth lowpass on the modulator with cutoff
//    20 * 500^tone Hz (20 Hz at 0, 640 Hz at 0.5577, 10 kHz at 1);
//  * Normalize blends the raw modulator toward a 3x-envelope normalised one,
//    which makes the distortion insensitive to input gain (the reference
//    normalises to roughly -9.5 dBFS peak);
//  * Mix is a linear dry/wet crossfade.
//
// This replaces the old input-controlled delay line. A delay line multiplies
// its phase deviation by the input frequency, so its harmonic signature moved
// with pitch; the reference's harmonic ratios are identical from 50 Hz to
// 2 kHz. A quadrature phase modulator is frequency-independent by construction.
class PhaseDistortion {
public:
    PhaseDistortion()
        : drive(0.0f), normalize(0.0f), tone(0.5f), bias(0.5f),
          spread(0.0f), mix(0.0f) {
        setSampleRate(48000.0f);
        reset();
    }

    void setDrive(float v)     { drive     = std::clamp(v, 0.0f, 1.0f); }
    void setNormalize(float v) { normalize = std::clamp(v, 0.0f, 1.0f); }
    void setTone(float v) {
        tone = std::clamp(v, 0.0f, 1.0f);
        updateToneCoeff();
    }
    void setBias(float v)   { bias   = std::clamp(v, 0.0f, 1.0f); }
    void setSpread(float v) { spread = std::clamp(v, 0.0f, 1.0f); }
    void setMix(float v)    { mix    = std::clamp(v, 0.0f, 1.0f); }

    float getDrive()     const { return drive; }
    float getNormalize() const { return normalize; }
    float getTone()      const { return tone; }
    float getBias()      const { return bias; }
    float getSpread()    const { return spread; }
    float getMix()       const { return mix; }

    // Host sample rate. Recomputes the 30 Hz highpass and the tone lowpass; the
    // allpass quadrature network is defined in the z plane and is rate-agnostic.
    void setSampleRate(float sr) {
        if (sr < 1000.0f) sr = 48000.0f;
        sampleRate = sr;
        const float w0 = 2.0f * kPi * 30.0f / sr;
        const float cw = std::cos(w0);
        const float sw = std::sin(w0);
        const float alpha = sw / (2.0f * 0.70710678f);
        const float a0 = 1.0f + alpha;
        hpB0 = (1.0f + cw) * 0.5f / a0;
        hpB1 = -(1.0f + cw) / a0;
        hpB2 = hpB0;
        hpA1 = -2.0f * cw / a0;
        hpA2 = (1.0f - alpha) / a0;
        envRelease = std::exp(-1.0f / (0.05f * sampleRate));
        updateToneCoeff();
    }

    void process(float& sampleL, float& sampleR) {
        // Documented bypass: fully dry and no drive. The analysis chain is
        // skipped for CPU, but the state is cleared on entry to bypass so a
        // later Mix/Drive automation cannot re-inject a stale allpass tail or
        // divide by a frozen envelope.
        if (mix <= 0.001f && drive <= 0.001f) {
            if (!bypassed) { reset(); bypassed = true; }
            return;
        }
        bypassed = false;

        const float dryL = sampleL;
        const float dryR = sampleR;
        const float d = drive * drive;               // measured quadratic law
        const float beta0 = (bias - 0.5f) * 2.0f * kPi;

        const float wetL = processChannel(sampleL, d,
            beta0 - spread * 0.25f * kPi,
            hpZ1L, hpZ2L, axL, ayL, bxL, byL, toneZ1L, toneZ2L, envL);
        const float wetR = processChannel(sampleR, d,
            beta0 + spread * 0.25f * kPi,
            hpZ1R, hpZ2R, axR, ayR, bxR, byR, toneZ1R, toneZ2R, envR);

        sampleL = dryL * (1.0f - mix) + wetL * mix;
        sampleR = dryR * (1.0f - mix) + wetR * mix;
    }

    void reset() {
        hpZ1L = hpZ2L = hpZ1R = hpZ2R = 0.0f;
        for (int i = 0; i < kStages; ++i) {
            axL[i] = ayL[i] = bxL[i] = byL[i] = 0.0f;
            axR[i] = ayR[i] = bxR[i] = byR[i] = 0.0f;
        }
        toneZ1L = toneZ2L = toneZ1R = toneZ2R = 0.0f;
        envL = envR = 0.0f;
    }

private:
    static constexpr int kStages = 10;
    static constexpr float kPi = 3.14159265358979f;
    // Measured drive-to-index gain for a raw (normalize = 0) modulator.
    static constexpr float kDriveGain = 37.76f;

    // Quadrature core (4 sections each, ~90 degrees apart over 50 Hz..10 kHz)
    // followed by a common 6-section shaper applied to BOTH branches. The
    // common shaper sets the base allpass phase without changing the 90-degree
    // A/B relationship; it is what makes bias and spread pure phase rotations.
    static constexpr float kA[kStages] = {
        -0.808220f, -0.976862f,  0.015937f, -0.998280f,
         0.847146f,  0.848326f, -0.308047f, -0.308036f, -0.308047f, -0.999500f
    };
    static constexpr float kB[kStages] = {
         0.650641f, -0.931857f, -0.512713f, -0.992513f,
         0.847146f,  0.848326f, -0.308047f, -0.308036f, -0.308047f, -0.999500f
    };

    float drive, normalize, tone, bias, spread, mix;
    bool bypassed = false;
    float sampleRate = 48000.0f;
    float toneB0 = 0.0f, toneB1 = 0.0f, toneB2 = 0.0f;
    float toneA1 = 0.0f, toneA2 = 0.0f;
    float envRelease = 0.99958f;

    float hpB0 = 0.0f, hpB1 = 0.0f, hpB2 = 0.0f, hpA1 = 0.0f, hpA2 = 0.0f;
    float hpZ1L = 0.0f, hpZ2L = 0.0f, hpZ1R = 0.0f, hpZ2R = 0.0f;

    float axL[kStages]{}, ayL[kStages]{}, bxL[kStages]{}, byL[kStages]{};
    float axR[kStages]{}, ayR[kStages]{}, bxR[kStages]{}, byR[kStages]{};
    float toneZ1L = 0.0f, toneZ2L = 0.0f, toneZ1R = 0.0f, toneZ2R = 0.0f;
    float envL = 0.0f, envR = 0.0f;

    void updateToneCoeff() {
        const float cutoff = 20.0f * std::pow(500.0f, tone); // 20 Hz .. 10 kHz
        const float w0 = 2.0f * kPi * cutoff / sampleRate;
        const float cw = std::cos(w0);
        const float sw = std::sin(w0);
        const float alpha = sw / (2.0f * 0.70710678f);
        const float a0 = 1.0f + alpha;
        toneB0 = (1.0f - cw) * 0.5f / a0;
        toneB1 = (1.0f - cw) / a0;
        toneB2 = toneB0;
        toneA1 = -2.0f * cw / a0;
        toneA2 = (1.0f - alpha) / a0;
    }

    static inline float flushDenorm(float v) {
        return (v > -1e-18f && v < 1e-18f) ? 0.0f : v;
    }

    float processChannel(float x, float d, float beta,
                         float& hpZ1, float& hpZ2,
                         float* ax, float* ay, float* bx, float* by,
                         float& toneZ1, float& toneZ2, float& env) {
        // Shared 30 Hz second-order highpass (reference wet path rolls off
        // below ~30 Hz before the quadrature network).
        float hp = flushDenorm(hpB0 * x + hpZ1);
        hpZ1 = flushDenorm(hpB1 * x - hpA1 * hp + hpZ2);
        hpZ2 = flushDenorm(hpB2 * x - hpA2 * hp);

        // Quadrature allpass pair. B leads A by ~90 degrees over the band.
        float a = hp;
        for (int i = 0; i < kStages; ++i) {
            const float oldX = ax[i];
            const float y = flushDenorm(kA[i] * a + oldX - kA[i] * ay[i]);
            ax[i] = a;
            ay[i] = y;
            a = y;
        }
        float b = hp;
        for (int i = 0; i < kStages; ++i) {
            const float oldX = bx[i];
            const float y = flushDenorm(kB[i] * b + oldX - kB[i] * by[i]);
            bx[i] = b;
            by[i] = y;
            b = y;
        }

        // Modulator: tone-filtered in-phase component, envelope-normalised.
        // Two-pole Butterworth: the reference's 220 Hz attenuation tracks
        // 1/sqrt(1+(f/fc)^4), not a one-pole response.
        float m = flushDenorm(toneB0 * a + toneZ1);
        toneZ1 = flushDenorm(toneB1 * a - toneA1 * m + toneZ2);
        toneZ2 = flushDenorm(toneB2 * a - toneA2 * m);
        const float absM = std::abs(m);
        env = flushDenorm(std::max(absM, env * envRelease));
        const float mNorm = (1.0f - normalize) * m +
                            normalize * (m / (3.0f * env + 1e-4f));

        const float phase = beta + d * kDriveGain * mNorm;
        const float c = std::cos(phase);
        const float s = std::sin(phase);
        return a * c + b * s;
    }
};

#endif
