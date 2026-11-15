#include "AirwindowsSubtlety.hpp"
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
    AudioEffectX* aw_create_Baxandall();
    AudioEffectX* aw_create_Baxandall2();
    AudioEffectX* aw_create_Baxandall3();
    AudioEffectX* aw_create_Calibre();
    AudioEffectX* aw_create_Coils();
    AudioEffectX* aw_create_Coils2();
    AudioEffectX* aw_create_Interstage();
    AudioEffectX* aw_create_Mojo();
    AudioEffectX* aw_create_PlatinumSlew();
    AudioEffectX* aw_create_SlewSonic();
    AudioEffectX* aw_create_Smooth();
    AudioEffectX* aw_create_SmoothEQ();
    AudioEffectX* aw_create_TubeDesk();
    AudioEffectX* aw_create_UnBox();
}

class AirSuiteSubtlety::Impl {
public:
    Impl(float sr) : m_adapter(sr, awspec::createFor_AirSuiteSubtlety, true) { m_adapter.setSampleRate(sr); }
    void setSampleRate(float sr) { m_adapter.setSampleRate(sr); }
    void setAlgorithm(AirSuiteSubtlety::Type type) { m_adapter.setType((int)type); }
    void reset() { m_adapter.reset(); }
    void process(float& L, float& R, AirSuiteSubtlety::Type type, const float* params) {
        m_adapter.setType((int)type);
        m_adapter.process(L, R, params, AirSuiteSubtlety::getSubtletySpec(type).count);
    }
private:
    AirwindowsAdapter m_adapter;
};

AirSuiteSubtlety::AirSuiteSubtlety(float sampleRate)
    : m_impl(std::make_unique<Impl>(sampleRate)) {}

AirSuiteSubtlety::~AirSuiteSubtlety() = default;

void AirSuiteSubtlety::setSampleRate(float sr) {
    if (m_impl) m_impl->setSampleRate(sr);
}

void AirSuiteSubtlety::reset() {
    if (m_impl) m_impl->reset();
}

void AirSuiteSubtlety::process(float& outL, float& outR, Type type, const float* params) {
    if (m_impl) m_impl->process(outL, outR, type, params);
}

END_NAMESPACE_DISTRHO

AirwindowsSpec AirSuiteSubtlety::getSubtletySpec(AirSuiteSubtlety::Type type) {
    return awspec::get_AirSuiteSubtlety((int)type);
}
