#include "AirwindowsStereo.hpp"
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
    AudioEffectX* aw_create_Channel4();
    AudioEffectX* aw_create_Channel5();
    AudioEffectX* aw_create_Channel6();
    AudioEffectX* aw_create_Channel7();
    AudioEffectX* aw_create_Channel8();
    AudioEffectX* aw_create_Channel9();
    AudioEffectX* aw_create_MidSide();
    AudioEffectX* aw_create_PhaseNudge();
    AudioEffectX* aw_create_Srsly();
    AudioEffectX* aw_create_Wider();
}

class AirSuiteStereo::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteStereo, true) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteStereo::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteStereo::Type type, const float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteStereo::getStereoSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteStereo::AirSuiteStereo(float sampleRate)
    : m_impl(std::make_unique<Impl>(sampleRate)) {}

AirSuiteStereo::~AirSuiteStereo() = default;

void AirSuiteStereo::setSampleRate(float sr) {
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteStereo::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteStereo::process(float& outL, float& outR, Type type, const float* params) {
    if (m_impl) m_impl->process(outL, outR, type, params);
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteStereo::getStereoSpec(AirSuiteStereo::Type type) {
    return awspec::get_AirSuiteStereo((int)type);
}
