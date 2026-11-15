#ifndef AIRWINDOWS_ADAPTER_HPP
#define AIRWINDOWS_ADAPTER_HPP

// One generic adapter over the winvst AudioEffectX interface, driven by the
// generated factory tables (AirwindowsFactories.inc). Replaces the
// per-algorithm boilerplate (member lists, ctor/dtor deletes, reset
// delete+recreate, process switches) in the 20 uniform AirSuite<Category>
// Impl classes. Audio-identical to the old eager pattern: only the selected
// algorithm's effect is created, and only it is ever processed.
//
// The effect's setParameter throws on out-of-range indices (upstream
// `default: throw`), so every call is gated by the class spec count.

#include "AirwindowsCommon.hpp"
#include "vst2stub/audioeffectx.h"

#include <cmath>

START_NAMESPACE_DISTRHO

class AirwindowsAdapter {
public:
    using Factory = AudioEffectX* (*)(int);

    explicit AirwindowsAdapter(float sr, Factory factory, bool numParamsPolicy = false)
        : m_sampleRate(sr), m_factory(factory), m_useNumParams(numParamsPolicy),
          m_effect(nullptr), m_type(-1) {}

    ~AirwindowsAdapter() { delete m_effect; }

    AirwindowsAdapter(const AirwindowsAdapter&) = delete;
    AirwindowsAdapter& operator=(const AirwindowsAdapter&) = delete;

    void setSampleRate(float sr) {
        m_sampleRate = sr;
        if (m_effect) m_effect->setSampleRate(sr);
    }

    // Selects the algorithm, creating the effect lazily on first use.
    void setType(int type) {
        if (type == m_type && m_effect) return;
        delete m_effect;
        m_effect = nullptr;
        m_type = type;
        m_effect = m_factory(type);
        if (m_effect) m_effect->setSampleRate(m_sampleRate);
    }

    // Hard reset: recreate the effect so internal delay lines / history are
    // cleared. Bypasses setType's same-type early-return (m_type is unchanged
    // here), so an existing effect is actually torn down and rebuilt. Called on
    // the exceptional preset/panic reset path, never per-block.
    void reset() {
        delete m_effect;
        m_effect = m_factory(m_type);
        if (m_effect) m_effect->setSampleRate(m_sampleRate);
    }

    void process(float& L, float& R, const float* params, int paramCount) {
        if (!m_effect) return; // unknown/unsupported type: silent (matches legacy)
        // Classes with the vendored-numParams policy applied min(numParams, 10)
        // params regardless of the spec count; spec-count classes use the count.
        const int n = m_useNumParams ? std::min(m_effect->numParams, 10) : paramCount;
        for (int i = 0; i < n && i < 13; ++i) {
            const float v = std::clamp(params[i], 0.0f, 1.0f);
            m_effect->setParameter(i, v);
        }
        float inL = L, inR = R, outL = 0.0f, outR = 0.0f;
        float* inputs[2] = {&inL, &inR};
        float* outputs[2] = {&outL, &outR};
        m_effect->processReplacing(inputs, outputs, 1);
        if (!std::isfinite(outL)) outL = 0.0f;
        if (!std::isfinite(outR)) outR = 0.0f;
        L = outL;
        R = outR;
    }

private:
    float m_sampleRate;
    Factory m_factory;
    bool m_useNumParams;
    AudioEffectX* m_effect;
    int m_type;
};

END_NAMESPACE_DISTRHO

#endif // AIRWINDOWS_ADAPTER_HPP
