#include "AirwindowsClipping.hpp"
#include "AirwindowsAdapter.hpp"
#include "AirwindowsFactories.inc"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include "vst2stub/audioeffectx.h"
#include <cstring>
#include <algorithm>
#include <cmath>

START_NAMESPACE_DISTRHO

// Factory function declarations
extern "C" {
    AudioEffectX* aw_create_ADClip7();
    AudioEffectX* aw_create_ADClip8();
    AudioEffectX* aw_create_ClipOnly();
    AudioEffectX* aw_create_ClipOnly2();
    AudioEffectX* aw_create_ClipSoftly();
    AudioEffectX* aw_create_ADClip9();
}

// Pimpl implementation class
class AirSuiteClipping::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteClipping, true) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteClipping::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteClipping::Type type, float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteClipping::getClippingSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

// Public delegation
AirSuiteClipping::AirSuiteClipping(float sampleRate)
    : sampleRate(sampleRate), m_impl(std::make_unique<Impl>(sampleRate)) {}
AirSuiteClipping::~AirSuiteClipping() = default;
void AirSuiteClipping::setSampleRate(float sr) {
    sampleRate = sr;
    if (m_impl) m_impl->setSampleRate(sr);
}
void AirSuiteClipping::reset() {
    if (m_impl) m_impl->reset();
}
void AirSuiteClipping::process(float& L, float& R, Type type, float* params) {
    if (m_impl) m_impl->process(L, R, type, params);
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteClipping::getClippingSpec(AirSuiteClipping::Type type) {
    return awspec::get_AirSuiteClipping((int)type);
}
