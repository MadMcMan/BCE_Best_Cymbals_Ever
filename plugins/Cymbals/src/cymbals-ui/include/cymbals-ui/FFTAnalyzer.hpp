#ifndef FFT_ANALYZER_HPP
#define FFT_ANALYZER_HPP

#include <cmath>
#include <algorithm>
#include <complex>

// Simple FFT Analyzer using Cooley-Tukey algorithm
// Outputs magnitude spectrum for visualization
class FFTAnalyzer {
public:
    static constexpr int kFFTSize = 2048;
    static constexpr int kNumBins = kFFTSize / 2;
    static constexpr int kNumBands = 64; // Display bands for UI

    FFTAnalyzer(float sampleRate = 48000.0f)
        : fSampleRate(sampleRate),
          writeIndex(0),
          sampleCount(0)
    {
        // Initialize Hann window
        for (int i = 0; i < kFFTSize; ++i) {
            fWindow[i] = 0.5f * (1.0f - std::cos(2.0f * 3.14159265359f * i / (kFFTSize - 1)));
        }

        // Initialize band center frequencies (logarithmic distribution 20Hz - 20kHz)
        float minFreq = 20.0f;
        float maxFreq = 20000.0f;
        for (int i = 0; i < kNumBands; ++i) {
            float t = (float)i / (kNumBands - 1);
            fBandFrequencies[i] = minFreq * std::pow(maxFreq / minFreq, t);
        }

        reset();
    }

    void setSampleRate(float sr) {
        fSampleRate = sr;
    }

    void reset() {
        for (int i = 0; i < kFFTSize; ++i) {
            fReal[i] = 0.0f;
            fImag[i] = 0.0f;
        }
        for (int i = 0; i < kNumBins; ++i) {
            fMagnitude[i] = 0.0f;
        }
        for (int i = 0; i < kNumBands; ++i) {
            fBands[i] = 0.0f;
        }
        writeIndex = 0;
        sampleCount = 0;
    }

    // Process stereo input and compute spectrum
    void process(float left, float right) {
        // Mix to mono and store in circular buffer
        float mixed = (left + right) * 0.5f;
        fReal[writeIndex] = mixed * fWindow[writeIndex];
        fImag[writeIndex] = 0.0f;

        writeIndex = (writeIndex + 1) % kFFTSize;
        sampleCount++;

        // Compute FFT when buffer is full
        if (sampleCount >= kFFTSize) {
            computeFFT();
            computeBands();
            sampleCount = 0;
        }
    }

    // Get magnitude for a specific frequency bin
    float getMagnitude(int bin) const {
        if (bin >= 0 && bin < kNumBins) {
            return fMagnitude[bin];
        }
        return 0.0f;
    }

    // Get averaged magnitude for display band
    float getBand(int band) const {
        if (band >= 0 && band < kNumBands) {
            return fBands[band];
        }
        return 0.0f;
    }

    // Get number of display bands
    int getNumBands() const { return kNumBands; }

    // Convert frequency to bin index
    int frequencyToBin(float freq) const {
        float binWidth = fSampleRate / kFFTSize;
        return (int)(freq / binWidth);
    }

    // Get frequency for a band index
    float getBandFrequency(int band) const {
        if (band >= 0 && band < kNumBands) {
            return fBandFrequencies[band];
        }
        return 0.0f;
    }

private:
    float fSampleRate;
    float fWindow[kFFTSize];
    float fReal[kFFTSize];
    float fImag[kFFTSize];
    float fMagnitude[kNumBins];
    float fBands[kNumBands];
    float fBandFrequencies[kNumBands];
    int writeIndex = 0;
    int sampleCount = 0;

    // Cooley-Tukey FFT (radix-2, in-place)
    void computeFFT() {
        // Bit-reversal permutation
        int n = kFFTSize;
        for (int i = 1, j = 0; i < n; i++) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) {
                std::swap(fReal[i], fReal[j]);
                std::swap(fImag[i], fImag[j]);
            }
        }

        // Cooley-Tukey iterative FFT
        for (int len = 2; len <= n; len <<= 1) {
            float ang = 2.0f * 3.14159265359f / len;
            float wlen_r = std::cos(ang);
            float wlen_i = std::sin(-ang);
            for (int i = 0; i < n; i += len) {
                float w_r = 1.0f;
                float w_i = 0.0f;
                for (int j = 0; j < len / 2; j++) {
                    float u_r = fReal[i + j];
                    float u_i = fImag[i + j];
                    float v_r = fReal[i + j + len / 2] * w_r - fImag[i + j + len / 2] * w_i;
                    float v_i = fReal[i + j + len / 2] * w_i + fImag[i + j + len / 2] * w_r;
                    fReal[i + j] = u_r + v_r;
                    fImag[i + j] = u_i + v_i;
                    fReal[i + j + len / 2] = u_r - v_r;
                    fImag[i + j + len / 2] = u_i - v_i;
                    float next_w_r = w_r * wlen_r - w_i * wlen_i;
                    w_i = w_r * wlen_i + w_i * wlen_r;
                    w_r = next_w_r;
                }
            }
        }

        // Compute magnitude spectrum
        for (int i = 0; i < kNumBins; ++i) {
            float mag = std::sqrt(fReal[i] * fReal[i] + fImag[i] * fImag[i]);
            // Apply scaling and convert to dB
            mag /= kFFTSize;
            fMagnitude[i] = mag > 0.0f ? 20.0f * std::log10(mag + 1e-10f) : -100.0f;
            // Normalize to 0-1 range (-60dB to 0dB range)
            fMagnitude[i] = std::clamp((fMagnitude[i] + 60.0f) / 60.0f, 0.0f, 1.0f);
        }
    }

    // Compute logarithmically-spaced frequency bands
    void computeBands() {
        float binWidth = fSampleRate / kFFTSize;

        for (int b = 0; b < kNumBands; ++b) {
            float lowFreq = (b == 0) ? 20.0f : fBandFrequencies[b] * 0.8f;
            float highFreq = (b == kNumBands - 1) ? 20000.0f : fBandFrequencies[b] * 1.25f;

            int lowBin = (int)(lowFreq / binWidth);
            int highBin = (int)(highFreq / binWidth);

            lowBin = std::max(0, lowBin);
            highBin = std::min(kNumBins - 1, highBin);

            float sum = 0.0f;
            int count = 0;
            for (int i = lowBin; i <= highBin; ++i) {
                sum += fMagnitude[i];
                count++;
            }

            fBands[b] = (count > 0) ? sum / count : 0.0f;
        }
    }
};

#endif // FFT_ANALYZER_HPP
