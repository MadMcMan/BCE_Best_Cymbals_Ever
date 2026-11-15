#include "AirwindowsAmbience.hpp"
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
    AudioEffectX* aw_create_Aura();
    AudioEffectX* aw_create_BrightAmbience();
    AudioEffectX* aw_create_BrightAmbience2();
    AudioEffectX* aw_create_BrightAmbience3();
    AudioEffectX* aw_create_Chamber();
    AudioEffectX* aw_create_Chamber2();
    AudioEffectX* aw_create_Verbity();
    AudioEffectX* aw_create_Verbity2();
}

class AirSuiteAmbience::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteAmbience, true) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteAmbience::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteAmbience::Type type, const float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteAmbience::getAmbienceSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteAmbience::AirSuiteAmbience(float sampleRate)
    : m_impl(std::make_unique<Impl>(sampleRate)) {}

AirSuiteAmbience::~AirSuiteAmbience() = default;

void AirSuiteAmbience::setSampleRate(float sr) {
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteAmbience::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteAmbience::process(float& outL, float& outR, Type type, const float* params) {
    if (m_impl) m_impl->process(outL, outR, type, params);
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteAmbience::getAmbienceSpec(AirSuiteAmbience::Type type) {
    return awspec::get_AirSuiteAmbience((int)type);
}
