#include "AirwindowsXYZ.hpp"
#include "AirwindowsAdapter.hpp"
#include "AirwindowsFactories.inc"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include "vst2stub/audioeffectx.h"
#include <cstring>
#include <memory>
#include <cmath>

START_NAMESPACE_DISTRHO

extern "C" {
    AudioEffectX* aw_create_XBandpass(); AudioEffectX* aw_create_XHighpass(); AudioEffectX* aw_create_XLowpass();
    AudioEffectX* aw_create_XNotch(); AudioEffectX* aw_create_XRegion(); AudioEffectX* aw_create_YBandpass();
    AudioEffectX* aw_create_YHighpass(); AudioEffectX* aw_create_YLowpass(); AudioEffectX* aw_create_YNotch();
    AudioEffectX* aw_create_ZBandpass(); AudioEffectX* aw_create_ZHighpass();
}

class AirSuiteXYZ::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteXYZ, true) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteXYZ::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteXYZ::Type type, const float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteXYZ::getXYZSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteXYZ::AirSuiteXYZ(float sampleRate) : m_impl(std::make_unique<Impl>(sampleRate)) {}
AirSuiteXYZ::~AirSuiteXYZ() = default;
void AirSuiteXYZ::setSampleRate(float sr) { if (m_impl) m_impl->setSampleRate(sr); }
void AirSuiteXYZ::reset() { if (m_impl) m_impl->reset(); }
void AirSuiteXYZ::process(float& outL, float& outR, Type type, const float* params) {
    if (m_impl) m_impl->process(outL, outR, type, params);
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteXYZ::getXYZSpec(AirSuiteXYZ::Type type) {
    return awspec::get_AirSuiteXYZ((int)type);
}
