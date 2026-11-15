#include "AirwindowsLoFi.hpp"
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
    AudioEffectX* aw_create_Desk();
    AudioEffectX* aw_create_Desk4();
    AudioEffectX* aw_create_DigitalBlack();
    AudioEffectX* aw_create_DitherFloat();
    AudioEffectX* aw_create_FathomFive();
    AudioEffectX* aw_create_Gringer();
    AudioEffectX* aw_create_GrooveWear();
    AudioEffectX* aw_create_Loud();
    AudioEffectX* aw_create_Mackity();
    AudioEffectX* aw_create_NCSeventeen();
    AudioEffectX* aw_create_PocketVerbs();
    AudioEffectX* aw_create_SampleDelay();
}

class AirSuiteLoFi::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteLoFi, true) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteLoFi::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteLoFi::Type type, const float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteLoFi::getLoFiSpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteLoFi::AirSuiteLoFi(float sampleRate)
    : m_impl(std::make_unique<Impl>(sampleRate)) {}

AirSuiteLoFi::~AirSuiteLoFi() = default;

void AirSuiteLoFi::setSampleRate(float sr) {
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteLoFi::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteLoFi::process(float& outL, float& outR, Type type, const float* params) {
    if (m_impl) m_impl->process(outL, outR, type, params);
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteLoFi::getLoFiSpec(AirSuiteLoFi::Type type) {
    return awspec::get_AirSuiteLoFi((int)type);
}
