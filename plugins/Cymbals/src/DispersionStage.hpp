#ifndef DISPERSION_STAGE_HPP
#define DISPERSION_STAGE_HPP

#include "Limiter.hpp"
#include "airwindows/PhaseNudge.hpp"
#include "PhaseDistortion.hpp"
#include "AdsrEnvelope.hpp"
#include "DSPUtils.hpp"
#include <cmath>

START_NAMESPACE_DISTRHO

using namespace DSPUtils;

class DispersionStage {
public:
    // Near-unity ceiling requested by the FB loop UI (99.999%). The old cap
    // was -0.1 dB (98.855%); this allows the top of the control to reach the
    // practical unity bound, while the stage limiter still guards the output.
    static constexpr float kMaxFeedbackAmount = 0.99999f;

    // Exponential control curve, normalized so f(0)=0 and f(1)=1:
    //   f(c) = (1 - exp(-k*c)) / (1 - exp(-k)),  k = 4
    // Unlike the old log1p curve, the derivative falls toward the top instead
    // of peaking there, so the last few control steps resolve 99.9% -> 99.999%
    // instead of jumping across the ceiling.
    static float feedbackExpCurve(float control) {
        const float c = std::clamp(control, 0.0f, 1.0f);
        static constexpr float kExpRate = 4.0f;
        static const float kDenom = 1.0f - std::exp(-kExpRate);
        return (1.0f - std::exp(-kExpRate * c)) / kDenom;
    }

    static float feedbackFromControl(float control) {
        return std::clamp(feedbackExpCurve(control) * kMaxFeedbackAmount,
                          0.0f, kMaxFeedbackAmount);
    }

    // Exponential overall feedback trim [0,1] -> [0,1], with finer control near 1.0.
    static float overallFeedbackScaleFromControl(float control) {
        return feedbackExpCurve(control);
    }

    DispersionStage(float sampleRate = 48000.0f)
        : sampleRate(sampleRate),
          limiter(sampleRate), lfoL(sampleRate), lfoR(sampleRate), chaosEnv(sampleRate),
          inputGain(1.0f), feedbackAmount(kMaxFeedbackAmount), delaySamples(1.0f),
          vibratoDepth(0.0f), chaosFactor(0.0f), chaosEnvAmount(0.0f),
          feedbackSignalL(0.0f), feedbackSignalR(0.0f)
    {
        limiter.setThreshold(0.95f);
        chaosEnv.setAttack(0.001f); chaosEnv.setDecay(0.1f); chaosEnv.setSustain(0.0f);
        dampL.setCutoff(0.8f); dampR.setCutoff(0.8f);
        for (int i = 0; i < 20; ++i) {
            nudgeValues[i] = 0.5f;
            amountValues[i] = 0.0f;
            nudges[i].setNudge(nudgeValues[i]);
            nudges[i].setAmount(amountValues[i]);
        }
        activeNudgeCount = 0;
        nudgeGainCompensation = 1.0f;
        phaseDistortion.setSampleRate(sampleRate);
    }

    void setSampleRate(float sr) {
        sampleRate = sr;
        limiter.setSampleRate(sr); lfoL.setSampleRate(sr); lfoR.setSampleRate(sr); chaosEnv.setSampleRate(sr);
        phaseDistortion.setSampleRate(sr);
        phaseDistortion.reset();
    }

    void setInputGain(float g) { inputGain = g; }
    // Positive amount maps to negative loop polarity in process(): loop = in - feedback*amount.
    void setFeedback(float amount) { feedbackAmount = std::clamp(amount, 0.0f, kMaxFeedbackAmount); }
    void setDelaySamples(float s) { delaySamples = std::max(1.0f, s); }
    void setVibratoRate(float rate) { lfoL.setRate(rate); lfoR.setRate(rate * 1.05f); }
    void setVibratoDepth(float depth) { vibratoDepth = depth; }
    void setVibratoShape(int shape) { lfoL.setShape((LFO::Shape)shape); lfoR.setShape((LFO::Shape)shape); }
    void setDelayMode(int m) { delayLineL.setMode(m); delayLineR.setMode(m); }
    void setVibratoStepMix(float mix) { lfoL.setStepMix(mix); lfoR.setStepMix(mix); }
    // Smooths the vibrato depth signal: c 0 -> passthrough, 1 -> ~fully smoothed.
    void setVibratoSmooth(float c) { vibSmoothCoeff = 1.0f - std::clamp(c, 0.0f, 1.0f) * 0.999f; }
    void setChaos(float chaos) { chaosFactor = chaos; }
    void setChaosEnvDecay(float decay) { chaosEnv.setDecay(decay); }
    void setChaosEnvAmount(float amount) { chaosEnvAmount = amount; }
    void trigger() { chaosEnv.trigger(); }
    
    void clear() {
        delayLineL.clear(); delayLineR.clear();
        dcBlockL.clear(); dcBlockR.clear();
        dampL.clear(); dampR.clear();
        for (int i = 0; i < 20; ++i) nudges[i].clear();
        phaseDistortion.reset();
        feedbackSignalL = feedbackSignalR = 0.0f;
    }

    void setPDDrive(float v)     { phaseDistortion.setDrive(v); }
    void setPDNormalize(float v) { phaseDistortion.setNormalize(v); }
    void setPDTone(float v)      { phaseDistortion.setTone(v); }
    void setPDBias(float v)      { phaseDistortion.setBias(v); }
    void setPDSpread(float v)    { phaseDistortion.setSpread(v); }
    void setPDMix(float v)       { phaseDistortion.setMix(v); }

    float getPDDrive() const { return phaseDistortion.getDrive(); }

    void setNudge(int index, float val) {
        if (index >= 0 && index < 20) {
            nudgeValues[index] = std::clamp(val, 0.0f, 1.0f);
            nudges[index].setNudge(nudgeValues[index]);
        }
    }
    void setAmount(int index, float val) {
        if (index >= 0 && index < 20) {
            // PhaseNudge gets unstable/harsh quickly near 1.0 when cascaded in feedback.
            amountValues[index] = std::clamp(val, 0.0f, 0.6f);
            nudges[index].setAmount(amountValues[index]);
            recalcActiveNudges();
        }
    }

    void process(float& sampleL, float& sampleR) {
        float effectiveChaos = std::clamp(chaosFactor + chaosEnv.process() * chaosEnvAmount, 0.0f, 1.0f);
        float inL = sampleL * inputGain, inR = sampleR * inputGain;
        float loopL = inL - (feedbackSignalL * feedbackAmount), loopR = inR - (feedbackSignalR * feedbackAmount);

        // Phase nudges: process before delay/vibrato so each stage
        // shapes the spectral content going into the dispersion loop
        for (int i = 0; i < activeNudgeCount; ++i) nudges[i].process(loopL, loopR);

        // Compensate cumulative nudge gain so adding stages does not overload the loop.
        if (activeNudgeCount > 0) {
            loopL *= nudgeGainCompensation;
            loopR *= nudgeGainCompensation;
        }

        delayLineL.process(loopL); delayLineR.process(loopR);

        float modL = lfoL.processWithRate(lfoL.getRate() * (1.0f + effectiveChaos * 3.0f)) * vibratoDepth;
        float modR = lfoR.processWithRate(lfoR.getRate() * (1.0f + effectiveChaos * 3.0f)) * vibratoDepth;
        // Depth smoothing: one-pole on the LFO signal (vibSmoothCoeff 1 = passthrough).
        modL = vibSmoothL + vibSmoothCoeff * (modL - vibSmoothL); vibSmoothL = modL;
        modR = vibSmoothR + vibSmoothCoeff * (modR - vibSmoothR); vibSmoothR = modR;
        
        float processedL = delayLineL.getDelayed(std::max(1.0f, delaySamples + modL));
        float processedR = delayLineR.getDelayed(std::max(1.0f, delaySamples + modR));

        float basePDMix = phaseDistortion.getMix();
        phaseDistortion.setMix(std::clamp(basePDMix + effectiveChaos * 0.5f, 0.0f, 1.0f));
        phaseDistortion.process(processedL, processedR);
        phaseDistortion.setMix(basePDMix);
        
        limiter.process(processedL, processedR);
        processedL = dcBlockL.process(dampL.processLP(processedL));
        processedR = dcBlockR.process(dampR.processLP(processedR));

        if (std::abs(processedL) < 1e-8f) processedL = 0.0f;
        if (std::abs(processedR) < 1e-8f) processedR = 0.0f;

        feedbackSignalL = processedL; feedbackSignalR = processedR;
        sampleL = processedL; sampleR = processedR;
    }

private:
    void recalcActiveNudges() {
        // UI adds/removes nudges as a contiguous block from index 0.
        // Stop at first zero amount so only user-added stages run in DSP.
        activeNudgeCount = 0;
        float cumulativeAmount = 0.0f;
        for (int i = 0; i < 20; ++i) {
            if (amountValues[i] <= 1.0e-5f) {
                break;
            }
            activeNudgeCount = i + 1;
            cumulativeAmount += amountValues[i];
        }
        nudgeGainCompensation = (activeNudgeCount > 0)
            ? (1.0f / std::sqrt(1.0f + 0.8f * cumulativeAmount))
            : 1.0f;
    }

    float sampleRate;
    FractionalDelayLine delayLineL, delayLineR;
    OnePoleFilter dampL, dampR;
    Limiter limiter;
    LFO lfoL, lfoR;
    PhaseDistortion phaseDistortion;
    AdsrEnvelope chaosEnv;
    float inputGain, feedbackAmount, delaySamples, vibratoDepth, chaosFactor, chaosEnvAmount, feedbackSignalL, feedbackSignalR;
    float vibSmoothCoeff = 1.0f, vibSmoothL = 0.0f, vibSmoothR = 0.0f;
    DCBlock dcBlockL, dcBlockR;
    PhaseNudge nudges[20];
    float nudgeValues[20];
    float amountValues[20];
    int activeNudgeCount = 0;
    float nudgeGainCompensation = 1.0f;
};

END_NAMESPACE_DISTRHO

#endif
