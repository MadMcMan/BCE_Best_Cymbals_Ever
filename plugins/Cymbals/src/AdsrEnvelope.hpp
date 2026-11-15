#ifndef ADSR_ENVELOPE_HPP
#define ADSR_ENVELOPE_HPP

#include <algorithm>
#include <cmath>

// ADSR Envelope generator with optional pre-delay, peak hold, and per-segment
// slope curves (curve 0 = exactly linear; -1 = slow start / fast finish;
// +1 = fast start / slow finish). Delay/hold/curves default to off/linear so
// existing users (StickSoundBank amp/pitch envs, DispersionStage chaosEnv,
// bloom/final-out envs) keep their previous behavior.
class AdsrEnvelope {
public:
    enum class State { Idle, Delay, Attack, Hold, Decay, Sustain, Release };

    AdsrEnvelope(float sampleRate = 44100.0f)
        : attack(0.001f), decay(0.05f), sustain(0.0f), release(0.1f),
          sampleRate(sampleRate), state(State::Idle), currentLevel(0.0f), releaseStartLevel(0.0f) {}
    
    void setSampleRate(float sr) { sampleRate = sr; }
    
    void setAttack(float t) { attack = std::max(0.0001f, t); }
    void setDecay(float t) { decay = std::max(0.0001f, t); }
    void setSustain(float l) { sustain = std::clamp(l, 0.0f, 1.0f); }
    void setRelease(float t) { release = std::max(0.0001f, t); }
    void setDelay(float t) { delay = std::max(0.0f, t); }
    void setHold(float t) { hold = std::max(0.0f, t); }
    void setCurves(float a, float d, float r) {
        attackCurve = std::clamp(a, -1.0f, 1.0f);
        decayCurve = std::clamp(d, -1.0f, 1.0f);
        releaseCurve = std::clamp(r, -1.0f, 1.0f);
        attackPow = curveExp(attackCurve);
        decayPow = curveExp(decayCurve);
        releasePow = curveExp(releaseCurve);
    }
    void setDecayCurve(float c) {
        decayCurve = std::clamp(c, -1.0f, 1.0f);
        decayPow = curveExp(decayCurve);
    }
    void setReleaseCurve(float c) {
        releaseCurve = std::clamp(c, -1.0f, 1.0f);
        releasePow = curveExp(releaseCurve);
    }
    
    void trigger() {
        // If a note is retriggered quickly, do not force the envelope back to 0
        // so fast legato/staccato passages do not cut out due to an enforced silence.
        if (state == State::Idle) {
            currentLevel = 0.0f;
        }
        releaseStartLevel = currentLevel;
        segStart = currentLevel;
        segElapsed = 0.0f;
        state = (delay > 0.0f) ? State::Delay : State::Attack;
    }
    
    void releaseNote() {
        if (state != State::Idle) {
            releaseStartLevel = currentLevel;
            segStart = releaseStartLevel;
            segElapsed = 0.0f;
            state = State::Release;
        }
    }
    
    float process() {
        if (state == State::Idle) return 0.0f;
        segElapsed += 1.0f; // samples since the current segment began
        switch (state) {
            case State::Delay: // hold current level until the delay elapses
                if (segElapsed >= delay * sampleRate) {
                    segElapsed = 0.0f;
                    segStart = currentLevel;
                    state = State::Attack;
                }
                break;
            case State::Attack: {
                const float p = std::min(1.0f, segElapsed / (attack * sampleRate));
                currentLevel = (attackPow == 1.0f)
                    ? segStart + (1.0f - segStart) * p
                    : segStart + (1.0f - segStart) * std::pow(p, attackPow);
                if (p >= 1.0f) {
                    currentLevel = 1.0f;
                    segElapsed = 0.0f;
                    segStart = 1.0f;
                    state = (hold > 0.0f) ? State::Hold : State::Decay;
                }
                break;
            }
            case State::Hold: // at peak until the hold elapses
                if (segElapsed >= hold * sampleRate) {
                    segElapsed = 0.0f;
                    segStart = currentLevel;
                    state = State::Decay;
                }
                break;
            case State::Decay: {
                const float p = std::min(1.0f, segElapsed / (decay * sampleRate));
                currentLevel = (decayPow == 1.0f)
                    ? segStart + (sustain - segStart) * p
                    : segStart + (sustain - segStart) * std::pow(p, decayPow);
                if (p >= 1.0f) {
                    currentLevel = sustain;
                    segElapsed = 0.0f; // sustain walk starts at 0 (mod-system phase)
                    state = State::Sustain;
                }
                break;
            }
            case State::Sustain:
                currentLevel = sustain;
                break;
            case State::Release:
                // Release must decay from the captured start level; using
                // sustain here can stall at non-zero.
                {
                    const float p = std::min(1.0f, segElapsed / (release * sampleRate));
                    const float shaped = (releasePow == 1.0f) ? p : std::pow(p, releasePow);
                    currentLevel = releaseStartLevel * (1.0f - shaped);
                    if (p >= 1.0f) {
                        currentLevel = 0.0f;
                        state = State::Idle;
                    }
                }
                break;
            case State::Idle:
                break;
        }
        return currentLevel;
    }
    
    bool isActive() const { return state != State::Idle; }

    // Mod-system phase readers: current segment + samples since that segment began.
    State getState() const { return state; }
    float getSegElapsedSamples() const { return segElapsed; }
    
    float attack, decay, sustain, release;
    float delay = 0.0f, hold = 0.0f;
    float attackCurve = 0.0f, decayCurve = 0.0f, releaseCurve = 0.0f; // -1..1, 0 = linear
    // Cached 2^-curve exponents; recomputed only in the setters, never per sample.
    float attackPow = 1.0f, decayPow = 1.0f, releasePow = 1.0f;

private:
    // e = 2^-curve: curve 0 -> e 1 -> linear; -1 -> e 2 -> slow start; +1 -> e 0.5 -> fast start.
    static float curveExp(float curve) { return std::pow(2.0f, -curve); }
    float sampleRate;
    State state;
    float currentLevel;
    float releaseStartLevel;
    float segStart = 0.0f;
    float segElapsed = 0.0f;
};

#endif // ADSR_ENVELOPE_HPP
