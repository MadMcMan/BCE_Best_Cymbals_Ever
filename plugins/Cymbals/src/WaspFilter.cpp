#include "WaspFilter.hpp"
#include "DSPUtils.hpp"

#include <algorithm>
#include <cmath>

namespace {
constexpr float kMinSampleRate = 8000.0f;
constexpr float kMaxCutoffNormalized = 0.48f;
constexpr float kDenormalFloor = 1.0e-20f;
constexpr float kHardClamp = 8.0f;
constexpr float kFeedbackGain = 0.5f;

inline float clampf(float v, float lo, float hi) noexcept {
    return std::max(lo, std::min(v, hi));
}

inline float flushDenormal(float v) noexcept {
    return (std::fabs(v) < kDenormalFloor) ? 0.0f : v;
}
} // namespace

WaspFilter::WaspFilter(float sampleRate) noexcept
    : m_sampleRate(sampleRate)
    , m_cutoffHz(1200.0f)
    , m_resonance(0.2f)
    , m_drive(0.0f)
    , m_asymmetry(0.0f)
    , m_mode(Mode::LowPass)
    , m_numStages(1)
    , m_g(0.0f)
    , m_a1(0.0f)
    , m_a2(0.0f)
    , m_a3(0.0f)
    , m_k(0.0f)
    , m_driveGain(1.0f)
    , m_lastLow(0.0f)
    , m_lastBand(0.0f)
    , m_lastHigh(0.0f) {
    // Initialize all stages
    for (int i = 0; i < MAX_STAGES; ++i) {
        m_stages[i].ic1eq = 0.0f;
        m_stages[i].ic2eq = 0.0f;
        m_stages[i].feedbackState = 0.0f;
    }
    updateCoefficients();
}

void WaspFilter::setSampleRate(float sampleRate) noexcept {
    m_sampleRate = std::isfinite(sampleRate) ? sampleRate : 48000.0f;
    if (m_sampleRate < kMinSampleRate) {
        m_sampleRate = kMinSampleRate;
    }
    updateCoefficients();
}

void WaspFilter::setCutoffHz(float cutoffHz) noexcept {
    m_cutoffHz = std::isfinite(cutoffHz) ? cutoffHz : 1200.0f;
    updateCoefficients();
}

void WaspFilter::setResonance(float resonance) noexcept {
    m_resonance = std::isfinite(resonance) ? resonance : 0.0f;
    m_resonance = clampf(m_resonance, 0.0f, 1.0f);
    updateCoefficients();
}

void WaspFilter::setDrive(float drive) noexcept {
    m_drive = std::isfinite(drive) ? drive : 0.0f;
    m_drive = clampf(m_drive, 0.0f, 1.0f);
    updateCoefficients();
}

void WaspFilter::setAsymmetry(float asymmetry) noexcept {
    m_asymmetry = std::isfinite(asymmetry) ? asymmetry : 0.0f;
    m_asymmetry = clampf(m_asymmetry, -1.0f, 1.0f);
}

void WaspFilter::setMode(Mode mode) noexcept {
    m_mode = mode;
}

void WaspFilter::setNumStages(int numStages) noexcept {
    m_numStages = std::clamp(numStages, 1, MAX_STAGES);
}

void WaspFilter::reset() noexcept {
    for (int i = 0; i < MAX_STAGES; ++i) {
        m_stages[i].ic1eq = 0.0f;
        m_stages[i].ic2eq = 0.0f;
        m_stages[i].feedbackState = 0.0f;
    }
    m_lastLow = 0.0f;
    m_lastBand = 0.0f;
    m_lastHigh = 0.0f;
}

float WaspFilter::sanitize(float x) noexcept {
    if (!std::isfinite(x)) {
        return 0.0f;
    }
    x = flushDenormal(x);
    return clampf(x, -kHardClamp, kHardClamp);
}

float WaspFilter::shape(float x) const noexcept {
    const float asym = m_asymmetry;
    const float posScale = 1.0f + 0.9f * asym;
    const float negScale = 1.0f - 0.9f * asym;
    const float sideScale = (x >= 0.0f) ? posScale : negScale;

    // Bias creates asymmetric saturation and stronger even-order/polarity skew.
    const float bias = 0.35f * asym;
    const float shaped = m_driveGain * ((x * sideScale) + bias);
    
    // Authentic CMOS hard clipping instead of smooth tanh
    // This creates hard saturation edges similar to real CMOS inverter behavior
    if (shaped < -1.0f) return -1.0f;
    if (shaped > 1.0f) return 1.0f;
    return shaped;
}

void WaspFilter::updateCoefficients() noexcept {
    if (!std::isfinite(m_sampleRate) || m_sampleRate < kMinSampleRate) {
        m_sampleRate = 48000.0f;
    }

    const float maxCutoff = m_sampleRate * kMaxCutoffNormalized;
    m_cutoffHz = std::isfinite(m_cutoffHz) ? m_cutoffHz : 1200.0f;
    m_cutoffHz = clampf(m_cutoffHz, 20.0f, maxCutoff);

    m_resonance = clampf(std::isfinite(m_resonance) ? m_resonance : 0.0f, 0.0f, 1.0f);
    m_drive = clampf(std::isfinite(m_drive) ? m_drive : 0.0f, 0.0f, 1.0f);

    // Resonance maps to damping term where lower values are more resonant.
    m_k = 2.0f - 1.95f * m_resonance;
    m_k = clampf(m_k, 0.05f, 2.0f);

    m_driveGain = 1.0f + 11.0f * m_drive;

    const float gArg = DSPUtils::kPi * m_cutoffHz / m_sampleRate;
    m_g = std::tan(clampf(gArg, 0.0f, 1.49f));
    m_g = sanitize(m_g);

    const float denom = 1.0f + m_g * (m_g + m_k);
    const float safeDenom = (std::fabs(denom) < 1.0e-12f) ? 1.0f : denom;

    m_a1 = sanitize(1.0f / safeDenom);
    m_a2 = sanitize(m_g * m_a1);
    m_a3 = sanitize(m_g * m_a2);
}

WaspFilter::Outputs WaspFilter::processAll(float input) noexcept {
    if (!std::isfinite(input)) {
        input = 0.0f;
    }

    // Process first stage with feedback from the filter
    Stage& stage0 = m_stages[0];
    stage0.ic1eq = sanitize(stage0.ic1eq);
    stage0.ic2eq = sanitize(stage0.ic2eq);
    stage0.feedbackState = sanitize(stage0.feedbackState);

    const float shapedInput = shape(input);
    const float feedback = clampf(stage0.feedbackState, -1.0f, 1.0f);
    const float x = sanitize(shapedInput - (m_resonance * kFeedbackGain * feedback));

    const float v3 = sanitize(x - stage0.ic2eq);
    const float v1 = sanitize(m_a1 * stage0.ic1eq + m_a2 * v3);
    const float v2 = sanitize(stage0.ic2eq + m_a2 * stage0.ic1eq + m_a3 * v3);

    stage0.ic1eq = sanitize(2.0f * v1 - stage0.ic1eq);
    stage0.ic2eq = sanitize(2.0f * v2 - stage0.ic2eq);

    m_lastLow = v2;
    m_lastBand = v1;
    m_lastHigh = sanitize(x - m_k * v1 - v2);

    stage0.feedbackState = m_lastBand;

    // Process through additional stages (if any), cascading the currently selected mode.
    float cascaded = (m_mode == Mode::HighPass) ? m_lastHigh : ((m_mode == Mode::BandPass) ? m_lastBand : m_lastLow);
    for (int stageIdx = 1; stageIdx < m_numStages; ++stageIdx) {
        Stage& stage = m_stages[stageIdx];
        stage.ic1eq = sanitize(stage.ic1eq);
        stage.ic2eq = sanitize(stage.ic2eq);
        stage.feedbackState = sanitize(stage.feedbackState);

        // For cascaded stages: no extra resonance feedback, only mode-selected signal propagation.
        const float v3c = sanitize(cascaded - stage.ic2eq);
        const float v1c = sanitize(m_a1 * stage.ic1eq + m_a2 * v3c);
        const float v2c = sanitize(stage.ic2eq + m_a2 * stage.ic1eq + m_a3 * v3c);
        const float vHigh = sanitize(cascaded - m_k * v1c - v2c);

        stage.ic1eq = sanitize(2.0f * v1c - stage.ic1eq);
        stage.ic2eq = sanitize(2.0f * v2c - stage.ic2eq);

        m_lastLow = v2c;
        m_lastBand = v1c;
        m_lastHigh = vHigh;
        cascaded = (m_mode == Mode::HighPass) ? vHigh : ((m_mode == Mode::BandPass) ? v1c : v2c);
    }

    Outputs out{};
    out.low = m_lastLow;
    out.band = m_lastBand;
    out.high = m_lastHigh;

    switch (m_mode) {
    case Mode::BandPass:
        out.selected = out.band;
        break;
    case Mode::HighPass:
        out.selected = out.high;
        break;
    case Mode::LowPass:
    default:
        out.selected = out.low;
        break;
    }

    out.low = sanitize(out.low);
    out.band = sanitize(out.band);
    out.high = sanitize(out.high);
    out.selected = sanitize(out.selected);
    return out;
}

float WaspFilter::processSample(float input) noexcept {
    return processAll(input).selected;
}
