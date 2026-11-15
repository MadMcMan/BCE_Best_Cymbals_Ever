#ifndef AUDIO_INSTRUMENTATION_HPP
#define AUDIO_INSTRUMENTATION_HPP

#include <cmath>
#include <algorithm>

/**
 * AudioInstrumentation - Header-only audio validation utilities
 * 
 * Provides detection and measurement functions for audio quality validation:
 * - Silence/dropout detection
 * - NaN/Inf detection (matching pattern from AirwindowsParametricEQ.hpp:267)
 * - DC offset detection
 * - RMS and peak measurements
 */

namespace AudioInstrumentation {

/**
 * Detect silence based on threshold in dB.
 * @param L Left channel sample
 * @param R Right channel sample
 * @param thresholdDb Threshold in dB (default -120.0f represents ~3.05e-6 linear amplitude)
 * @return true if both channels are below threshold (silence detected)
 */
inline bool detectSilence(float L, float R, float thresholdDb = -120.0f) {
    const float linearThreshold = std::pow(10.0f, thresholdDb / 20.0f);
    return (std::abs(L) < linearThreshold) && (std::abs(R) < linearThreshold);
}

/**
 * Detect NaN or Inf in audio samples.
 * Pattern matches AirwindowsParametricEQ.hpp behavior.
 * @param L Left channel sample
 * @param R Right channel sample
 * @return true if either channel contains NaN or Inf
 */
inline bool detectNaNInf(float L, float R) {
    return (std::isnan(L) || std::isinf(L)) || (std::isnan(R) || std::isinf(R));
}

/**
 * Detect DC offset (sustained non-zero baseline).
 * @param L Left channel sample
 * @param R Right channel sample
 * @param threshold DC offset threshold (default 0.01f)
 * @return true if either channel has DC offset above threshold
 */
inline bool detectDCOffset(float L, float R, float threshold = 0.01f) {
    return (std::abs(L) > threshold) || (std::abs(R) > threshold);
}

/**
 * Calculate RMS (Root Mean Square) of a buffer.
 * @param buffer Audio buffer
 * @param samples Number of samples
 * @return RMS value
 */
inline float calculateRMS(const float* buffer, int samples) {
    if (samples <= 0) return 0.0f;
    
    float sumSquares = 0.0f;
    for (int i = 0; i < samples; ++i) {
        const float s = buffer[i];
        sumSquares += s * s;
    }
    return std::sqrt(sumSquares / samples);
}

/**
 * Measure peak amplitude of a buffer.
 * @param buffer Audio buffer
 * @param samples Number of samples
 * @return Peak absolute amplitude
 */
inline float measurePeak(const float* buffer, int samples) {
    if (samples <= 0) return 0.0f;
    
    float peak = 0.0f;
    for (int i = 0; i < samples; ++i) {
        peak = std::max(peak, std::abs(buffer[i]));
    }
    return peak;
}

/**
 * Convert linear amplitude to dB.
 * @param linear Linear amplitude (0.0 to 1.0 typical)
 * @return dB value (negative for amplitudes < 1.0)
 */
inline float linearToDb(float linear) {
    if (linear <= 0.0f) return -120.0f;
    return 20.0f * std::log10(linear);
}

} // namespace AudioInstrumentation

#endif // AUDIO_INSTRUMENTATION_HPP
