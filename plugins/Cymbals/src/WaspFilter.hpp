#pragma once

class WaspFilter {
public:
    enum class Mode {
        LowPass = 0,
        BandPass,
        HighPass,
    };

    struct Outputs {
        float low;
        float band;
        float high;
        float selected;
    };

    explicit WaspFilter(float sampleRate = 48000.0f) noexcept;

    void setSampleRate(float sampleRate) noexcept;
    void setCutoffHz(float cutoffHz) noexcept;
    void setResonance(float resonance) noexcept;
    void setDrive(float drive) noexcept;
    void setAsymmetry(float asymmetry) noexcept;
    void setMode(Mode mode) noexcept;
    void setNumStages(int numStages) noexcept;

    float getSampleRate() const noexcept { return m_sampleRate; }
    float getCutoffHz() const noexcept { return m_cutoffHz; }
    float getResonance() const noexcept { return m_resonance; }
    float getDrive() const noexcept { return m_drive; }
    float getAsymmetry() const noexcept { return m_asymmetry; }
    Mode getMode() const noexcept { return m_mode; }
    int getNumStages() const noexcept { return m_numStages; }

    void reset() noexcept;

    float processSample(float input) noexcept;
    Outputs processAll(float input) noexcept;

    float getLowPass() const noexcept { return m_lastLow; }
    float getBandPass() const noexcept { return m_lastBand; }
    float getHighPass() const noexcept { return m_lastHigh; }

private:
    // Multi-stage cascade support
    struct Stage {
        float ic1eq = 0.0f;
        float ic2eq = 0.0f;
        float feedbackState = 0.0f;
    };

    void updateCoefficients() noexcept;
    float shape(float x) const noexcept;
    static float sanitize(float x) noexcept;
    
    // Process a single sample through a specific stage's SVF
    // Returns the low-pass output of that stage only
    float processSingleStageLP(float input, Stage& stage) noexcept;

    float m_sampleRate;
    float m_cutoffHz;
    float m_resonance;
    float m_drive;
    float m_asymmetry;
    Mode m_mode;
    int m_numStages;

    float m_g;
    float m_a1;
    float m_a2;
    float m_a3;
    float m_k;
    float m_driveGain;

    // Stage array for cascading (max 4 stages = 8 poles)
    static constexpr int MAX_STAGES = 4;
    Stage m_stages[MAX_STAGES];

    float m_lastLow;
    float m_lastBand;
    float m_lastHigh;
};
