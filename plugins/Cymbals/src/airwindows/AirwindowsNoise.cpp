#include "AirwindowsNoise.hpp"
#include "AirwindowsAdapter.hpp"
#include "AirwindowsFactories.inc"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include "vst2stub/audioeffectx.h"
#include <cstring>
#include <memory>
#include <cmath>

START_NAMESPACE_DISTRHO

// Factory function declarations (implemented in winvst/<Name>_winvst.cpp)
extern "C" {
    AudioEffectX* aw_create_DarkNoise();
    AudioEffectX* aw_create_Holt();
    AudioEffectX* aw_create_Noise();
    AudioEffectX* aw_create_Pear();
    AudioEffectX* aw_create_Surge();
    AudioEffectX* aw_create_SurgeTide();
}

class AirSuiteNoise::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteNoise, true) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteNoise::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteNoise::Type type, const float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteNoise::getNoiseSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteNoise::AirSuiteNoise(float sampleRate)
    : m_impl(std::make_unique<Impl>(sampleRate)) {}

AirSuiteNoise::~AirSuiteNoise() = default;

void AirSuiteNoise::setSampleRate(float sr) {
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteNoise::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteNoise::process(float& outL, float& outR, Type type, const float* params) {
    if (m_impl) m_impl->process(outL, outR, type, params);
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteNoise::getNoiseSpec(AirSuiteNoise::Type type) {
    return awspec::get_AirSuiteNoise((int)type);
}
