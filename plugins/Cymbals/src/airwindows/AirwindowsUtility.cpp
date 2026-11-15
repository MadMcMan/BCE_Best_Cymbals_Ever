#include "AirwindowsUtility.hpp"
#include "AirwindowsAdapter.hpp"
#include "AirwindowsFactories.inc"
#include "AirSuiteUniversal.hpp"
#include "AirwindowsSpecTable.inc"
#include "vst2stub/audioeffectx.h"
#include <cstring>
#include <algorithm>
#include <memory>
#include <cmath>

START_NAMESPACE_DISTRHO

// Factory function declarations (implemented in winvst/<Name>_winvst.cpp)
// Exception: StereoFX has no winvst implementation, so we stub it locally
extern "C" {
    AudioEffectX* aw_create_Average();
    AudioEffectX* aw_create_Balanced();
    AudioEffectX* aw_create_BitShiftGain();
    AudioEffectX* aw_create_CStrip2();
    AudioEffectX* aw_create_PurestFade();
    AudioEffectX* aw_create_PurestGain();
    AudioEffectX* aw_create_PurestWarm();
    AudioEffectX* aw_create_Righteous4();
    AudioEffectX* aw_create_Slew();
    AudioEffectX* aw_create_Slew2();
    AudioEffectX* aw_create_SlewOnly();
    AudioEffectX* aw_create_StereoFX();   // provided by winvst/StereoFX_winvst.cpp
    AudioEffectX* aw_create_StudioTan();
    AudioEffectX* aw_create_Weight();
}

class AirSuiteUtility::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteUtility, true) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteUtility::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteUtility::Type type, const float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteUtility::getUtilitySpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteUtility::AirSuiteUtility(float sampleRate)
    : m_impl(std::make_unique<Impl>(sampleRate)) {}

AirSuiteUtility::~AirSuiteUtility() = default;

void AirSuiteUtility::setSampleRate(float sr) {
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteUtility::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteUtility::process(float& outL, float& outR, Type type, const float* params) {
    if (m_impl) m_impl->process(outL, outR, type, params);
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteUtility::getUtilitySpec(AirSuiteUtility::Type type) {
    return awspec::get_AirSuiteUtility((int)type);
}
