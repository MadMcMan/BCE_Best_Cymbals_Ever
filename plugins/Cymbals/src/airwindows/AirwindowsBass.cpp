#include "AirwindowsBass.hpp"
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
    AudioEffectX* aw_create_BassAmp();
    AudioEffectX* aw_create_BassDrive();
    AudioEffectX* aw_create_BassKit();
    AudioEffectX* aw_create_DubSub();
    AudioEffectX* aw_create_SubsOnly();
    AudioEffectX* aw_create_RetroBass();
}

class AirSuiteBass::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteBass, true) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteBass::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteBass::Type type, const float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteBass::getBassSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteBass::AirSuiteBass(float sampleRate)
    : m_impl(std::make_unique<Impl>(sampleRate)) {}

AirSuiteBass::~AirSuiteBass() = default;

void AirSuiteBass::setSampleRate(float sr) {
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteBass::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteBass::process(float& outL, float& outR, Type type, const float* params) {
    if (m_impl) m_impl->process(outL, outR, type, params);
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteBass::getBassSpec(AirSuiteBass::Type type) {
    return awspec::get_AirSuiteBass((int)type);
}
