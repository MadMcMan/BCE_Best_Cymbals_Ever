#ifndef LIMITER_HPP
#define LIMITER_HPP

#include <cmath>
#include <algorithm>

// Simple lookahead-free peak limiter with attack/release
class Limiter {
public:
    Limiter(float sampleRate = 44100.0f)
        : sampleRate(sampleRate),
          threshold(0.95f),
          attackCoeff(0.0f),
          releaseCoeff(0.0f),
          envelope(0.0f)
    {
        setAttack(0.001f);   // 1ms attack
        setRelease(0.050f);  // 50ms release
    }
    
    void setSampleRate(float sr) {
        sampleRate = sr;
        // Recalculate coefficients
        setAttack(attackTime);
        setRelease(releaseTime);
    }
    
    void setThreshold(float t) {
        threshold = std::max(0.01f, std::min(1.0f, t));
    }
    
    void setAttack(float seconds) {
        attackTime = std::max(0.0001f, seconds);
        attackCoeff = std::exp(-1.0f / (attackTime * sampleRate));
    }
    
    void setRelease(float seconds) {
        releaseTime = std::max(0.001f, seconds);
        releaseCoeff = std::exp(-1.0f / (releaseTime * sampleRate));
    }
    
    // Process a stereo pair in-place
    void process(float& left, float& right) {
        // Detect peak level
        float peak = std::max(std::abs(left), std::abs(right));
        
        // Envelope follower (fast attack, slow release)
        if (peak > envelope) {
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * peak;
        } else {
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * peak;
        }
        
        // Calculate gain reduction
        float gain = 1.0f;
        if (envelope > threshold) {
            gain = threshold / envelope;
        }
        
        left *= gain;
        right *= gain;
    }
    
    void reset() {
        envelope = 0.0f;
    }
    
private:
    float sampleRate;
    float threshold;
    float attackTime = 0.001f;
    float releaseTime = 0.050f;
    float attackCoeff;
    float releaseCoeff;
    float envelope;
};

#endif
