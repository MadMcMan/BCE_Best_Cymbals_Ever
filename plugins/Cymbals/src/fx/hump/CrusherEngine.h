#pragma once

#include "Compressor.h"
#include "Distortion.h"
#include "Filter.h"
#include "ParamRamp.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

namespace hump
{

// All controls in their normalised 0..1 form (switches are plain bools).
struct EngineParams
{
    bool masterOn = true;
    bool distortionOn = true;
    float mech = 0.0f;
    float tube = 0.0f;
    bool filterOn = false;
    float cutoff = 1.0f;
    float resonance = 0.0f;
    bool compressorOn = false;
    float compAmount = 0.0f;
    bool phat = false;
    float mix = 1.0f;
    float volume = 1.0f;
};

// Signal chain: Mech -> Filter -> Tube -> Compressor -> Mix/Volume -> clip.
// Audio is processed in sub-blocks of at most 64 samples; continuous
// parameters glide per sub-block, mix and volume glide per sample.
class CrusherEngine
{
public:
    static constexpr int kSubBlockSize = 64;
    static constexpr float kVolumeCurve = 2.737f;
    static constexpr float kOutputCeiling = 0.9999f;

    static float volumeToGain (float volume) noexcept
    {
        return static_cast<float> (std::pow (static_cast<double> (volume), static_cast<double> (kVolumeCurve)));
    }

    void prepare (double sampleRate) noexcept
    {
        for (auto* ramp : allRamps())
            ramp->setSampleRate (sampleRate);
        filter.setSampleRate (sampleRate);
        compressor.setSampleRate (sampleRate);
        reset();
    }

    void reset() noexcept
    {
        filter.reset();
        compressor.reset();
        for (auto* ramp : allRamps())
            ramp->snapToTarget();
        mixState = 0.0f;
        gainState = 0.0f;
    }

    void setParameters (const EngineParams& newParams) noexcept
    {
        params = newParams;
        mech.setTarget (newParams.mech);
        tube.setTarget (newParams.tube);
        cutoff.setTarget (newParams.cutoff);
        resonance.setTarget (newParams.resonance);
        // The amount control spans 0..0.99 internally (+40 dB of drive at most).
        compAmount.setTarget (newParams.compAmount * Compressor::kMaxAmount);
        mix.setTarget (newParams.mix);
        volume.setTarget (newParams.volume);
        compressor.setPhatMode (newParams.phat);
    }

    // Processes a stereo buffer in place. When master is off the audio passes
    // through untouched.
    void process (float* left, float* right, int numSamples) noexcept
    {
        if (! params.masterOn)
            return;

        for (int start = 0; start < numSamples; start += kSubBlockSize)
        {
            const int count = std::min (kSubBlockSize, numSamples - start);
            processSubBlock (left + start, right + start, count);
        }
    }

private:
    void processSubBlock (float* left, float* right, int count) noexcept
    {
        // A NaN/inf from upstream would otherwise latch in the filter history.
        for (int i = 0; i < count; ++i)
        {
            if (! std::isfinite (left[i]))
                left[i] = 0.0f;
            if (! std::isfinite (right[i]))
                right[i] = 0.0f;
        }

        std::memcpy (dryLeft.data(), left, sizeof (float) * static_cast<size_t> (count));
        std::memcpy (dryRight.data(), right, sizeof (float) * static_cast<size_t> (count));

        const float mechValue = mech.advance (count);
        const float tubeValue = tube.advance (count);
        const float cutoffValue = cutoff.advance (count);
        const float resonanceValue = resonance.advance (count);
        const float compValue = compAmount.advance (count);

        if (params.distortionOn && mechValue > 0.0f)
            distortion::processStereo (distortion::MechShape (mechValue), left, right, count);

        if (params.filterOn)
            filter.process (left, right, count, cutoffValue, resonanceValue);

        if (params.distortionOn && tubeValue > 0.0f)
            distortion::processStereo (distortion::TubeShape (tubeValue), left, right, count);

        if (params.compressorOn)
            compressor.process (left, right, count, compValue);

        applyMixAndVolume (left, right, count, mix.advance (count), volumeToGain (volume.advance (count)));
    }

    void applyMixAndVolume (float* left, float* right, int count, float targetMix, float targetGain) noexcept
    {
        const float invCount = 1.0f / static_cast<float> (count);
        const float mixStep = (targetMix - mixState) * invCount;
        const float gainStep = (targetGain - gainState) * invCount;

        for (int i = 0; i < count; ++i)
        {
            const float l = (mixState * left[i] + (1.0f - mixState) * dryLeft[static_cast<size_t> (i)]) * gainState;
            const float r = (mixState * right[i] + (1.0f - mixState) * dryRight[static_cast<size_t> (i)]) * gainState;
            left[i] = std::clamp (l, -kOutputCeiling, kOutputCeiling);
            right[i] = std::clamp (r, -kOutputCeiling, kOutputCeiling);
            mixState += mixStep;
            gainState += gainStep;
        }
    }

    std::array<ParamRamp*, 7> allRamps() noexcept
    {
        return { &mech, &tube, &cutoff, &resonance, &compAmount, &mix, &volume };
    }

    EngineParams params;
    ParamRamp mech, tube, cutoff { 1.0f }, resonance, compAmount, mix { 1.0f }, volume { 1.0f };
    Filter filter;
    Compressor compressor;
    std::array<float, kSubBlockSize> dryLeft {}, dryRight {};
    float mixState = 0.0f;
    float gainState = 0.0f;
};

} // namespace hump
