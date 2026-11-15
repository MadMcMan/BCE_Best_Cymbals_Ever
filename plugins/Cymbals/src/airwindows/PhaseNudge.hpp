#ifndef PHASE_NUDGE_HPP
#define PHASE_NUDGE_HPP

#include <cmath>
#include <algorithm>

// =============================================================================
// Airwindows PhaseNudge Port/Wrapper
// Original source: deps/airwindows/plugins/WinVST/PhaseNudge/PhaseNudgeProc.cpp
// Licensed under the MIT license by Chris Johnson (Airwindows)
// =============================================================================

class PhaseNudge {
public:
    static constexpr int kNumStages = 4; // Cascaded allpasses for stronger effect

    PhaseNudge() : A(0.5f), B(0.0f) {
        for (int s = 0; s < kNumStages; ++s) {
            stages[s].one = 0;
            stages[s].maxdelay = 0;
            for (int i = 0; i < 1503; ++i) {
                stages[s].dL[i] = 0.0;
                stages[s].dR[i] = 0.0;
            }
        }
    }

    void setNudge(float nudge) { A = nudge; } // 0..1
    void setAmount(float amount) { B = amount; } // 0..1 (Wet)

    void clear() {
        for (int s = 0; s < kNumStages; ++s) {
            for (int i = 0; i < 1503; ++i) {
                stages[s].dL[i] = 0.0;
                stages[s].dR[i] = 0.0;
            }
            stages[s].one = 0;
            stages[s].maxdelay = 0;
        }
    }

    void process(float& sampleL, float& sampleR) {
        if (B <= 0.0f) return; // Completely dry

        // Avoid pathological near-zero-delay allpass behavior inside feedback loops.
        // At very low nudge values, bypass entirely for stability and predictability.
        const float nudge = std::clamp(A, 0.0f, 1.0f);
        if (nudge < 0.01f) return;

        double outallpass = 0.618033988749894848204586; // Golden ratio
        int maxdelayTarget = std::max(2, (int)(std::pow(nudge, 3) * 1501.0));
        double wet = B;
        
        double inputSampleL = sampleL;
        double inputSampleR = sampleR;
        double drySampleL = inputSampleL;
        double drySampleR = inputSampleR;

        // Pre-shaping (sin)
        inputSampleL /= 4.0;
        inputSampleR /= 4.0;
        
        double bridgerectifierL = std::abs(inputSampleL);
        bridgerectifierL = std::sin(bridgerectifierL);
        if (inputSampleL > 0) inputSampleL = bridgerectifierL;
        else inputSampleL = -bridgerectifierL;

        double bridgerectifierR = std::abs(inputSampleR);
        bridgerectifierR = std::sin(bridgerectifierR);
        if (inputSampleR > 0) inputSampleR = bridgerectifierR;
        else inputSampleR = -bridgerectifierR;

        // Cascaded allpass stages — each adds more phase shift,
        // making S produce clearly audible spectral changes
        for (int s = 0; s < kNumStages; ++s) {
            AllpassStage& st = stages[s];

            if (std::abs(st.maxdelay - maxdelayTarget) > 1500) st.maxdelay = maxdelayTarget;
            if (st.maxdelay < maxdelayTarget) {
                st.maxdelay++;
                st.dL[st.maxdelay] = (st.dL[0] + st.dL[st.maxdelay - 1]) / 2.0;
                st.dR[st.maxdelay] = (st.dR[0] + st.dR[st.maxdelay - 1]) / 2.0;
            } else if (st.maxdelay > maxdelayTarget) {
                st.maxdelay--;
                st.dL[st.maxdelay] = (st.dL[0] + st.dL[st.maxdelay]) / 2.0;
                st.dR[st.maxdelay] = (st.dR[0] + st.dR[st.maxdelay]) / 2.0;
            }

            int allpasstemp = st.one - 1;
            if (allpasstemp < 0 || allpasstemp > st.maxdelay) allpasstemp = st.maxdelay;

            inputSampleL -= st.dL[allpasstemp] * outallpass;
            inputSampleR -= st.dR[allpasstemp] * outallpass;
            st.dL[st.one] = inputSampleL;
            st.dR[st.one] = inputSampleR;
            inputSampleL *= outallpass;
            inputSampleR *= outallpass;
            st.one--;
            if (st.one < 0 || st.one > st.maxdelay) { st.one = st.maxdelay; }
            inputSampleL += st.dL[st.one];
            inputSampleR += st.dR[st.one];
        }

        // Post-shaping (cos)
        bridgerectifierL = std::abs(inputSampleL);
        bridgerectifierL = 1.0 - std::cos(bridgerectifierL);
        if (inputSampleL > 0) inputSampleL -= bridgerectifierL;
        else inputSampleL += bridgerectifierL;

        bridgerectifierR = std::abs(inputSampleR);
        bridgerectifierR = 1.0 - std::cos(bridgerectifierR);
        if (inputSampleR > 0) inputSampleR -= bridgerectifierR;
        else inputSampleR += bridgerectifierR;

        inputSampleL *= 4.0;
        inputSampleR *= 4.0;

        // Mix — wet/dry interference creates the audible comb filtering
        sampleL = (float)((drySampleL * (1.0 - wet)) + (inputSampleL * wet));
        sampleR = (float)((drySampleR * (1.0 - wet)) + (inputSampleR * wet));
    }

private:
    struct AllpassStage {
        double dL[1503];
        double dR[1503];
        int one, maxdelay;
    };
    AllpassStage stages[kNumStages];
    float A; // Nudge
    float B; // Amount/Wet
};

#endif
